#include "win/app.h"

#include <dbt.h>
#include <shellapi.h>

#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <string>
#include <utility>

#include "core/config.h"
#include "core/hook_health.h"
#include "core/idle_policy.h"
#include "example_ini.h"
#include "win/hooks.h"
#include "win/led.h"
#include "win/log.h"
#include "win/overlay.h"
#include "win/paths.h"
#include "win/res/resource.h"
#include "win/tray.h"

namespace kk::win {

namespace {

constexpr UINT WM_KK_TRAY = WM_APP + 3;
constexpr UINT_PTR kTimerHealth = 1;
constexpr UINT kHealthIntervalMs = 2000;
constexpr UINT_PTR kTimerIdle = 3;
constexpr UINT kIdleIntervalMs = 1000;
constexpr std::int32_t kReinstallThrottleMs = 60000;
constexpr std::int32_t kCatSuppressLogMs = 60000;
#ifdef KK_WATCHDOG
constexpr UINT_PTR kTimerWatchdog = 2;
constexpr UINT kWatchdogMs = 30000;
#endif
constexpr LONGLONG kMaxIniBytes = 64 * 1024;
constexpr const wchar_t* kNoCodeText =
    L"Kilit kodu tanımlı değil. Tepsi menüsünden ayar dosyasını aç, [lock] code değerini gir "
    L"(4-16 rakam).";

struct App {
    HWND window = nullptr;
    Paths paths;
    Config config;
    std::string ini_text;
    bool ini_loaded = false;
    UINT taskbar_created = 0;
    bool shut_down = false;
    bool reinstalled_before = false;
    DWORD last_reinstall_log = 0;
    int reinstalls_since_log = 0;
    bool cat_suppress_logged = false;
    DWORD last_cat_suppress_log = 0;
    IdlePolicy idle;
};

App g_app;

std::string read_file(const std::wstring& path) {
    const HANDLE file =
        CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return {};
    }
    std::string text;
    LARGE_INTEGER size{};
    if (GetFileSizeEx(file, &size) && size.QuadPart > 0 && size.QuadPart <= kMaxIniBytes) {
        text.resize(static_cast<std::size_t>(size.QuadPart));
        DWORD read = 0;
        if (ReadFile(file, text.data(), static_cast<DWORD>(text.size()), &read, nullptr)) {
            text.resize(read);
        } else {
            text.clear();
        }
    }
    CloseHandle(file);
    return text;
}

// Yalnız kilit açıkken. Dosya değişmediyse bir şey yapmaz.
void reload_config() {
    std::string text = read_file(g_app.paths.ini);
    if (g_app.ini_loaded && text == g_app.ini_text) {
        return;
    }
    g_app.ini_text = std::move(text);
    ConfigResult result = parse_config(g_app.ini_text);
    for (const ConfigIssue& issue : result.issues) {
        log_line(describe(issue));
    }
    g_app.config = std::move(result.config);
    const Config& c = g_app.config;
    hooks_configure(c.code, c.hotkey, c.cat_detect && c.has_code(),
                    CatThresholds{c.cat_min_simultaneous, static_cast<std::uint32_t>(c.cat_window_ms),
                                  c.cat_min_distinct});
    // Timer yalnız gerekince: boşta CPU ≈ 0.
    if (c.idle_lock_min > 0) {
        SetTimer(g_app.window, kTimerIdle, kIdleIntervalMs, nullptr);
    } else {
        KillTimer(g_app.window, kTimerIdle);
    }
    g_app.ini_loaded = true;
    log_line(g_app.config.has_code() ? "config loaded, code set" : "config loaded, no code");
}

void show_overlay() {
    if (g_app.config.overlay_enabled) {
        overlay_show(g_app.config.overlay_alpha);
    }
}

void lock(const char* reason) {
    if (hooks_locked()) {
        return;
    }
    reload_config();
    if (!g_app.config.has_code()) {
        log_line(std::string("lock refused, no code: ") + reason);
        tray_balloon(kNoCodeText);
        return;
    }
    hooks_lock();
    show_overlay();
    tray_set_locked(true);
    log_line(std::string("lock ") + reason);
#ifdef KK_WATCHDOG
    SetTimer(g_app.window, kTimerWatchdog, kWatchdogMs, nullptr);
#endif
    // Sonda: cihaz yavaşsa kilit ve overlay beklemesin.
    if (g_app.config.led_enabled) {
        led_lock(g_app.window, g_app.config.led_normal_color);
    }
}

// D3D tam ekran, tam ekran pencere veya sunum.
bool fullscreen_active() {
    QUERY_USER_NOTIFICATION_STATE state{};
    if (FAILED(SHQueryUserNotificationState(&state))) {
        return false;
    }
    return state == QUNS_BUSY || state == QUNS_RUNNING_D3D_FULL_SCREEN ||
           state == QUNS_PRESENTATION_MODE;
}

void on_cat() {
    if (hooks_locked()) {
        return;
    }
    if (g_app.config.pause_in_fullscreen && fullscreen_active()) {
        const DWORD now = GetTickCount();
        if (!g_app.cat_suppress_logged ||
            tick_diff(now, g_app.last_cat_suppress_log) >= kCatSuppressLogMs) {
            log_line("cat ignored, fullscreen");
            g_app.cat_suppress_logged = true;
            g_app.last_cat_suppress_log = now;
        }
        return;
    }
    lock("cat");
}

void on_idle_tick() {
    if (hooks_locked()) {
        return;
    }
    LASTINPUTINFO info{};
    info.cbSize = sizeof(info);
    if (!GetLastInputInfo(&info)) {
        return;
    }
    const std::uint32_t idle_ms = GetTickCount() - info.dwTime;  // işaretsiz: taşmaya dayanıklı
    if (!g_app.idle.due(idle_ms, g_app.config.idle_lock_min)) {
        return;
    }
    if (g_app.config.pause_in_fullscreen && fullscreen_active()) {
        return;
    }
    g_app.idle.mark_fired();
    lock("idle");
}

void on_unlocked(const char* reason) {
#ifdef KK_WATCHDOG
    KillTimer(g_app.window, kTimerWatchdog);
#endif
    overlay_hide();
    led_unlock(g_app.window);
    tray_set_locked(false);
    log_line(std::string("unlock ") + reason);
}

void check_hooks() {
    LASTINPUTINFO info{};
    info.cbSize = sizeof(info);
    if (!GetLastInputInfo(&info)) {
        return;
    }
    const std::uint32_t last_hook = hooks_last_event_tick();
    if (!hook_looks_dead(info.dwTime, last_hook)) {
        return;
    }
    // Yönetici penceresi odaktayken (UIPI) hook girdi görmez; kilit açıksa ve
    // uygulama yükseltilmemişse bu her 2 saniyede bir tekrar ederdi.
    const DWORD now = GetTickCount();
    const bool throttled = g_app.reinstalled_before &&
                           tick_diff(now, g_app.last_reinstall_log) < kReinstallThrottleMs;
    if (throttled && !hooks_locked()) {
        return;
    }
    const bool ok = hooks_reinstall();
    ++g_app.reinstalls_since_log;
    if (!throttled) {
        log_line("hook reinstall gap_ms=" + std::to_string(tick_diff(info.dwTime, last_hook)) +
                 " count=" + std::to_string(g_app.reinstalls_since_log) + (ok ? " ok" : " failed"));
        g_app.reinstalled_before = true;
        g_app.last_reinstall_log = now;
        g_app.reinstalls_since_log = 0;
    }
}

void ensure_ini() {
    const HANDLE file = CreateFileW(g_app.paths.ini.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(file, kExampleIni, static_cast<DWORD>(sizeof(kExampleIni) - 1), &written, nullptr);
        CloseHandle(file);
        log_line("config template created");
    }
}

void open_config() {
    ensure_ini();
    ShellExecuteW(g_app.window, L"open", g_app.paths.ini.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

// Önce geçici dosya: yazma yarıda kalırsa kodu içeren .ini bozulmasın.
bool write_ini(const std::string& text) {
    const std::wstring temp = g_app.paths.ini + L".tmp";
    const HANDLE file = CreateFileW(temp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD written = 0;
    const bool ok = WriteFile(file, text.data(), static_cast<DWORD>(text.size()), &written, nullptr) &&
                    written == text.size() && FlushFileBuffers(file);
    CloseHandle(file);
    if (!ok || !MoveFileExW(temp.c_str(), g_app.paths.ini.c_str(),
                            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(temp.c_str());
        return false;
    }
    return true;
}

struct IniSetting {
    const char* section;
    const char* key;
    std::string value;
};

// Tepsi ayarlarının ortak yazma yolu. Yazılan değerler yalnız sabit menü seçenekleridir.
bool apply_settings(std::initializer_list<IniSetting> settings) {
    ensure_ini();
    std::string text = read_file(g_app.paths.ini);
    // Boş okuma geçici bir hata olabilir: yazarsak kod silinirdi.
    if (text.empty()) {
        log_line("setting failed, config unreadable");
        return false;
    }
    for (const IniSetting& setting : settings) {
        text = set_ini_value(text, setting.section, setting.key, setting.value);
    }
    if (!write_ini(text)) {
        log_line("setting failed, config not written");
        return false;
    }
    reload_config();
    return true;
}

void toggle_cat_detect() {
    const bool enable = !g_app.config.cat_detect;
    if (apply_settings({{"auto", "cat_detect", enable ? "1" : "0"}})) {
        log_line(enable ? "cat detect on" : "cat detect off");
    }
}

void toggle_fullscreen_pause() {
    const bool enable = !g_app.config.pause_in_fullscreen;
    if (apply_settings({{"auto", "pause_in_fullscreen", enable ? "1" : "0"}})) {
        log_line(enable ? "fullscreen pause on" : "fullscreen pause off");
    }
}

void set_dim_off() {
    if (apply_settings({{"overlay", "enabled", "0"}})) {
        log_line("overlay off");
    }
}

void set_dim(int alpha) {
    if (apply_settings({{"overlay", "enabled", "1"}, {"overlay", "alpha", std::to_string(alpha)}})) {
        log_line("overlay alpha " + std::to_string(alpha));
    }
}

void set_idle_minutes(int minutes) {
    if (apply_settings({{"auto", "idle_lock_min", std::to_string(minutes)}})) {
        log_line("idle lock min " + std::to_string(minutes));
    }
}

void set_led_color(Rgb color) {
    char hex[7];
    std::snprintf(hex, sizeof(hex), "%02X%02X%02X", color.r, color.g, color.b);
    if (apply_settings({{"led", "normal_color", hex}})) {
        log_line(std::string("led normal color ") + hex);
        if (g_app.config.led_enabled) {
            led_set_normal(g_app.config.led_normal_color);
        }
    }
}

void open_log_folder() {
    ShellExecuteW(g_app.window, L"open", g_app.paths.dir.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void shut_down(const char* reason) {
    if (g_app.shut_down) {
        return;
    }
    g_app.shut_down = true;
    KillTimer(g_app.window, kTimerHealth);
    KillTimer(g_app.window, kTimerIdle);
    hooks_uninstall();
    overlay_hide();
    led_shutdown(g_app.window);
    tray_remove();
    log_line(std::string("exit ") + reason);
}

void on_tray_event(LPARAM event) {
    if (event != WM_RBUTTONUP || hooks_locked()) {
        return;
    }
    reload_config();
    const Config& config = g_app.config;
    const TrayMenuState state{config.cat_detect, config.pause_in_fullscreen, config.overlay_enabled,
                              config.overlay_alpha, config.idle_lock_min,
                              config.led_normal_color};
    const int command = tray_show_menu(g_app.window, state);
    if (command >= kTrayIdleFirst && command <= kTrayIdleLast) {
        set_idle_minutes(kIdleChoices[command - kTrayIdleFirst]);
        return;
    }
    if (command >= kTrayColorFirst && command <= kTrayColorLast) {
        set_led_color(kLedColors[command - kTrayColorFirst].color);
        return;
    }
    switch (command) {
        case kTrayLock:
            lock("tray");
            break;
        case kTrayToggleCat:
            toggle_cat_detect();
            break;
        case kTrayToggleFullscreen:
            toggle_fullscreen_pause();
            break;
        case kTrayDimOff:
            set_dim_off();
            break;
        case kTrayDim20:
            set_dim(kAlpha20);
            break;
        case kTrayDim30:
            set_dim(kAlpha30);
            break;
        case kTrayDim50:
            set_dim(kAlpha50);
            break;
        case kTrayOpenConfig:
            open_config();
            break;
        case kTrayOpenLogFolder:
            open_log_folder();
            break;
        case kTrayExit:
            DestroyWindow(g_app.window);
            break;
        default:
            break;
    }
}

void on_timer(WPARAM id) {
    if (id == kTimerHealth) {
        check_hooks();
    } else if (id == kTimerIdle) {
        on_idle_tick();
    } else if (led_on_timer(g_app.window, id)) {
        return;
    }
#ifdef KK_WATCHDOG
    if (id == kTimerWatchdog) {
        KillTimer(g_app.window, kTimerWatchdog);
        if (hooks_locked()) {
            hooks_unlock();
            on_unlocked("watchdog");
        }
    }
#endif
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (g_app.taskbar_created != 0 && message == g_app.taskbar_created) {
        tray_add(window, WM_KK_TRAY);  // Explorer yeniden başladı
        tray_set_locked(hooks_locked());
        return 0;
    }
    switch (message) {
        case WM_KK_LOCK_REQUEST:
            lock("hotkey");
            return 0;
        case WM_KK_UNLOCKED:
            on_unlocked("code");
            return 0;
        case WM_KK_CAT:
            on_cat();
            return 0;
        case WM_KK_TRAY:
            on_tray_event(lparam);
            return 0;
        case WM_TIMER:
            on_timer(wparam);
            return 0;
        case WM_DISPLAYCHANGE:
            if (hooks_locked()) {
                show_overlay();  // monitör eklendi, çıkarıldı veya çözünürlük değişti
            }
            return DefWindowProcW(window, message, wparam, lparam);
        case WM_DEVICECHANGE:
            // Üst düzey pencereye kayıt olmadan gelir; tak/çıkar ayrımı tarama sırasında yapılır.
            if (wparam == DBT_DEVNODES_CHANGED && g_app.config.led_enabled) {
                led_on_device_change(window, g_app.config.led_normal_color);
            }
            return TRUE;
        case WM_ENDSESSION:
            if (wparam != 0) {
                shut_down("session end");
            }
            return 0;
        case WM_DESTROY:
            shut_down("menu");
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(window, message, wparam, lparam);
    }
}

HWND create_window(HINSTANCE instance) {
    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(window_class);
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(IDI_KK_PAW));
    window_class.lpszClassName = L"CatLockCppWindow";
    if (RegisterClassExW(&window_class) == 0) {
        return nullptr;
    }
    // Message-only pencere TaskbarCreated yayınını almaz: görünmez normal pencere.
    return CreateWindowExW(WS_EX_TOOLWINDOW, window_class.lpszClassName, L"cat-lock-cpp", WS_POPUP,
                           0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
}

void fatal(const wchar_t* text) {
    MessageBoxW(nullptr, text, L"cat-lock-cpp", MB_OK | MB_ICONERROR);
}

}  // namespace

int run_app(HINSTANCE instance) {
    if (!resolve_paths(g_app.paths)) {
        fatal(L"Ayar klasörü oluşturulamadı.");
        return 1;
    }
    log_init(g_app.paths.log);
    log_line("start");
    if (g_app.paths.migrated) {
        log_line("settings migrated from KediKilit");
    }

    g_app.window = create_window(instance);
    if (g_app.window == nullptr) {
        log_line("window create failed");
        fatal(L"Pencere oluşturulamadı.");
        return 1;
    }
    g_app.taskbar_created = RegisterWindowMessageW(L"TaskbarCreated");
    // Yükseltilmiş çalışırken Explorer'ın yayını UIPI'ye takılmasın.
    ChangeWindowMessageFilterEx(g_app.window, g_app.taskbar_created, MSGFLT_ALLOW, nullptr);

    reload_config();
    if (!hooks_install(g_app.window)) {
        log_line("hook install failed");
        fatal(L"Klavye ve fare hook'ları kurulamadı.");
        DestroyWindow(g_app.window);
        return 1;
    }
    tray_add(g_app.window, WM_KK_TRAY);
    if (!g_app.config.has_code()) {
        tray_balloon(kNoCodeText);
    }
    SetTimer(g_app.window, kTimerHealth, kHealthIntervalMs, nullptr);

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}

}  // namespace kk::win

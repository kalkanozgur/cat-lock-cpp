#include "win/tray.h"

#include <shellapi.h>

#include <initializer_list>

#include "win/res/resource.h"

namespace kk::win {

namespace {

NOTIFYICONDATAW g_icon{};
bool g_locked = false;

HICON load_icon(int id) {
    // LR_SHARED: tutamağı sistem yönetir, DestroyIcon gerekmez.
    return static_cast<HICON>(LoadImageW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(id), IMAGE_ICON,
                                         GetSystemMetrics(SM_CXSMICON),
                                         GetSystemMetrics(SM_CYSMICON), LR_SHARED));
}

void apply_state() {
    g_icon.hIcon = load_icon(g_locked ? IDI_KK_PAW_BANNED : IDI_KK_PAW);
    lstrcpynW(g_icon.szTip, g_locked ? L"cat-lock-cpp: kilitli" : L"cat-lock-cpp",
              ARRAYSIZE(g_icon.szTip));
}

}  // namespace

void tray_add(HWND window, UINT callback_message) {
    g_icon.cbSize = sizeof(g_icon);
    g_icon.hWnd = window;
    g_icon.uID = 1;
    g_icon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    g_icon.uCallbackMessage = callback_message;
    apply_state();
    Shell_NotifyIconW(NIM_ADD, &g_icon);
}

void tray_remove() {
    if (g_icon.hWnd != nullptr) {
        Shell_NotifyIconW(NIM_DELETE, &g_icon);
        g_icon.hWnd = nullptr;
    }
}

void tray_set_locked(bool locked) {
    g_locked = locked;
    apply_state();
    g_icon.uFlags = NIF_ICON | NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &g_icon);
}

void tray_balloon(const wchar_t* text) {
    g_icon.uFlags = NIF_INFO;
    g_icon.dwInfoFlags = NIIF_WARNING;
    lstrcpynW(g_icon.szInfoTitle, L"cat-lock-cpp", ARRAYSIZE(g_icon.szInfoTitle));
    lstrcpynW(g_icon.szInfo, text, ARRAYSIZE(g_icon.szInfo));
    Shell_NotifyIconW(NIM_MODIFY, &g_icon);
}

int tray_show_menu(HWND window, const TrayMenuState& state) {
    const HMENU menu = CreatePopupMenu();
    const HMENU dim_menu = CreatePopupMenu();
    const HMENU idle_menu = CreatePopupMenu();
    const HMENU color_menu = CreatePopupMenu();
    if (menu == nullptr || dim_menu == nullptr || idle_menu == nullptr || color_menu == nullptr) {
        for (const HMENU created : {menu, dim_menu, idle_menu, color_menu}) {
            if (created != nullptr) {
                DestroyMenu(created);
            }
        }
        return kTrayNone;
    }
    constexpr const wchar_t* kIdleLabels[] = {L"Kapalı", L"1 dk", L"3 dk", L"5 dk", L"10 dk"};
    static_assert(ARRAYSIZE(kIdleLabels) == ARRAYSIZE(kIdleChoices));
    static_assert(kTrayIdleLast - kTrayIdleFirst + 1 == ARRAYSIZE(kIdleChoices));
    for (int i = 0; i < static_cast<int>(ARRAYSIZE(kIdleChoices)); ++i) {
        AppendMenuW(idle_menu, MF_STRING, static_cast<UINT_PTR>(kTrayIdleFirst + i), kIdleLabels[i]);
        if (kIdleChoices[i] == state.idle_lock_min) {
            CheckMenuRadioItem(idle_menu, kTrayIdleFirst, kTrayIdleLast, kTrayIdleFirst + i,
                               MF_BYCOMMAND);
        }
    }
    static_assert(kTrayColorLast - kTrayColorFirst + 1 == ARRAYSIZE(kLedColors));
    // ini'de elle verilmiş başka bir renk varsa hiçbiri işaretlenmez.
    for (int i = 0; i < static_cast<int>(ARRAYSIZE(kLedColors)); ++i) {
        AppendMenuW(color_menu, MF_STRING, static_cast<UINT_PTR>(kTrayColorFirst + i),
                    kLedColors[i].label);
        if (kLedColors[i].color == state.led_normal_color) {
            CheckMenuRadioItem(color_menu, kTrayColorFirst, kTrayColorLast, kTrayColorFirst + i,
                               MF_BYCOMMAND);
        }
    }
    AppendMenuW(dim_menu, MF_STRING, kTrayDimOff, L"Kapalı");
    AppendMenuW(dim_menu, MF_STRING, kTrayDim20, L"%20");
    AppendMenuW(dim_menu, MF_STRING, kTrayDim30, L"%30");
    AppendMenuW(dim_menu, MF_STRING, kTrayDim50, L"%50");
    // ini'de elle verilmiş başka bir alpha varsa hiçbir seviye işaretlenmez.
    int checked = kTrayNone;
    if (!state.overlay_enabled) {
        checked = kTrayDimOff;
    } else if (state.overlay_alpha == kAlpha20) {
        checked = kTrayDim20;
    } else if (state.overlay_alpha == kAlpha30) {
        checked = kTrayDim30;
    } else if (state.overlay_alpha == kAlpha50) {
        checked = kTrayDim50;
    }
    if (checked != kTrayNone) {
        CheckMenuRadioItem(dim_menu, kTrayDimOff, kTrayDim50, checked, MF_BYCOMMAND);
    }

    AppendMenuW(menu, MF_STRING, kTrayLock, L"Kilitle");
    AppendMenuW(menu, MF_STRING | (state.cat_detect ? MF_CHECKED : MF_UNCHECKED), kTrayToggleCat,
                L"Kedi algılama");
    AppendMenuW(menu, MF_STRING | (state.pause_in_fullscreen ? MF_CHECKED : MF_UNCHECKED),
                kTrayToggleFullscreen, L"Tam ekranda otomatik kilit yok");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(idle_menu), L"Boşta kilit");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(dim_menu), L"Karartma");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(color_menu), L"Klavye ışığı");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kTrayOpenConfig, L"Ayar dosyasını aç");
    AppendMenuW(menu, MF_STRING, kTrayOpenLogFolder, L"Log klasörü");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kTrayExit, L"Çıkış");

    POINT cursor{};
    GetCursorPos(&cursor);
    // Menü dışına tıklanınca kapanması için pencere ön planda olmalı.
    SetForegroundWindow(window);
    const int command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
                                       cursor.x, cursor.y, 0, window, nullptr);
    PostMessageW(window, WM_NULL, 0, 0);
    DestroyMenu(menu);
    return command;
}

}  // namespace kk::win

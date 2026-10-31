#include "win/tray.h"

#include <shellapi.h>

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
    lstrcpynW(g_icon.szTip, g_locked ? L"KediKilit: kilitli" : L"KediKilit",
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
    lstrcpynW(g_icon.szInfoTitle, L"KediKilit", ARRAYSIZE(g_icon.szInfoTitle));
    lstrcpynW(g_icon.szInfo, text, ARRAYSIZE(g_icon.szInfo));
    Shell_NotifyIconW(NIM_MODIFY, &g_icon);
}

int tray_show_menu(HWND window, const TrayMenuState& state) {
    const HMENU menu = CreatePopupMenu();
    const HMENU dim_menu = CreatePopupMenu();
    if (menu == nullptr || dim_menu == nullptr) {
        if (menu != nullptr) {
            DestroyMenu(menu);
        }
        if (dim_menu != nullptr) {
            DestroyMenu(dim_menu);
        }
        return kTrayNone;
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
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(dim_menu), L"Karartma");
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

#pragma once
#include <windows.h>

namespace kk::win {

enum TrayCommand : int {
    kTrayNone = 0,
    kTrayLock = 1,
    kTrayToggleCat,
    kTrayToggleFullscreen,
    kTrayDimOff,
    kTrayDim20,
    kTrayDim30,
    kTrayDim50,
    kTrayIdleFirst,
    kTrayIdleLast = kTrayIdleFirst + 4,  // kIdleChoices ile aynı uzunluk
    kTrayOpenConfig,
    kTrayOpenLogFolder,
    kTrayExit,
};

struct TrayMenuState {
    bool cat_detect;
    bool pause_in_fullscreen;
    bool overlay_enabled;
    int overlay_alpha;
    int idle_lock_min;
};

// Boşta kilit alt menüsü, dakika. 0 = kapalı.
inline constexpr int kIdleChoices[] = {0, 1, 3, 5, 10};

// Karartma alt menüsü: alpha = yüzde * 255 / 100, yuvarlanmış.
inline constexpr int kAlpha20 = 51;
inline constexpr int kAlpha30 = 77;
inline constexpr int kAlpha50 = 128;

void tray_add(HWND window, UINT callback_message);
void tray_remove();
void tray_set_locked(bool locked);
void tray_balloon(const wchar_t* text);

// Modal; seçilen TrayCommand'ı döner. Hook'lar bu sırada çalışmaya devam eder.
int tray_show_menu(HWND window, const TrayMenuState& state);

}  // namespace kk::win

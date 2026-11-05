#pragma once
#include <windows.h>

#include "core/g213.h"

namespace kk::win {

struct LedColorChoice {
    const wchar_t* label;
    Rgb color;
};

// Klavye ışığı alt menüsü: kilit açıldıktan sonraki sabit renk.
inline constexpr LedColorChoice kLedColors[] = {
    {L"Mor", {0x80, 0x00, 0xFF}},       {L"Beyaz", {0xFF, 0xFF, 0xFF}},
    {L"Kırmızı", {0xFF, 0x00, 0x00}},   {L"Turuncu", {0xFF, 0x80, 0x00}},
    {L"Sarı", {0xFF, 0xFF, 0x00}},      {L"Yeşil", {0x00, 0xFF, 0x00}},
    {L"Camgöbeği", {0x00, 0xFF, 0xFF}}, {L"Mavi", {0x00, 0x00, 0xFF}},
    {L"Pembe", {0xFF, 0x00, 0x80}},
};

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
    kTrayColorFirst,
    kTrayColorLast = kTrayColorFirst + 8,  // kLedColors ile aynı uzunluk
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
    Rgb led_normal_color;
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

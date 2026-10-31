#pragma once
#include <windows.h>

namespace kk::win {

enum TrayCommand : int {
    kTrayNone = 0,
    kTrayLock = 1,
    kTrayOpenConfig,
    kTrayOpenLogFolder,
    kTrayExit,
};

void tray_add(HWND window, UINT callback_message);
void tray_remove();
void tray_set_locked(bool locked);
void tray_balloon(const wchar_t* text);

// Modal; seçilen TrayCommand'ı döner. Hook'lar bu sırada çalışmaya devam eder.
int tray_show_menu(HWND window);

}  // namespace kk::win

#include "win/overlay.h"

#include <windows.h>

#include <vector>

namespace kk::win {

namespace {

constexpr const wchar_t* kClassName = L"CatLockCppOverlay";
constexpr DWORD kExStyle = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW |
                           WS_EX_NOACTIVATE;

std::vector<HWND> g_windows;

bool ensure_class(HINSTANCE instance) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW window_class{};
        window_class.cbSize = sizeof(window_class);
        window_class.lpfnWndProc = DefWindowProcW;
        window_class.hInstance = instance;
        window_class.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        window_class.lpszClassName = kClassName;
        registered = RegisterClassExW(&window_class) != 0;
    }
    return registered;
}

BOOL CALLBACK add_monitor(HMONITOR monitor, HDC, LPRECT, LPARAM alpha) {
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) {
        return TRUE;
    }
    const RECT& r = info.rcMonitor;
    const int width = r.right - r.left;
    const int height = r.bottom - r.top;
    const HWND window = CreateWindowExW(kExStyle, kClassName, L"", WS_POPUP, r.left, r.top, width,
                                        height, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (window == nullptr) {
        return TRUE;
    }
    SetLayeredWindowAttributes(window, 0, static_cast<BYTE>(alpha), LWA_ALPHA);
    // Farklı DPI'lı monitörde oluşturma boyutu değişebilir: fiziksel piksellerle yeniden ver.
    SetWindowPos(window, HWND_TOPMOST, r.left, r.top, width, height,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    g_windows.push_back(window);
    return TRUE;
}

}  // namespace

void overlay_show(int alpha) {
    overlay_hide();
    if (alpha <= 0 || !ensure_class(GetModuleHandleW(nullptr))) {
        return;
    }
    EnumDisplayMonitors(nullptr, nullptr, add_monitor, alpha);
}

void overlay_hide() {
    for (const HWND window : g_windows) {
        DestroyWindow(window);
    }
    g_windows.clear();
}

}  // namespace kk::win

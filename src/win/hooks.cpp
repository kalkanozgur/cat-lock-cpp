#include "win/hooks.h"

#include <array>
#include <cstddef>

#include "core/code_matcher.h"

namespace kk::win {

namespace {

struct State {
    HWND notify = nullptr;
    HHOOK keyboard = nullptr;
    HHOOK mouse = nullptr;
    CodeMatcher matcher;
    LockHotkey hotkey = LockHotkey::Both;
    bool locked = false;
    DWORD last_event = 0;
    // Sisteme iletilmiş basmalar. Bu tuşların bırakması kilitliyken de geçirilir;
    // yoksa kilit anında basılı Ctrl/Alt açılıştan sonra takılı kalır.
    std::array<bool, 256> key_down{};
    std::array<bool, 5> button_down{};
};

State g;

bool key_pressed(int vk) noexcept {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

bool is_lock_hotkey(const KBDLLHOOKSTRUCT& key) noexcept {
    if (g.hotkey != LockHotkey::CtrlAltL && key.vkCode == VK_PAUSE) {
        return true;
    }
    return g.hotkey != LockHotkey::Pause && key.vkCode == 'L' && key_pressed(VK_CONTROL) &&
           key_pressed(VK_MENU);
}

LRESULT CALLBACK keyboard_proc(int code, WPARAM message, LPARAM data) {
    if (code != HC_ACTION) {
        return CallNextHookEx(nullptr, code, message, data);
    }
    const auto& key = *reinterpret_cast<const KBDLLHOOKSTRUCT*>(data);
    const bool down = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
    const std::size_t index = key.vkCode & 0xFF;
    g.last_event = GetTickCount();

    if (g.locked) {
        if (down) {
            const KeyEvent event{key.vkCode, key.scanCode, (key.flags & LLKHF_EXTENDED) != 0};
            if (g.matcher.on_keydown(event)) {
                g.locked = false;
                PostMessageW(g.notify, WM_KK_UNLOCKED, 0, 0);
            }
            return 1;
        }
        if (g.key_down[index]) {
            g.key_down[index] = false;
            return CallNextHookEx(nullptr, code, message, data);
        }
        return 1;
    }

    if (down && is_lock_hotkey(key)) {
        PostMessageW(g.notify, WM_KK_LOCK_REQUEST, 0, 0);
        return 1;
    }
    g.key_down[index] = down;
    return CallNextHookEx(nullptr, code, message, data);
}

// Düğme olayı değilse -1.
int button_index(WPARAM message, DWORD mouse_data) noexcept {
    switch (message) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            return 0;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            return 1;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            return 2;
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
            return HIWORD(mouse_data) == XBUTTON2 ? 4 : 3;
        default:
            return -1;
    }
}

bool is_button_up(WPARAM message) noexcept {
    return message == WM_LBUTTONUP || message == WM_RBUTTONUP || message == WM_MBUTTONUP ||
           message == WM_XBUTTONUP;
}

LRESULT CALLBACK mouse_proc(int code, WPARAM message, LPARAM data) {
    if (code != HC_ACTION) {
        return CallNextHookEx(nullptr, code, message, data);
    }
    const auto& mouse = *reinterpret_cast<const MSLLHOOKSTRUCT*>(data);
    const int button = button_index(message, mouse.mouseData);
    g.last_event = GetTickCount();

    if (g.locked) {
        if (button >= 0 && is_button_up(message) && g.button_down[static_cast<std::size_t>(button)]) {
            g.button_down[static_cast<std::size_t>(button)] = false;
            return CallNextHookEx(nullptr, code, message, data);
        }
        return 1;
    }
    if (button >= 0) {
        g.button_down[static_cast<std::size_t>(button)] = !is_button_up(message);
    }
    return CallNextHookEx(nullptr, code, message, data);
}

HHOOK install_one(int type, HOOKPROC proc) noexcept {
    return SetWindowsHookExW(type, proc, GetModuleHandleW(nullptr), 0);
}

}  // namespace

bool hooks_install(HWND notify) {
    g.notify = notify;
    g.last_event = GetTickCount();
    g.keyboard = install_one(WH_KEYBOARD_LL, keyboard_proc);
    g.mouse = install_one(WH_MOUSE_LL, mouse_proc);
    if (g.keyboard == nullptr || g.mouse == nullptr) {
        hooks_uninstall();
        return false;
    }
    return true;
}

bool hooks_reinstall() {
    // Önce yenisi: kilitliyken arada girdi kaçmasın.
    const HHOOK keyboard = install_one(WH_KEYBOARD_LL, keyboard_proc);
    const HHOOK mouse = install_one(WH_MOUSE_LL, mouse_proc);
    if (keyboard != nullptr && g.keyboard != nullptr) {
        UnhookWindowsHookEx(g.keyboard);
    }
    if (mouse != nullptr && g.mouse != nullptr) {
        UnhookWindowsHookEx(g.mouse);
    }
    if (keyboard != nullptr) {
        g.keyboard = keyboard;
    }
    if (mouse != nullptr) {
        g.mouse = mouse;
    }
    g.last_event = GetTickCount();
    return keyboard != nullptr && mouse != nullptr;
}

void hooks_uninstall() {
    if (g.keyboard != nullptr) {
        UnhookWindowsHookEx(g.keyboard);
        g.keyboard = nullptr;
    }
    if (g.mouse != nullptr) {
        UnhookWindowsHookEx(g.mouse);
        g.mouse = nullptr;
    }
    g.locked = false;
}

void hooks_configure(std::string_view code, LockHotkey hotkey) {
    g.matcher.set_code(code);
    g.hotkey = hotkey;
}

void hooks_lock() {
    g.matcher.reset();
    g.locked = true;
}

void hooks_unlock() {
    g.locked = false;
    g.matcher.reset();
}

bool hooks_locked() {
    return g.locked;
}

std::uint32_t hooks_last_event_tick() {
    return g.last_event;
}

}  // namespace kk::win

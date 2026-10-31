#pragma once
// WH_KEYBOARD_LL ve WH_MOUSE_LL. Callback'ler yalnız durum okur/günceller ve
// PostMessage atar: IO, LED, alloc, log, SendMessage yok.
// Hook'lar kuran thread'de çalışır; bu API yalnız ana thread'den çağrılır.
#include <windows.h>

#include <cstdint>
#include <string_view>

#include "core/config.h"

namespace kk::win {

inline constexpr UINT WM_KK_LOCK_REQUEST = WM_APP + 1;  // kısayola basıldı
inline constexpr UINT WM_KK_UNLOCKED = WM_APP + 2;      // kod girildi, kilit açıldı

bool hooks_install(HWND notify);
bool hooks_reinstall();
void hooks_uninstall();

// Tamponu sıfırlar. Yalnız kilit açıkken çağrılır.
void hooks_configure(std::string_view code, LockHotkey hotkey);

void hooks_lock();
void hooks_unlock();
bool hooks_locked();

// GetTickCount tabanlı son hook olayı.
std::uint32_t hooks_last_event_tick();

}  // namespace kk::win

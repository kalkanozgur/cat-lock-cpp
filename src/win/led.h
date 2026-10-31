#pragma once
// G213 aydınlatması, doğrudan HID. G HUB gerekmez. Yalnız ana mesaj döngüsünden
// çağrılır, hook callback'inden asla.
#include <windows.h>

#include "core/g213.h"

namespace kk::win {

void led_lock(HWND window, Rgb normal_color);  // kırmızı nefes
void led_unlock(HWND window);                  // 500 ms yeşil, sonra normal renk
bool led_on_timer(HWND window, WPARAM id);     // LED timer'ıysa true
void led_shutdown(HWND window);                // kilitliyken çıkışta normal renge döner

}  // namespace kk::win

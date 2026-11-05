#pragma once
// G213 aydınlatması, doğrudan HID. G HUB gerekmez. Yalnız ana mesaj döngüsünden
// çağrılır, hook callback'inden asla.
#include <windows.h>

#include "core/g213.h"

namespace kk::win {

void led_lock(HWND window, Rgb normal_color);  // kırmızı nefes, tüm cihazlar senkron
void led_unlock(HWND window);                  // 3× yeşil yanıp sönme, sonra normal renk
void led_set_normal(Rgb color);                // açıkken hemen uygular, animasyondaysa sonda
void led_on_device_change(HWND window, Rgb normal_color);  // takılan cihaz o anki deseni alır
bool led_on_timer(HWND window, WPARAM id);     // LED timer'ıysa true
void led_shutdown(HWND window);                // kilitliyken çıkışta normal renge döner

}  // namespace kk::win

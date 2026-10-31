#pragma once
// Zaman + olay. Kod ve basılan tuşlar asla yazılmaz. Hook callback'inden çağrılmaz.
#include <string>
#include <string_view>

namespace kk::win {

void log_init(std::wstring path);
void log_line(std::string_view message);

}  // namespace kk::win

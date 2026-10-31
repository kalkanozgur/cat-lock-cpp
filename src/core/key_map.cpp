#include "core/key_map.h"

namespace kk {

namespace {

std::optional<int> numpad_digit_from_vk(std::uint32_t code) noexcept {
    switch (code) {
        case vk::kInsert: return 0;
        case vk::kEnd:    return 1;
        case vk::kDown:   return 2;
        case vk::kNext:   return 3;
        case vk::kLeft:   return 4;
        case vk::kClear:  return 5;
        case vk::kRight:  return 6;
        case vk::kHome:   return 7;
        case vk::kUp:     return 8;
        case vk::kPrior:  return 9;
        default:          return std::nullopt;
    }
}

// VK başka bir yazılımca değiştirilmiş olsa da numpad tanınsın: kilit açılamaması
// yanlış bir rakamdan daha kötü. Bu scan kodlarını extended olmadan yalnız numpad üretir.
std::optional<int> numpad_digit_from_scan(std::uint32_t scan) noexcept {
    switch (scan) {
        case 0x52: return 0;
        case 0x4F: return 1;
        case 0x50: return 2;
        case 0x51: return 3;
        case 0x4B: return 4;
        case 0x4C: return 5;
        case 0x4D: return 6;
        case 0x47: return 7;
        case 0x48: return 8;
        case 0x49: return 9;
        default:   return std::nullopt;
    }
}

}  // namespace

std::optional<int> digit_from_key(const KeyEvent& key) noexcept {
    if (key.vk >= vk::kDigit0 && key.vk <= vk::kDigit9) {
        return static_cast<int>(key.vk - vk::kDigit0);
    }
    if (key.vk >= vk::kNumpad0 && key.vk <= vk::kNumpad9) {
        return static_cast<int>(key.vk - vk::kNumpad0);
    }
    if (key.extended) {
        return std::nullopt;
    }
    if (const auto digit = numpad_digit_from_vk(key.vk)) {
        return digit;
    }
    return numpad_digit_from_scan(key.scan);
}

bool is_modifier(std::uint32_t code) noexcept {
    return code == vk::kShift || code == vk::kControl || code == vk::kMenu ||
           code == vk::kLWin || code == vk::kRWin ||
           (code >= vk::kLShift && code <= vk::kRMenu);
}

}  // namespace kk

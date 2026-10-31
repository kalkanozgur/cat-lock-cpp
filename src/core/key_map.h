#pragma once
// (vk, scan, extended) -> rakam | yok. Win32 include yok; VK değerleri burada,
// windows.h ile uyumu tests/test_key_map.cpp derleme anında doğrular.
#include <cstdint>
#include <optional>

namespace kk {

namespace vk {
inline constexpr std::uint32_t kClear = 0x0C;
inline constexpr std::uint32_t kShift = 0x10;
inline constexpr std::uint32_t kControl = 0x11;
inline constexpr std::uint32_t kMenu = 0x12;
inline constexpr std::uint32_t kSpace = 0x20;
inline constexpr std::uint32_t kPrior = 0x21;
inline constexpr std::uint32_t kNext = 0x22;
inline constexpr std::uint32_t kEnd = 0x23;
inline constexpr std::uint32_t kHome = 0x24;
inline constexpr std::uint32_t kLeft = 0x25;
inline constexpr std::uint32_t kUp = 0x26;
inline constexpr std::uint32_t kRight = 0x27;
inline constexpr std::uint32_t kDown = 0x28;
inline constexpr std::uint32_t kInsert = 0x2D;
inline constexpr std::uint32_t kDigit0 = 0x30;
inline constexpr std::uint32_t kDigit9 = 0x39;
inline constexpr std::uint32_t kLWin = 0x5B;
inline constexpr std::uint32_t kRWin = 0x5C;
inline constexpr std::uint32_t kNumpad0 = 0x60;
inline constexpr std::uint32_t kNumpad9 = 0x69;
inline constexpr std::uint32_t kLShift = 0xA0;
inline constexpr std::uint32_t kRMenu = 0xA5;
}  // namespace vk

struct KeyEvent {
    std::uint32_t vk = 0;
    std::uint32_t scan = 0;
    bool extended = false;  // LLKHF_EXTENDED
};

// Üst sıra, NumLock açık numpad ve NumLock kapalı numpad (extended = false).
// Extended gezinme tuşları rakam değildir.
std::optional<int> digit_from_key(const KeyEvent& key) noexcept;

// Shift / Ctrl / Alt / Win (genel, sol ve sağ).
bool is_modifier(std::uint32_t vk) noexcept;

}  // namespace kk

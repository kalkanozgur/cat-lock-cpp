#pragma once
// Logitech G213 aydınlatma paketleri (belgesiz protokol, topluluk kaynaklı).
// Win32 içermez; yazma src/win/led.cpp'dedir.
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace kk {

inline constexpr std::uint16_t kG213VendorId = 0x046D;
inline constexpr std::uint16_t kG213ProductId = 0xC336;
inline constexpr std::uint16_t kG213UsagePage = 0xFF43;  // üretici koleksiyonu
inline constexpr std::size_t kG213PacketSize = 20;

using G213Packet = std::array<std::uint8_t, kG213PacketSize>;

struct Rgb {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    constexpr bool operator==(const Rgb&) const noexcept = default;
};

namespace detail {
inline constexpr std::uint8_t kModeStatic = 0x01;
inline constexpr std::uint8_t kModeBreathe = 0x02;

constexpr G213Packet g213_header(std::uint8_t mode, Rgb color) noexcept {
    // 0x00: tüm bölgeler.
    return {0x11, 0xFF, 0x0C, 0x3A, 0x00, mode, color.r, color.g, color.b};
}

constexpr int hex_digit(char c) noexcept {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
}  // namespace detail

constexpr G213Packet g213_static(Rgb color) noexcept {
    G213Packet packet = detail::g213_header(detail::kModeStatic, color);
    packet[9] = 0x02;
    return packet;
}

constexpr G213Packet g213_breathe(Rgb color, std::uint16_t period_ms) noexcept {
    G213Packet packet = detail::g213_header(detail::kModeBreathe, color);
    packet[9] = static_cast<std::uint8_t>(period_ms >> 8);
    packet[10] = static_cast<std::uint8_t>(period_ms & 0xFF);
    packet[12] = 0x64;
    return packet;
}

// Logitech LED'lerinde orta kırmızı sönük kalır: 8000FF açık mavi görünür, E000FF mor (kullanıcı
// gözlemi, 05.11.2026). Kırmızıya ×1,75 kazanç, 255'te kırpılır. Laptop'a uygulanmaz.
// shortcut: tek nokta (mor) ölçümü; başka ara tonlarda sapma görülürse kanal eğrisini ölç.
constexpr Rgb logitech_tone(Rgb color) noexcept {
    const unsigned red = color.r * 7u / 4u;
    color.r = static_cast<std::uint8_t>(red > 255u ? 255u : red);
    return color;
}

// Tam olarak RRGGBB; önek yok.
constexpr std::optional<Rgb> parse_rgb(std::string_view text) noexcept {
    if (text.size() != 6) {
        return std::nullopt;
    }
    std::array<std::uint8_t, 3> bytes{};
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        const int high = detail::hex_digit(text[i * 2]);
        const int low = detail::hex_digit(text[i * 2 + 1]);
        if (high < 0 || low < 0) {
            return std::nullopt;
        }
        bytes[i] = static_cast<std::uint8_t>(high * 16 + low);
    }
    return Rgb{bytes[0], bytes[1], bytes[2]};
}

}  // namespace kk

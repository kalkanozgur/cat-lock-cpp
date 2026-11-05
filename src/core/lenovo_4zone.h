#pragma once
// Lenovo 4 bölge RGB laptop klavyesi (ITE 8295 vb.), belgesiz protokol, topluluk kaynaklı.
// Üretici koleksiyonu FF89/00CC, 33 baytlık feature raporu. Win32 içermez.
#include <array>
#include <cstddef>
#include <cstdint>

#include "core/g213.h"

namespace kk {

inline constexpr std::uint16_t kLenovoVendorId = 0x048D;  // ITE
inline constexpr std::uint16_t kLenovoUsagePage = 0xFF89;
inline constexpr std::uint16_t kLenovoUsage = 0x00CC;
inline constexpr std::size_t kLenovoPacketSize = 33;

// Nefes hız kademesi: 1 yavaş … 4 hızlı.
inline constexpr std::uint8_t kLenovoSpeedMin = 1;
inline constexpr std::uint8_t kLenovoSpeedMax = 4;

using LenovoPacket = std::array<std::uint8_t, kLenovoPacketSize>;

namespace detail {
inline constexpr std::uint8_t kLenovoReportId = 0xCC;
inline constexpr std::uint8_t kLenovoCommand = 0x16;
inline constexpr std::uint8_t kLenovoEffectStatic = 0x01;
inline constexpr std::uint8_t kLenovoEffectBreathe = 0x03;
inline constexpr std::uint8_t kLenovoBrightnessHigh = 0x02;

constexpr LenovoPacket lenovo_packet(std::uint8_t effect, std::uint8_t speed, Rgb color) noexcept {
    LenovoPacket packet{kLenovoReportId, kLenovoCommand, effect, speed, kLenovoBrightnessHigh};
    for (std::size_t zone = 0; zone < 4; ++zone) {
        packet[5 + zone * 3] = color.r;
        packet[6 + zone * 3] = color.g;
        packet[7 + zone * 3] = color.b;
    }
    return packet;
}
}  // namespace detail

// Dört bölge aynı renk.
constexpr LenovoPacket lenovo_static(Rgb color) noexcept {
    return detail::lenovo_packet(detail::kLenovoEffectStatic, kLenovoSpeedMin, color);
}

// speed aralık dışıysa sınırlanır.
constexpr LenovoPacket lenovo_breathe(Rgb color, std::uint8_t speed) noexcept {
    const std::uint8_t clamped = speed < kLenovoSpeedMin   ? kLenovoSpeedMin
                                 : speed > kLenovoSpeedMax ? kLenovoSpeedMax
                                                           : speed;
    return detail::lenovo_packet(detail::kLenovoEffectBreathe, clamped, color);
}

}  // namespace kk

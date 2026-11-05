#pragma once
// Logitech G502 HERO, HID++ 2.0 uzun rapor (20 bayt), özellik 0x8070 (renkli LED efektleri).
// Win32 içermez; yazma src/win/led_g502.cpp'dedir.
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "core/g213.h"

namespace kk {

inline constexpr std::uint16_t kG502VendorId = 0x046D;
inline constexpr std::uint16_t kG502ProductId = 0xC08B;
inline constexpr std::uint16_t kHidppUsagePage = 0xFF00;
inline constexpr std::size_t kHidppLongSize = 20;
inline constexpr std::uint16_t kHidppColorLedEffects = 0x8070;
inline constexpr std::uint8_t kG502ZoneCount = 2;  // logo + DPI şeridi

using HidppPacket = std::array<std::uint8_t, kHidppLongSize>;

namespace detail {
inline constexpr std::uint8_t kHidppLongId = 0x11;
inline constexpr std::uint8_t kHidppDevice = 0xFF;  // kablolu
inline constexpr std::uint8_t kHidppSoftwareId = 0x0A;
inline constexpr std::uint8_t kHidppError = 0xFF;
// shortcut: efekt index'leri bu farenin listesinden (05.11.2026 sorgusu: 1 sabit, 2 nefes);
// başka model eklenirse getZoneEffectInfo ile sorgula.
inline constexpr std::uint8_t kG502EffectStatic = 1;
inline constexpr std::uint8_t kG502EffectBreathe = 2;
inline constexpr std::uint8_t kSetZoneEffect = 3;

constexpr HidppPacket hidpp_request(std::uint8_t feature_index, std::uint8_t function) noexcept {
    return {kHidppLongId, kHidppDevice, feature_index,
            static_cast<std::uint8_t>((function << 4) | kHidppSoftwareId)};
}

// Kalıcılık baytı 0: yalnız RAM, flash'a yazılmaz.
constexpr HidppPacket g502_zone(std::uint8_t feature_index, std::uint8_t zone, std::uint8_t effect,
                                Rgb color) noexcept {
    HidppPacket packet = hidpp_request(feature_index, kSetZoneEffect);
    packet[4] = zone;
    packet[5] = effect;
    packet[6] = color.r;
    packet[7] = color.g;
    packet[8] = color.b;
    return packet;
}
}  // namespace detail

// IRoot (index 0) getFeature: özelliğin bu cihazdaki index'i.
constexpr HidppPacket hidpp_get_feature(std::uint16_t feature) noexcept {
    HidppPacket packet = detail::hidpp_request(0, 0);
    packet[4] = static_cast<std::uint8_t>(feature >> 8);
    packet[5] = static_cast<std::uint8_t>(feature & 0xFF);
    return packet;
}

// getFeature yanıtı mı? Değilse nullopt. Yanıt hata ya da index 0 (özellik yok) ise 0.
constexpr std::optional<std::uint8_t> hidpp_feature_index(const HidppPacket& response) noexcept {
    const HidppPacket request = hidpp_get_feature(0);
    if (response[0] != detail::kHidppLongId) {
        return std::nullopt;
    }
    if (response[2] == detail::kHidppError && response[3] == request[2] &&
        response[4] == request[3]) {
        return std::uint8_t{0};
    }
    if (response[2] != request[2] || response[3] != request[3]) {
        return std::nullopt;
    }
    return response[4];
}

constexpr HidppPacket g502_static(std::uint8_t feature_index, std::uint8_t zone,
                                  Rgb color) noexcept {
    return detail::g502_zone(feature_index, zone, detail::kG502EffectStatic, color);
}

constexpr HidppPacket g502_breathe(std::uint8_t feature_index, std::uint8_t zone, Rgb color,
                                   std::uint16_t period_ms) noexcept {
    HidppPacket packet = detail::g502_zone(feature_index, zone, detail::kG502EffectBreathe, color);
    packet[9] = static_cast<std::uint8_t>(period_ms >> 8);
    packet[10] = static_cast<std::uint8_t>(period_ms & 0xFF);
    packet[12] = 0x64;  // yoğunluk %100
    return packet;
}

}  // namespace kk

#pragma once
// Kilit/açılış LED desenleri, cihazdan bağımsız. Win32 içermez.
#include <cstdint>

#include "core/g213.h"

namespace kk {

inline constexpr Rgb kLockColor{255, 0, 0};
inline constexpr Rgb kUnlockColor{0, 255, 0};
inline constexpr Rgb kLedOff{0, 0, 0};

// Lenovo nefes süresi ayarlanamaz, yalnız kademe seçilir. Senkron için G213 ve farenin
// süresi bu kademenin ölçülen süresine eşitlenir.
// Kullanıcı kararı: laptop yavaş kademe. Kıyas (05.11.2026): 3200 ms'de laptop, 3100 ms'de
// G213 öne geçiyor → 3150. Donanım nefesleri yine kayar: her kLockResyncBreaths nefeste
// tüm cihazlara nefes komutu yeniden gönderilir, kayma birikmez.
inline constexpr std::uint8_t kLenovoLockSpeed = 1;
inline constexpr std::uint16_t kLockBreathePeriodMs = 3150;
inline constexpr unsigned kLockResyncBreaths = 3;
inline constexpr unsigned kLockResyncMs = kLockBreathePeriodMs * kLockResyncBreaths;

// Açılış: yeşil / kapalı / yeşil / kapalı / yeşil, her adım 150 ms, sonra normal renk.
inline constexpr unsigned kUnlockBlinkCount = 3;
inline constexpr unsigned kUnlockBlinkStepMs = 150;
inline constexpr unsigned kUnlockBlinkSteps = kUnlockBlinkCount * 2 - 1;

constexpr Rgb unlock_blink_color(unsigned step) noexcept {
    return step % 2 == 0 ? kUnlockColor : kLedOff;
}

}  // namespace kk

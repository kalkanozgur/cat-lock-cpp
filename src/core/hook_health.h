#pragma once
// Windows, LowLevelHooksTimeout aşılınca hook'u sessizce kaldırır. Sistem girdi
// görüp hook görmüyorsa hook ölmüş sayılır. Tick'ler 32 bit, 49,7 günde bir taşar.
#include <cstdint>

namespace kk {

inline constexpr std::uint32_t kHookGapMs = 2000;

// a, b'den ne kadar yeni (ms). a daha eskiyse negatif.
constexpr std::int32_t tick_diff(std::uint32_t a, std::uint32_t b) noexcept {
    return static_cast<std::int32_t>(a - b);
}

constexpr bool hook_looks_dead(std::uint32_t last_input_tick, std::uint32_t last_hook_tick,
                               std::uint32_t gap_ms = kHookGapMs) noexcept {
    return tick_diff(last_input_tick, last_hook_tick) > static_cast<std::int32_t>(gap_ms);
}

}  // namespace kk

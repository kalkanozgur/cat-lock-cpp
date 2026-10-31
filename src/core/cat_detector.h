#pragma once
// Tuş olay akışı -> "kedi" kararı. Hook callback'inde çalışır: alloc ve IO yok.
#include <array>
#include <cstdint>

namespace kk {

// Bu süreden uzun basılı tuş eşzamanlı sayılmaz. Oyunda uzun basılı W gibi tuşları
// ve bırakması kaybolan tuşları (Ctrl+Alt+Del, Win+L) saymamak için.
inline constexpr std::uint32_t kCatHeldMaxAgeMs = 3000;

struct CatThresholds {
    int min_simultaneous = 4;
    std::uint32_t window_ms = 250;
    int min_distinct = 6;
};

class CatDetector {
public:
    void configure(const CatThresholds& thresholds) noexcept;
    void reset() noexcept;

    // Karar verilirse true döner ve durumu sıfırlar. tick: olayın ms zamanı.
    // Modifier'lar, Space, WASD ve tuş tekrarları sayılmaz.
    bool on_key(std::uint32_t vk, bool down, std::uint32_t tick) noexcept;

private:
    CatThresholds thresholds_;
    std::array<bool, 256> held_{};
    std::array<bool, 256> seen_{};
    std::array<std::uint32_t, 256> last_press_{};
};

}  // namespace kk

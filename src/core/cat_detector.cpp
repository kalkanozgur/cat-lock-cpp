#include "core/cat_detector.h"

#include <cstddef>

#include "core/key_map.h"

namespace kk {

namespace {

// Oyunlarda sürekli birlikte basılan tuşlar kedi sayılmaz.
bool is_ignored(std::uint32_t code) noexcept {
    switch (code) {
        case vk::kSpace:
        case 'W':
        case 'A':
        case 'S':
        case 'D':
            return true;
        default:
            return is_modifier(code);
    }
}

}  // namespace

void CatDetector::configure(const CatThresholds& thresholds) noexcept {
    thresholds_ = thresholds;
    reset();
}

void CatDetector::reset() noexcept {
    held_.fill(false);
    seen_.fill(false);
}

bool CatDetector::on_key(std::uint32_t vk, bool down, std::uint32_t tick) noexcept {
    if (is_ignored(vk)) {
        return false;
    }
    const std::size_t index = vk & 0xFF;
    if (!down) {
        held_[index] = false;
        return false;
    }
    if (held_[index]) {
        return false;  // tuş tekrarı
    }
    held_[index] = true;
    seen_[index] = true;
    last_press_[index] = tick;

    int simultaneous = 0;
    int distinct = 0;
    for (std::size_t i = 0; i < last_press_.size(); ++i) {
        if (!seen_[i]) {
            continue;
        }
        const std::uint32_t age = tick - last_press_[i];  // işaretsiz: taşmaya dayanıklı
        if (held_[i] && age <= kCatHeldMaxAgeMs) {
            ++simultaneous;
        }
        if (age <= thresholds_.window_ms) {
            ++distinct;
        }
    }
    if (simultaneous >= thresholds_.min_simultaneous || distinct >= thresholds_.min_distinct) {
        reset();
        return true;
    }
    return false;
}

}  // namespace kk

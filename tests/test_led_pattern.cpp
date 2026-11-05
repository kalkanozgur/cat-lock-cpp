#include "core/led_pattern.h"
#include "harness.h"

using kk::Rgb;

TEST(led_pattern_lock_breathe_matches_lenovo_slow) {
    CHECK_EQ(kk::kLockBreathePeriodMs, std::uint16_t{3300});
    CHECK_EQ(kk::kLenovoLockSpeed, std::uint8_t{1});
}

TEST(led_pattern_unlock_blinks_three_times) {
    unsigned green = 0;
    for (unsigned step = 0; step < kk::kUnlockBlinkSteps; ++step) {
        if (kk::unlock_blink_color(step) == kk::kUnlockColor) {
            ++green;
        }
    }
    CHECK_EQ(green, 3u);
    CHECK_EQ(kk::kUnlockBlinkSteps, 5u);
}

TEST(led_pattern_unlock_alternates_and_ends_green) {
    CHECK(kk::unlock_blink_color(0) == kk::kUnlockColor);
    CHECK(kk::unlock_blink_color(1) == kk::kLedOff);
    CHECK(kk::unlock_blink_color(2) == kk::kUnlockColor);
    CHECK(kk::unlock_blink_color(3) == kk::kLedOff);
    CHECK(kk::unlock_blink_color(kk::kUnlockBlinkSteps - 1) == kk::kUnlockColor);
}

TEST(g213_breathe_lock_period_bytes) {
    const auto packet = kk::g213_breathe(kk::kLockColor, kk::kLockBreathePeriodMs);
    CHECK_EQ(packet[9], std::uint8_t{0x0C});
    CHECK_EQ(packet[10], std::uint8_t{0xE4});
}

#include <cstdint>
#include <string_view>

#include "core/cat_detector.h"
#include "core/key_map.h"
#include "harness.h"

using kk::CatDetector;

namespace {

// Tuşlara sırayla basar; biri kararı verirse true.
bool press_all(CatDetector& d, std::string_view keys, std::uint32_t tick, std::uint32_t step = 0) {
    bool decided = false;
    for (const char key : keys) {
        decided = d.on_key(static_cast<std::uint32_t>(key), true, tick) || decided;
        tick += step;
    }
    return decided;
}

void release_all(CatDetector& d, std::string_view keys, std::uint32_t tick) {
    for (const char key : keys) {
        d.on_key(static_cast<std::uint32_t>(key), false, tick);
    }
}

// Her tuşa basıp hemen bırakır: yalnız "farklı tuş" koşulunu sınar.
bool tap_all(CatDetector& d, std::string_view keys, std::uint32_t tick, std::uint32_t step) {
    bool decided = false;
    for (const char key : keys) {
        const auto k = static_cast<std::uint32_t>(key);
        decided = d.on_key(k, true, tick) || decided;
        d.on_key(k, false, tick + 1);
        tick += step;
    }
    return decided;
}

}  // namespace

TEST(cat_three_simultaneous_is_not_cat) {
    CatDetector d;
    CHECK(!press_all(d, "HJK", 1000, 100));
}

TEST(cat_four_simultaneous_is_cat) {
    CatDetector d;
    CHECK(!press_all(d, "HJK", 1000, 300));
    CHECK(d.on_key('L', true, 1900));
}

TEST(cat_modifiers_are_not_counted) {
    CatDetector d;
    CHECK(!d.on_key(kk::vk::kLShift, true, 1000));
    CHECK(!d.on_key(kk::vk::kControl, true, 1000));
    CHECK(!d.on_key(kk::vk::kMenu, true, 1000));
    CHECK(!d.on_key(kk::vk::kLWin, true, 1000));
    CHECK(!press_all(d, "HJK", 1400, 300));
}

TEST(cat_wasd_alone_does_not_trigger) {
    CatDetector d;
    CHECK(!press_all(d, "WASD", 1000));
}

TEST(cat_game_chord_does_not_trigger) {
    CatDetector d;
    CHECK(!d.on_key(kk::vk::kLShift, true, 1000));
    CHECK(!d.on_key(kk::vk::kControl, true, 1000));
    CHECK(!press_all(d, "WASD ", 1000));
    CHECK(!press_all(d, "QER", 1050));
}

TEST(cat_space_is_not_counted_as_held) {
    CatDetector d;
    CHECK(!press_all(d, "HJK", 1000, 300));
    CHECK(!d.on_key(' ', true, 1950));
}

TEST(cat_ignored_keys_are_not_counted_as_distinct) {
    CatDetector d;
    CHECK(!tap_all(d, "BCEFGWASD ", 1000, 10));
}

TEST(cat_wasd_keeps_other_keys_detected) {
    CatDetector d;
    CHECK(!press_all(d, "WASD", 1000));
    CHECK(press_all(d, "HJKL", 1100));
}

TEST(cat_released_keys_do_not_count_as_held) {
    CatDetector d;
    CHECK(!press_all(d, "HJK", 1000, 300));
    release_all(d, "H", 1700);
    CHECK(!d.on_key('L', true, 1800));
}

TEST(cat_key_repeat_is_ignored) {
    CatDetector d;
    CHECK(!d.on_key('H', true, 1000));
    for (std::uint32_t t = 1500; t < 3000; t += 33) {
        CHECK(!d.on_key('H', true, t));
    }
}

TEST(cat_five_distinct_in_window_is_not_cat) {
    CatDetector d;
    CHECK(!tap_all(d, "BCEFG", 1000, 0));
}

TEST(cat_six_distinct_in_window_is_cat) {
    CatDetector d;
    CHECK(tap_all(d, "BCEFGH", 1000, 50));
}

TEST(cat_distinct_outside_window_does_not_count) {
    CatDetector d;
    CHECK(!tap_all(d, "BCEFG", 1000, 50));
    CHECK(!d.on_key('H', true, 1251));
}

TEST(cat_same_key_twice_counts_once) {
    CatDetector d;
    for (std::uint32_t i = 0; i < 20; ++i) {
        CHECK(!d.on_key('H', true, 1000 + i * 10));
        d.on_key('H', false, 1005 + i * 10);
    }
}

TEST(cat_fast_typing_does_not_trigger) {
    // 120 WPM: 100 ms'de bir tuş, 130 ms basılı (rollover), 300 tuş.
    constexpr std::string_view kText = "HIZLI YAZAN BIR INSAN KEDI DEGILDIR ";
    CatDetector d;
    std::uint32_t tick = 1000;
    for (int i = 0; i < 300; ++i) {
        const auto key = static_cast<std::uint32_t>(kText[static_cast<std::size_t>(i) % kText.size()]);
        CHECK(!d.on_key(key, true, tick));
        d.on_key(key, false, tick + 130);
        tick += 100;
    }
}

TEST(cat_long_held_keys_do_not_count) {
    CatDetector d;
    CHECK(!press_all(d, "HJ", 1000));
    CHECK(!d.on_key('K', true, 6000));
    CHECK(!d.on_key('L', true, 6050));
}

TEST(cat_key_with_lost_release_expires) {
    CatDetector d;
    CHECK(!press_all(d, "HJK", 1000));
    CHECK(!press_all(d, "LN", 10000));
    CHECK(!d.on_key('M', true, 10100));
}

TEST(cat_state_resets_after_decision) {
    CatDetector d;
    CHECK(press_all(d, "HJKL", 1000));
    CHECK(!press_all(d, "HJK", 1100, 300));
}

TEST(cat_reset_clears_held_keys) {
    CatDetector d;
    press_all(d, "HJK", 1000);
    d.reset();
    CHECK(!d.on_key('L', true, 1100));
}

TEST(cat_custom_thresholds) {
    CatDetector d;
    d.configure({6, 100, 3});
    CHECK(!press_all(d, "BCEFG", 1000, 200));
    d.reset();
    CHECK(tap_all(d, "BCE", 2000, 50));
}

TEST(cat_tick_wraparound) {
    CatDetector d;
    const std::uint32_t start = 0xFFFFFFF0u;
    CHECK(!press_all(d, "HJK", start, 20));
    CHECK(d.on_key('L', true, start + 60));
}

TEST(cat_non_ascii_vk_is_safe) {
    CatDetector d;
    CHECK(!d.on_key(0x1FF, true, 1000));
    CHECK(!d.on_key(0x1FF, false, 1001));
}

#include "core/config.h"
#include "core/g213.h"
#include "harness.h"

using kk::G213Packet;
using kk::Rgb;

TEST(g213_static_packet_bytes) {
    const G213Packet expected{0x11, 0xFF, 0x0C, 0x3A, 0x00, 0x01, 0x00, 0xFF, 0x00, 0x02};
    CHECK(kk::g213_static(Rgb{0x00, 0xFF, 0x00}) == expected);
}

TEST(g213_breathe_packet_bytes) {
    const G213Packet expected{0x11, 0xFF, 0x0C, 0x3A, 0x00, 0x02, 0xFF, 0x00,
                              0x00, 0x03, 0xE8, 0x00, 0x64};
    CHECK(kk::g213_breathe(Rgb{0xFF, 0x00, 0x00}, 1000) == expected);
}

TEST(g213_packets_are_full_size) {
    CHECK_EQ(kk::g213_static(Rgb{}).size(), std::size_t{20});
    CHECK_EQ(kk::g213_breathe(Rgb{}, 1000).size(), std::size_t{20});
}

TEST(parse_rgb_accepts_both_cases) {
    CHECK(kk::parse_rgb("FFA0b1") == Rgb(0xFF, 0xA0, 0xB1));
    CHECK(kk::parse_rgb("000000") == Rgb(0, 0, 0));
}

TEST(parse_rgb_rejects_bad_values) {
    CHECK(!kk::parse_rgb(""));
    CHECK(!kk::parse_rgb("FFF"));
    CHECK(!kk::parse_rgb("#FFFFF"));
    CHECK(!kk::parse_rgb("FFFFFFF"));
    CHECK(!kk::parse_rgb("GGGGGG"));
}

TEST(config_normal_color_default_and_value) {
    CHECK(kk::parse_config("").config.led_normal_color == Rgb(0x80, 0x00, 0xFF));
    const auto result = kk::parse_config("[led]\nnormal_color=102030\n");
    CHECK(result.issues.empty());
    CHECK(result.config.led_normal_color == Rgb(0x10, 0x20, 0x30));
}

TEST(config_normal_color_invalid_keeps_default) {
    const auto result = kk::parse_config("[led]\nnormal_color=mavi\n");
    CHECK_EQ(result.issues.size(), std::size_t{1});
    CHECK(result.config.led_normal_color == Rgb(0x80, 0x00, 0xFF));
}

TEST(logitech_tone_boosts_mid_red_only) {
    CHECK(kk::logitech_tone(Rgb{0x80, 0x00, 0xFF}) == Rgb(0xE0, 0x00, 0xFF));
    CHECK(kk::logitech_tone(Rgb{0xFF, 0x80, 0x00}) == Rgb(0xFF, 0x80, 0x00));
    CHECK(kk::logitech_tone(Rgb{0x00, 0xFF, 0x00}) == Rgb(0x00, 0xFF, 0x00));
    CHECK(kk::logitech_tone(Rgb{0xC0, 0x10, 0x20}) == Rgb(0xFF, 0x10, 0x20));
}

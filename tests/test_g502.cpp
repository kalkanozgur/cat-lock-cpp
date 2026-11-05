#include "core/g502.h"
#include "harness.h"

using kk::HidppPacket;
using kk::Rgb;

TEST(hidpp_get_feature_bytes) {
    const HidppPacket expected{0x11, 0xFF, 0x00, 0x0A, 0x80, 0x70};
    CHECK(kk::hidpp_get_feature(0x8070) == expected);
}

TEST(hidpp_feature_index_from_response) {
    const HidppPacket ok{0x11, 0xFF, 0x00, 0x0A, 0x02};
    CHECK(kk::hidpp_feature_index(ok) == std::optional<std::uint8_t>{2});
    const HidppPacket error{0x11, 0xFF, 0xFF, 0x00, 0x0A, 0x05};
    CHECK(kk::hidpp_feature_index(error) == std::optional<std::uint8_t>{0});
    const HidppPacket other{0x11, 0xFF, 0x02, 0x3A};
    CHECK(!kk::hidpp_feature_index(other).has_value());
}

// Baytlar 05.11.2026'da gerçek G502 HERO'da doğrulanan demoyla aynı.
TEST(g502_breathe_bytes) {
    const HidppPacket expected{0x11, 0xFF, 0x02, 0x3A, 0x01, 0x02, 0xFF, 0x00,
                               0x00, 0x07, 0xD0, 0x00, 0x64};
    CHECK(kk::g502_breathe(2, 1, Rgb{0xFF, 0, 0}, 2000) == expected);
}

TEST(g502_static_bytes) {
    const HidppPacket expected{0x11, 0xFF, 0x02, 0x3A, 0x00, 0x01, 0x00, 0xFF, 0x00};
    CHECK(kk::g502_static(2, 0, Rgb{0, 0xFF, 0}) == expected);
}

#include "core/lenovo_4zone.h"
#include "harness.h"

using kk::LenovoPacket;
using kk::Rgb;

TEST(lenovo_static_packet_bytes) {
    const LenovoPacket expected{0xCC, 0x16, 0x01, 0x01, 0x02,  //
                                0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
                                0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00};
    CHECK(kk::lenovo_static(Rgb{0x00, 0xFF, 0x00}) == expected);
}

TEST(lenovo_breathe_packet_bytes) {
    const LenovoPacket expected{0xCC, 0x16, 0x03, 0x02, 0x02,  //
                                0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
                                0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00};
    CHECK(kk::lenovo_breathe(Rgb{0xFF, 0x00, 0x00}, 2) == expected);
}

TEST(lenovo_breathe_speed_is_clamped) {
    CHECK_EQ(kk::lenovo_breathe(Rgb{}, 0)[3], std::uint8_t{1});
    CHECK_EQ(kk::lenovo_breathe(Rgb{}, 9)[3], std::uint8_t{4});
}

TEST(lenovo_packet_size_and_tail_zero) {
    const LenovoPacket packet = kk::lenovo_static(Rgb{1, 2, 3});
    CHECK_EQ(packet.size(), std::size_t{33});
    bool tail_zero = true;
    for (std::size_t i = 17; i < packet.size(); ++i) {
        tail_zero = tail_zero && packet[i] == 0;
    }
    CHECK(tail_zero);
}

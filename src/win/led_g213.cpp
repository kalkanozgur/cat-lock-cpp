// Logitech G213: üretici koleksiyonuna 20 baytlık çıkış raporu.
#include "win/hid.h"
#include "win/led_device.h"

namespace kk::win {

namespace {

class G213 final : public LedDevice {
public:
    explicit G213(HANDLE device) noexcept : device_(device) {}
    ~G213() override { CloseHandle(device_); }
    G213(const G213&) = delete;
    G213& operator=(const G213&) = delete;

    bool breathe(Rgb color, std::uint16_t period_ms) override {
        return send(g213_breathe(color, period_ms));
    }
    bool solid(Rgb color) override { return send(g213_static(color)); }

private:
    bool send(const G213Packet& packet) {
        return hid::write_output(device_, packet.data(), packet.size());
    }

    HANDLE device_;
};

bool is_g213_control(const hid::Info& info) {
    return info.vendor_id == kG213VendorId && info.product_id == kG213ProductId &&
           info.usage_page == kG213UsagePage && info.output_length == kG213PacketSize;
}

}  // namespace

std::unique_ptr<LedDevice> open_g213() {
    const HANDLE device = hid::open_first(is_g213_control, GENERIC_WRITE, true);
    if (device == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    return std::make_unique<G213>(device);
}

}  // namespace kk::win

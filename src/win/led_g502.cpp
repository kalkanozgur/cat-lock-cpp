// Logitech G502 HERO: HID++ 2.0, özellik 0x8070, iki bölge aynı renk.
#include "core/g502.h"
#include "win/hid.h"
#include "win/led_device.h"

namespace kk::win {

namespace {

constexpr int kMaxReadsForReply = 8;  // araya fare olay/bildirim raporları girebilir

class G502 final : public LedDevice {
public:
    G502(HANDLE device, std::uint8_t feature_index) noexcept
        : device_(device), feature_index_(feature_index) {}
    ~G502() override { CloseHandle(device_); }
    G502(const G502&) = delete;
    G502& operator=(const G502&) = delete;

    bool breathe(Rgb color, std::uint16_t period_ms) override {
        for (std::uint8_t zone = 0; zone < kG502ZoneCount; ++zone) {
            if (!send(g502_breathe(feature_index_, zone, color, period_ms))) {
                return false;
            }
        }
        return true;
    }
    bool solid(Rgb color) override {
        for (std::uint8_t zone = 0; zone < kG502ZoneCount; ++zone) {
            if (!send(g502_static(feature_index_, zone, color))) {
                return false;
            }
        }
        return true;
    }

private:
    // Yanıt okunmaz: sürücü tamponu taşarsa eski raporları kendisi atar.
    bool send(const HidppPacket& packet) {
        return hid::write_output(device_, packet.data(), packet.size());
    }

    HANDLE device_;
    std::uint8_t feature_index_;
};

bool is_g502_hidpp_long(const hid::Info& info) {
    return info.vendor_id == kG502VendorId && info.product_id == kG502ProductId &&
           info.usage_page == kHidppUsagePage && info.output_length == kHidppLongSize;
}

// Özellik index'i firmware'e göre değişebilir: her açılışta sorulur. Bulunamazsa 0.
std::uint8_t query_led_feature(HANDLE device) {
    const HidppPacket request = hidpp_get_feature(kHidppColorLedEffects);
    if (!hid::write_output(device, request.data(), request.size())) {
        return 0;
    }
    HidppPacket reply{};
    for (int i = 0; i < kMaxReadsForReply; ++i) {
        if (!hid::read_input(device, reply.data(), reply.size())) {
            return 0;
        }
        if (const auto index = hidpp_feature_index(reply)) {
            return *index;
        }
    }
    return 0;
}

}  // namespace

std::unique_ptr<LedDevice> open_g502() {
    const HANDLE device =
        hid::open_first(is_g502_hidpp_long, GENERIC_READ | GENERIC_WRITE, true);
    if (device == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    const std::uint8_t feature_index = query_led_feature(device);
    if (feature_index == 0) {
        CloseHandle(device);
        return nullptr;
    }
    return std::make_unique<G502>(device, feature_index);
}

}  // namespace kk::win

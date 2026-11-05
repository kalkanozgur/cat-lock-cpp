// Lenovo 4 bölge RGB laptop klavyesi: üretici koleksiyonuna 33 baytlık feature raporu.
#include "core/led_pattern.h"
#include "core/lenovo_4zone.h"
#include "win/hid.h"
#include "win/led_device.h"

namespace kk::win {

namespace {

class Lenovo4Zone final : public LedDevice {
public:
    explicit Lenovo4Zone(HANDLE device) noexcept : device_(device) {}
    ~Lenovo4Zone() override { CloseHandle(device_); }
    Lenovo4Zone(const Lenovo4Zone&) = delete;
    Lenovo4Zone& operator=(const Lenovo4Zone&) = delete;

    // Süre ayarlanamaz: kLenovoLockSpeed sabit kademe, diğer cihazlar ona eşitlenir.
    bool breathe(Rgb color, std::uint16_t /*period_ms*/) override {
        return send(lenovo_breathe(color, kLenovoLockSpeed));
    }
    bool solid(Rgb color) override { return send(lenovo_static(color)); }

private:
    bool send(const LenovoPacket& packet) {
        return hid::set_feature(device_, packet.data(), packet.size());
    }

    HANDLE device_;
};

// PID'e bakılmaz: aynı koleksiyon farklı Lenovo modellerinde farklı PID'le gelir.
bool is_lenovo_control(const hid::Info& info) {
    return info.vendor_id == kLenovoVendorId && info.usage_page == kLenovoUsagePage &&
           info.usage == kLenovoUsage && info.feature_length == kLenovoPacketSize;
}

}  // namespace

std::unique_ptr<LedDevice> open_lenovo_4zone() {
    const HANDLE device = hid::open_first(is_lenovo_control, GENERIC_READ | GENERIC_WRITE, true);
    if (device == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    return std::make_unique<Lenovo4Zone>(device);
}

}  // namespace kk::win

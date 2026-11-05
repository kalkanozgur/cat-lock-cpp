#pragma once
// Aydınlatma arka ucu. Her cihaz kendi protokolünü uygular; desen ve zamanlama led.cpp'de.
// Yalnız ana mesaj döngüsünden, hook callback'inden asla.
#include <cstdint>
#include <memory>

#include "core/g213.h"

namespace kk::win {

class LedDevice {
public:
    virtual ~LedDevice() = default;
    virtual bool breathe(Rgb color, std::uint16_t period_ms) = 0;
    virtual bool solid(Rgb color) = 0;
};

// Cihaz yoksa nullptr.
std::unique_ptr<LedDevice> open_g213();

}  // namespace kk::win

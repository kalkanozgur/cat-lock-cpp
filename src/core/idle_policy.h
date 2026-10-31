#pragma once
// Boşta süre -> kilit kararı. Karar verildikten sonra yeni girdi gelene dek tekrar
// vermez: kilit reddedilse veya gözetimsiz açılsa bile her saniye denenmez.
#include <cstdint>

namespace kk {

class IdlePolicy {
public:
    // Süre doldu ve bu boşta kalma için henüz kilitlenmedi mi?
    constexpr bool due(std::uint32_t idle_ms, int idle_lock_min) noexcept {
        if (idle_lock_min <= 0 || idle_ms < static_cast<std::uint32_t>(idle_lock_min) * 60000u) {
            armed_ = true;
            return false;
        }
        return armed_;
    }

    // Askıda (tam ekran) due() true kalır; çağrılmazsa askı bitince kilitlenir.
    constexpr void mark_fired() noexcept { armed_ = false; }

private:
    bool armed_ = true;
};

}  // namespace kk

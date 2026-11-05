#include "win/led.h"

#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "core/led_pattern.h"
#include "win/led_device.h"
#include "win/log.h"

namespace kk::win {

namespace {

constexpr UINT_PTR kTimerRelease = 11;
constexpr UINT_PTR kTimerResync = 12;  // yalnız kilitliyken
constexpr UINT_PTR kTimerRescan = 13;  // cihaz takıldı: koleksiyon bildirimleri dinsin diye bekle
constexpr UINT kRescanDelayMs = 500;

// Desteklenen cihazlar. Hepsi aynı anda aynı deseni alır.
struct Backend {
    const char* name;
    std::unique_ptr<LedDevice> (*open)();
    bool limit_resync;    // ayarı kalıcı hafızaya yazıyor olabilir: kilit başına kLockResyncMax
    bool failure_logged;  // cihaz sonradan takılabilir: her kilitte denenir, bir kez loglanır
};

Backend g_backends[] = {
    {"G213", open_g213, false, false},
    {"Lenovo4Zone", open_lenovo_4zone, true, false},
    {"G502", open_g502, false, false},
};

struct Active {
    Backend* backend;
    std::unique_ptr<LedDevice> device;
};

enum class State { Off, Locked, Releasing };

State g_state = State::Off;
std::vector<Active> g_active;
unsigned g_blink_step = 0;
unsigned g_resync_count = 0;
Rgb g_normal_color;

void log_failure_once(Backend& backend, const char* text) {
    if (!backend.failure_logged) {
        log_line(std::string(text) + " " + backend.name);
        backend.failure_logged = true;
    }
}

bool is_active(const Backend& backend) {
    for (const Active& active : g_active) {
        if (active.backend == &backend) {
            return true;
        }
    }
    return false;
}

// Açık olmayan cihazları açar. Yeni açılan varsa true.
bool open_missing() {
    bool opened = false;
    for (Backend& backend : g_backends) {
        if (is_active(backend)) {
            continue;
        }
        const DWORD started = GetTickCount();
        auto device = backend.open();
        const DWORD elapsed = GetTickCount() - started;
        if (!device) {
            log_failure_once(backend, "LED device not found");
            continue;
        }
        log_line(std::string("LED open ") + backend.name + " ms=" + std::to_string(elapsed));
        g_active.push_back({&backend, std::move(device)});
        opened = true;
    }
    return opened;
}

// Yazma hatasında cihaz bırakılır: çıkarılmış olabilir. Cihaz kalmazsa durum Off.
template <typename Fn>
void broadcast(Fn fn) {
    for (auto it = g_active.begin(); it != g_active.end();) {
        if (fn(*it)) {
            ++it;
        } else {
            log_failure_once(*it->backend, "LED write failed");
            it = g_active.erase(it);
        }
    }
    if (g_active.empty()) {
        g_state = State::Off;
    }
}

void solid_all(Rgb color) {
    broadcast([color](Active& active) { return active.device->solid(color); });
}

// limited: kalıcı hafızaya yazıyor olabilecek cihazlar da dahil mi.
void start_breathe(bool limited) {
    broadcast([limited](Active& active) {
        if (active.backend->limit_resync && !limited) {
            return true;
        }
        return active.device->breathe(kLockColor, kLockBreathePeriodMs);
    });
}

void finish(HWND window) {
    KillTimer(window, kTimerRelease);
    KillTimer(window, kTimerResync);
    solid_all(g_normal_color);
    g_active.clear();
    g_state = State::Off;
}

}  // namespace

void led_lock(HWND window, Rgb normal_color) {
    g_normal_color = normal_color;
    if (g_state == State::Releasing) {
        // Yanıp sönme sürerken yeniden kilit: cihazlar zaten açık.
        KillTimer(window, kTimerRelease);
    } else if (g_state == State::Off) {
        open_missing();
        if (g_active.empty()) {
            return;
        }
    } else {
        return;
    }
    g_state = State::Locked;
    start_breathe(true);
    for (const Active& active : g_active) {
        active.backend->failure_logged = false;
    }
    if (g_state == State::Locked) {
        g_resync_count = 0;
        SetTimer(window, kTimerResync, kLockResyncMs, nullptr);
    }
}

void led_unlock(HWND window) {
    KillTimer(window, kTimerResync);
    if (g_state != State::Locked) {
        return;
    }
    g_blink_step = 0;
    solid_all(unlock_blink_color(0));
    if (g_state == State::Locked) {
        SetTimer(window, kTimerRelease, kUnlockBlinkStepMs, nullptr);
        g_state = State::Releasing;
    }
}

void led_set_normal(Rgb color) {
    g_normal_color = color;
    if (g_state != State::Off) {
        return;  // kilit/yanıp sönme bitince finish() bu rengi yazar
    }
    open_missing();
    solid_all(color);
    g_active.clear();
    g_state = State::Off;
}

void led_on_device_change(HWND window, Rgb normal_color) {
    if (g_state == State::Off) {
        g_normal_color = normal_color;
    }
    // Bir cihaz birden çok HID koleksiyonuyla gelir: bildirimler dinince tek tarama.
    SetTimer(window, kTimerRescan, kRescanDelayMs, nullptr);
}

bool led_on_timer(HWND window, WPARAM id) {
    if (id == kTimerRescan) {
        KillTimer(window, kTimerRescan);
        if (g_state == State::Off) {
            led_set_normal(g_normal_color);  // takılan cihaz kendi varsayılan rengiyle gelir
            return true;
        }
        g_active.clear();  // çıkarılıp takılan cihazın eski tutamacı geçersiz: hepsini yeniden aç
        open_missing();
        if (g_active.empty()) {
            KillTimer(window, kTimerRelease);
            KillTimer(window, kTimerResync);
            g_state = State::Off;
        } else if (g_state == State::Locked) {
            start_breathe(true);  // yeni cihaz da aynı fazdan başlasın
        }
        return true;
    }
    if (id == kTimerResync) {
        if (g_state == State::Locked) {
            // Donanım nefesleri kayar; hepsi aynı anda yeniden başlar.
            start_breathe(++g_resync_count <= kLockResyncMax);
        }
        if (g_state != State::Locked) {
            KillTimer(window, kTimerResync);
        }
        return true;
    }
    if (id != kTimerRelease) {
        return false;
    }
    if (g_state != State::Releasing) {
        KillTimer(window, kTimerRelease);
        return true;
    }
    ++g_blink_step;
    if (g_blink_step < kUnlockBlinkSteps) {
        solid_all(unlock_blink_color(g_blink_step));
        if (g_state == State::Off) {
            KillTimer(window, kTimerRelease);
        }
        return true;
    }
    finish(window);
    return true;
}

void led_shutdown(HWND window) {
    KillTimer(window, kTimerRescan);
    if (g_state != State::Off) {
        finish(window);
    } else {
        KillTimer(window, kTimerRelease);
        KillTimer(window, kTimerResync);
    }
}

}  // namespace kk::win

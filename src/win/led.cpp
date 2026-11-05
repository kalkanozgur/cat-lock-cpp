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

// Desteklenen cihazlar. Hepsi aynı anda aynı deseni alır.
struct Backend {
    const char* name;
    std::unique_ptr<LedDevice> (*open)();
    bool failure_logged;  // cihaz sonradan takılabilir: her kilitte denenir, bir kez loglanır
};

Backend g_backends[] = {
    {"G213", open_g213, false},
    {"Lenovo4Zone", open_lenovo_4zone, false},
};

struct Active {
    Backend* backend;
    std::unique_ptr<LedDevice> device;
};

enum class State { Off, Locked, Releasing };

State g_state = State::Off;
std::vector<Active> g_active;
unsigned g_blink_step = 0;
Rgb g_normal_color;

void log_failure_once(Backend& backend, const char* text) {
    if (!backend.failure_logged) {
        log_line(std::string(text) + " " + backend.name);
        backend.failure_logged = true;
    }
}

void open_all() {
    for (Backend& backend : g_backends) {
        const DWORD started = GetTickCount();
        auto device = backend.open();
        const DWORD elapsed = GetTickCount() - started;
        if (!device) {
            log_failure_once(backend, "LED device not found");
            continue;
        }
        log_line(std::string("LED open ") + backend.name + " ms=" + std::to_string(elapsed));
        g_active.push_back({&backend, std::move(device)});
    }
}

// Yazma hatasında cihaz bırakılır: çıkarılmış olabilir. Cihaz kalmazsa durum Off.
template <typename Fn>
void broadcast(Fn fn) {
    for (auto it = g_active.begin(); it != g_active.end();) {
        if (fn(*it->device)) {
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

void finish(HWND window) {
    KillTimer(window, kTimerRelease);
    broadcast([](LedDevice& device) { return device.solid(g_normal_color); });
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
        open_all();
        if (g_active.empty()) {
            return;
        }
    } else {
        return;
    }
    g_state = State::Locked;
    broadcast([](LedDevice& device) {
        return device.breathe(kLockColor, kLockBreathePeriodMs);
    });
    for (const Active& active : g_active) {
        active.backend->failure_logged = false;
    }
}

void led_unlock(HWND window) {
    if (g_state != State::Locked) {
        return;
    }
    g_blink_step = 0;
    broadcast([](LedDevice& device) { return device.solid(unlock_blink_color(0)); });
    if (g_state == State::Locked) {
        SetTimer(window, kTimerRelease, kUnlockBlinkStepMs, nullptr);
        g_state = State::Releasing;
    }
}

bool led_on_timer(HWND window, WPARAM id) {
    if (id != kTimerRelease) {
        return false;
    }
    if (g_state != State::Releasing) {
        KillTimer(window, kTimerRelease);
        return true;
    }
    ++g_blink_step;
    if (g_blink_step < kUnlockBlinkSteps) {
        const Rgb color = unlock_blink_color(g_blink_step);
        broadcast([color](LedDevice& device) { return device.solid(color); });
        if (g_state == State::Off) {
            KillTimer(window, kTimerRelease);
        }
        return true;
    }
    finish(window);
    return true;
}

void led_shutdown(HWND window) {
    if (g_state != State::Off) {
        finish(window);
    } else {
        KillTimer(window, kTimerRelease);
    }
}

}  // namespace kk::win

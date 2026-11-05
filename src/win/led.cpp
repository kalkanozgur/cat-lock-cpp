#include "win/led.h"

#include <hidsdi.h>
#include <setupapi.h>

#include <string>

#include "core/led_pattern.h"
#include "win/log.h"

namespace kk::win {

namespace {

constexpr UINT_PTR kTimerRelease = 11;
constexpr DWORD kWriteTimeoutMs = 100;  // ana iş parçacığı hook'ları da çalıştırıyor

enum class State { Off, Locked, Releasing };

State g_state = State::Off;
unsigned g_blink_step = 0;
HANDLE g_device = INVALID_HANDLE_VALUE;
Rgb g_normal_color;
bool g_failure_logged = false;

void log_failure_once(const char* text) {
    // Klavye sonradan takılabilir: her kilitte denenir, bir kez loglanır.
    if (!g_failure_logged) {
        log_line(text);
        g_failure_logged = true;
    }
}

bool is_g213_control(const wchar_t* path) {
    // Erişim 0: klavye koleksiyonları dahil öznitelikler okunabilir, cihaz etkilenmez.
    const HANDLE probe = CreateFileW(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                     OPEN_EXISTING, 0, nullptr);
    if (probe == INVALID_HANDLE_VALUE) {
        return false;
    }
    bool match = false;
    HIDD_ATTRIBUTES attributes{};
    attributes.Size = sizeof(attributes);
    PHIDP_PREPARSED_DATA preparsed = nullptr;
    if (HidD_GetAttributes(probe, &attributes) && attributes.VendorID == kG213VendorId &&
        attributes.ProductID == kG213ProductId && HidD_GetPreparsedData(probe, &preparsed)) {
        HIDP_CAPS caps{};
        match = HidP_GetCaps(preparsed, &caps) == HIDP_STATUS_SUCCESS &&
                caps.UsagePage == kG213UsagePage && caps.OutputReportByteLength == kG213PacketSize;
        HidD_FreePreparsedData(preparsed);
    }
    CloseHandle(probe);
    return match;
}

HANDLE open_g213() {
    GUID hid_guid{};
    HidD_GetHidGuid(&hid_guid);
    const HDEVINFO devices =
        SetupDiGetClassDevsW(&hid_guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (devices == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }
    HANDLE device = INVALID_HANDLE_VALUE;
    SP_DEVICE_INTERFACE_DATA item{};
    item.cbSize = sizeof(item);
    alignas(SP_DEVICE_INTERFACE_DETAIL_DATA_W) BYTE buffer[2048];
    auto* const detail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(buffer);
    for (DWORD i = 0; device == INVALID_HANDLE_VALUE &&
                      SetupDiEnumDeviceInterfaces(devices, nullptr, &hid_guid, i, &item);
         ++i) {
        detail->cbSize = sizeof(*detail);
        if (SetupDiGetDeviceInterfaceDetailW(devices, &item, detail, sizeof(buffer), nullptr,
                                             nullptr) &&
            is_g213_control(detail->DevicePath)) {
            device = CreateFileW(detail->DevicePath, GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                 FILE_FLAG_OVERLAPPED, nullptr);
        }
    }
    SetupDiDestroyDeviceInfoList(devices);
    return device;
}

void close_device() {
    if (g_device != INVALID_HANDLE_VALUE) {
        CloseHandle(g_device);
        g_device = INVALID_HANDLE_VALUE;
    }
}

// Zaman aşımlı: cihaz yanıt vermezse ana iş parçacığı ve hook'lar beklemez.
bool write_packet(const G213Packet& packet) {
    OVERLAPPED overlapped{};
    overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (overlapped.hEvent == nullptr) {
        return false;
    }
    bool ok = WriteFile(g_device, packet.data(), static_cast<DWORD>(packet.size()), nullptr,
                        &overlapped) != FALSE;
    if (!ok && GetLastError() == ERROR_IO_PENDING) {
        if (WaitForSingleObject(overlapped.hEvent, kWriteTimeoutMs) != WAIT_OBJECT_0) {
            CancelIo(g_device);
        }
        ok = true;
    }
    DWORD written = 0;
    // CancelIo sonrası da beklenir: overlapped ve paket işlem bitene dek geçerli kalmalı.
    ok = ok && GetOverlappedResult(g_device, &overlapped, &written, TRUE) &&
         written == packet.size();
    CloseHandle(overlapped.hEvent);
    return ok;
}

// Yazma hatasında cihaz bırakılır: klavye çıkarılmış olabilir.
bool send(const G213Packet& packet) {
    if (write_packet(packet)) {
        return true;
    }
    log_failure_once("LED write failed");
    close_device();
    g_state = State::Off;
    return false;
}

}  // namespace

void led_lock(HWND window, Rgb normal_color) {
    g_normal_color = normal_color;
    if (g_state == State::Releasing) {
        // Yanıp sönme sürerken yeniden kilit: cihaz zaten açık.
        KillTimer(window, kTimerRelease);
    } else if (g_state == State::Off) {
        const DWORD started = GetTickCount();
        g_device = open_g213();
        const DWORD elapsed = GetTickCount() - started;
        if (g_device == INVALID_HANDLE_VALUE) {
            log_failure_once("LED device not found");
            return;
        }
        log_line("LED open ms=" + std::to_string(elapsed));
    } else {
        return;
    }
    g_state = State::Locked;
    if (send(g213_breathe(kLockColor, kLockBreathePeriodMs))) {
        g_failure_logged = false;
    }
}

void led_unlock(HWND window) {
    if (g_state != State::Locked) {
        return;
    }
    g_blink_step = 0;
    if (send(g213_static(unlock_blink_color(g_blink_step)))) {
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
        if (!send(g213_static(unlock_blink_color(g_blink_step)))) {
            KillTimer(window, kTimerRelease);  // send durumu Off yaptı
        }
        return true;
    }
    KillTimer(window, kTimerRelease);
    if (send(g213_static(g_normal_color))) {
        close_device();
        g_state = State::Off;
    }
    return true;
}

void led_shutdown(HWND window) {
    KillTimer(window, kTimerRelease);
    if (g_state != State::Off && send(g213_static(g_normal_color))) {
        close_device();
        g_state = State::Off;
    }
}

}  // namespace kk::win

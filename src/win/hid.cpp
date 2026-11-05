#include "win/hid.h"

#include <winioctl.h>  // WIN32_LEAN_AND_MEAN dışarıda bırakıyor; hidclass.h'ye gerekli

#include <hidclass.h>
#include <hidsdi.h>
#include <setupapi.h>

namespace kk::win::hid {

namespace {

constexpr DWORD kWriteTimeoutMs = 100;  // ana iş parçacığı hook'ları da çalıştırıyor

bool matches(const wchar_t* path, Match match) {
    // Erişim 0: klavye koleksiyonları dahil öznitelikler okunabilir, cihaz etkilenmez.
    const HANDLE probe = CreateFileW(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                     OPEN_EXISTING, 0, nullptr);
    if (probe == INVALID_HANDLE_VALUE) {
        return false;
    }
    bool result = false;
    HIDD_ATTRIBUTES attributes{};
    attributes.Size = sizeof(attributes);
    PHIDP_PREPARSED_DATA preparsed = nullptr;
    if (HidD_GetAttributes(probe, &attributes) && HidD_GetPreparsedData(probe, &preparsed)) {
        HIDP_CAPS caps{};
        if (HidP_GetCaps(preparsed, &caps) == HIDP_STATUS_SUCCESS) {
            const Info info{attributes.VendorID,         attributes.ProductID,
                            caps.UsagePage,              caps.Usage,
                            caps.OutputReportByteLength, caps.FeatureReportByteLength};
            result = match(info);
        }
        HidD_FreePreparsedData(preparsed);
    }
    CloseHandle(probe);
    return result;
}

}  // namespace

HANDLE open_first(Match match, DWORD access, bool overlapped) {
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
            matches(detail->DevicePath, match)) {
            device = CreateFileW(detail->DevicePath, access,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                 overlapped ? FILE_FLAG_OVERLAPPED : 0, nullptr);
        }
    }
    SetupDiDestroyDeviceInfoList(devices);
    return device;
}

namespace {

// Zaman aşımlı G/Ç: cihaz yanıt vermezse ana iş parçacığı ve hook'lar beklemez.
template <typename Start>
bool timed_io(HANDLE device, Start start, DWORD* transferred) {
    OVERLAPPED overlapped{};
    overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (overlapped.hEvent == nullptr) {
        return false;
    }
    bool ok = start(&overlapped);
    if (!ok && GetLastError() == ERROR_IO_PENDING) {
        if (WaitForSingleObject(overlapped.hEvent, kWriteTimeoutMs) != WAIT_OBJECT_0) {
            CancelIo(device);
        }
        ok = true;
    }
    // CancelIo sonrası da beklenir: overlapped ve veri işlem bitene dek geçerli kalmalı.
    ok = ok && GetOverlappedResult(device, &overlapped, transferred, TRUE);
    CloseHandle(overlapped.hEvent);
    return ok;
}

}  // namespace

bool write_output(HANDLE device, const std::uint8_t* data, std::size_t size) {
    DWORD written = 0;
    const bool ok = timed_io(
        device,
        [&](OVERLAPPED* overlapped) {
            return WriteFile(device, data, static_cast<DWORD>(size), nullptr, overlapped) !=
                   FALSE;
        },
        &written);
    return ok && written == size;
}

bool read_input(HANDLE device, std::uint8_t* data, std::size_t size) {
    DWORD read = 0;
    const bool ok = timed_io(
        device,
        [&](OVERLAPPED* overlapped) {
            return ReadFile(device, data, static_cast<DWORD>(size), nullptr, overlapped) != FALSE;
        },
        &read);
    return ok && read == size;
}

bool set_feature(HANDLE device, const std::uint8_t* data, std::size_t size) {
    DWORD returned = 0;
    return timed_io(
        device,
        [&](OVERLAPPED* overlapped) {
            // Giriş tamponu yalnız okunur; API imzası const değil.
            return DeviceIoControl(device, IOCTL_HID_SET_FEATURE, const_cast<std::uint8_t*>(data),
                                   static_cast<DWORD>(size), nullptr, 0, nullptr,
                                   overlapped) != FALSE;
        },
        &returned);
}

}  // namespace kk::win::hid

#pragma once
// Ortak HID yardımcıları: cihaz bulma ve zaman aşımlı yazma. Yalnız ana mesaj döngüsünden.
#include <windows.h>

#include <cstddef>
#include <cstdint>

namespace kk::win::hid {

struct Info {
    std::uint16_t vendor_id = 0;
    std::uint16_t product_id = 0;
    std::uint16_t usage_page = 0;
    std::uint16_t usage = 0;
    std::uint16_t output_length = 0;
    std::uint16_t feature_length = 0;
};

using Match = bool (*)(const Info&);

// Eşleşen ilk koleksiyonu açar. Bulamazsa INVALID_HANDLE_VALUE.
// access: GENERIC_WRITE ve/veya GENERIC_READ. overlapped: write_output için true olmalı.
HANDLE open_first(Match match, DWORD access, bool overlapped);

// Çıkış raporu; cihaz yanıt vermezse kWriteTimeoutMs sonra iptal edilir.
bool write_output(HANDLE device, const std::uint8_t* data, std::size_t size);

// Feature raporu (ilk bayt rapor ID'si), aynı zaman aşımıyla. Cihaz overlapped açılmalı.
bool set_feature(HANDLE device, const std::uint8_t* data, std::size_t size);

}  // namespace kk::win::hid

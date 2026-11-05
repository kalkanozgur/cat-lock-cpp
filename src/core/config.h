#pragma once
// .ini ayrıştırma, varsayılanlar, doğrulama. Sorunlar değer içermez: kilit kodu
// yanlış satıra yazılmış olsa bile loga geçemez.
#include <string>
#include <string_view>
#include <vector>

#include "core/g213.h"

namespace kk {

enum class LockHotkey { Pause, CtrlAltL, Both };

struct Config {
    std::string code;  // boşsa kilitleme yapılmaz
    LockHotkey hotkey = LockHotkey::Both;

    bool cat_detect = true;
    int cat_min_simultaneous = 4;
    int cat_window_ms = 250;
    int cat_min_distinct = 6;
    int idle_lock_min = 3;  // 0 = kapalı
    bool pause_in_fullscreen = true;

    bool overlay_enabled = true;
    int overlay_alpha = 77;  // siyah, yaklaşık %30 karartma

    bool led_enabled = true;
    Rgb led_normal_color{0x80, 0x00, 0xFF};  // mor  // kilit açıldıktan sonraki sabit renk

    bool has_code() const noexcept { return !code.empty(); }
};

enum class ConfigIssueKind { MalformedLine, UnknownKey, InvalidValue };

struct ConfigIssue {
    int line = 0;
    ConfigIssueKind kind = ConfigIssueKind::MalformedLine;
    const char* key = nullptr;  // yalnız InvalidValue: bilinen anahtarın adı
};

struct ConfigResult {
    Config config;
    std::vector<ConfigIssue> issues;
};

// Geçersiz değerde varsayılan kalır ve sorun kaydedilir. Geçersiz `code` kodu siler.
// Satır içi yorum yoktur: yalnız satır başındaki ; ve # yorumdur.
ConfigResult parse_config(std::string_view text);

// Loga yazılabilir tek satır.
std::string describe(const ConfigIssue& issue);

// Metinde [section] key değerini değiştirir, diğer satırlara dokunmaz. Anahtar yoksa
// bölümün başına, bölüm de yoksa sona eklenir. section ve key küçük harf verilir.
std::string set_ini_value(std::string_view text, std::string_view section, std::string_view key,
                          std::string_view value);

}  // namespace kk

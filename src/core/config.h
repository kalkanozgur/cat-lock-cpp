#pragma once
// .ini ayrıştırma, varsayılanlar, doğrulama. Sorunlar değer içermez: kilit kodu
// yanlış satıra yazılmış olsa bile loga geçemez.
#include <string>
#include <string_view>
#include <vector>

namespace kk {

enum class LockHotkey { Pause, CtrlAltL, Both };

struct Config {
    std::string code;  // boşsa kilitleme yapılmaz
    LockHotkey hotkey = LockHotkey::Both;

    bool cat_detect = true;
    int cat_min_simultaneous = 4;
    int cat_window_ms = 250;
    int cat_min_distinct = 6;
    int idle_lock_min = 0;  // 0 = kapalı
    bool pause_in_fullscreen = true;

    bool overlay_enabled = true;
    int overlay_alpha = 128;  // siyah, yaklaşık %50 karartma

    bool led_enabled = true;

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

}  // namespace kk

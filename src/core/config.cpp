#include "core/config.h"

#include <charconv>
#include <optional>

#include "core/code_matcher.h"

namespace kk {

namespace {

struct IntKey {
    const char* name;
    int Config::*field;
    int min;
    int max;
};

struct BoolKey {
    const char* name;
    bool Config::*field;
};

constexpr IntKey kIntKeys[] = {
    {"auto.cat_min_simultaneous", &Config::cat_min_simultaneous, 3, 32},
    {"auto.cat_window_ms", &Config::cat_window_ms, 50, 5000},
    {"auto.cat_min_distinct", &Config::cat_min_distinct, 3, 64},
    {"auto.idle_lock_min", &Config::idle_lock_min, 0, 1440},
    {"overlay.alpha", &Config::overlay_alpha, 0, 255},
};

constexpr BoolKey kBoolKeys[] = {
    {"auto.cat_detect", &Config::cat_detect},
    {"auto.pause_in_fullscreen", &Config::pause_in_fullscreen},
    {"overlay.enabled", &Config::overlay_enabled},
    {"led.enabled", &Config::led_enabled},
};

constexpr const char* kCodeKey = "lock.code";
constexpr const char* kHotkeyKey = "lock.hotkey";

std::string_view trim(std::string_view s) noexcept {
    constexpr std::string_view kSpace = " \t\r";
    const auto first = s.find_first_not_of(kSpace);
    if (first == std::string_view::npos) {
        return {};
    }
    return s.substr(first, s.find_last_not_of(kSpace) - first + 1);
}

std::string lower(std::string_view s) {
    std::string out(s);
    for (char& c : out) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return out;
}

std::optional<int> parse_int(std::string_view s) noexcept {
    int value = 0;
    const char* const end = s.data() + s.size();
    const auto [ptr, ec] = std::from_chars(s.data(), end, value);
    if (ec != std::errc{} || ptr != end || s.empty()) {
        return std::nullopt;
    }
    return value;
}

std::optional<LockHotkey> parse_hotkey(std::string_view s) {
    const std::string v = lower(s);
    if (v == "pause") return LockHotkey::Pause;
    if (v == "ctrl_alt_l") return LockHotkey::CtrlAltL;
    if (v == "both") return LockHotkey::Both;
    return std::nullopt;
}

// Değeri uygular. Bilinen anahtarın adını, değer geçersizse *invalid = true döner.
const char* apply(Config& config, const std::string& name, std::string_view value,
                  bool* invalid) {
    *invalid = false;
    if (name == kCodeKey) {
        if (value.empty() || is_valid_code(value)) {
            config.code = std::string(value);
        } else {
            config.code.clear();
            *invalid = true;
        }
        return kCodeKey;
    }
    if (name == kHotkeyKey) {
        if (const auto hotkey = parse_hotkey(value)) {
            config.hotkey = *hotkey;
        } else {
            *invalid = true;
        }
        return kHotkeyKey;
    }
    for (const IntKey& key : kIntKeys) {
        if (name == key.name) {
            const auto parsed = parse_int(value);
            if (parsed && *parsed >= key.min && *parsed <= key.max) {
                config.*key.field = *parsed;
            } else {
                *invalid = true;
            }
            return key.name;
        }
    }
    for (const BoolKey& key : kBoolKeys) {
        if (name == key.name) {
            if (value == "0" || value == "1") {
                config.*key.field = value == "1";
            } else {
                *invalid = true;
            }
            return key.name;
        }
    }
    return nullptr;
}

}  // namespace

ConfigResult parse_config(std::string_view text) {
    ConfigResult result;
    constexpr std::string_view kBom = "\xEF\xBB\xBF";
    if (text.substr(0, kBom.size()) == kBom) {
        text.remove_prefix(kBom.size());
    }

    std::string section;
    int line_number = 0;
    while (!text.empty()) {
        const auto newline = text.find('\n');
        const std::string_view raw = text.substr(0, newline);
        text.remove_prefix(newline == std::string_view::npos ? text.size() : newline + 1);
        ++line_number;

        const std::string_view line = trim(raw);
        if (line.empty() || line.front() == ';' || line.front() == '#') {
            continue;
        }
        if (line.front() == '[') {
            if (line.back() == ']') {
                section = lower(trim(line.substr(1, line.size() - 2)));
            } else {
                section.clear();
                result.issues.push_back({line_number, ConfigIssueKind::MalformedLine, nullptr});
            }
            continue;
        }
        const auto equals = line.find('=');
        if (equals == std::string_view::npos) {
            result.issues.push_back({line_number, ConfigIssueKind::MalformedLine, nullptr});
            continue;
        }

        const std::string name = section + "." + lower(trim(line.substr(0, equals)));
        const std::string_view value = trim(line.substr(equals + 1));
        bool invalid = false;
        const char* const key = apply(result.config, name, value, &invalid);
        if (key == nullptr) {
            result.issues.push_back({line_number, ConfigIssueKind::UnknownKey, nullptr});
        } else if (invalid) {
            result.issues.push_back({line_number, ConfigIssueKind::InvalidValue, key});
        }
    }
    return result;
}

std::string describe(const ConfigIssue& issue) {
    std::string out = "ayar satir " + std::to_string(issue.line) + ": ";
    switch (issue.kind) {
        case ConfigIssueKind::MalformedLine:
            return out + "bozuk satir";
        case ConfigIssueKind::UnknownKey:
            return out + "bilinmeyen anahtar";
        case ConfigIssueKind::InvalidValue:
            return out + "gecersiz deger: " + (issue.key != nullptr ? issue.key : "?");
    }
    return out;
}

}  // namespace kk

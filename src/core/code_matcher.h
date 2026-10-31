#pragma once
// Rakam tamponu ve kod eşleşmesi. Hook callback'inde çalışır: alloc ve IO yok.
#include <array>
#include <cstddef>
#include <string_view>

#include "core/key_map.h"

namespace kk {

inline constexpr std::size_t kMinCodeLen = 4;
inline constexpr std::size_t kMaxCodeLen = 16;

// 4-16 ASCII rakam.
bool is_valid_code(std::string_view code) noexcept;

class CodeMatcher {
public:
    // Geçersiz kod mevcut kodu siler ve false döner.
    bool set_code(std::string_view code) noexcept;
    bool has_code() const noexcept { return len_ != 0; }

    // Girilen rakamlar kodla bitiyorsa true döner ve tamponu sıfırlar. Öndeki yanlış
    // rakamlar eşleşmeyi bozmaz: "9912345" kodu "12345" ise açar.
    bool on_digit(int digit) noexcept;

    // Yalnız KEYDOWN için çağrılır. Rakam tampona girer, modifier yok sayılır,
    // diğer tuşlar tamponu sıfırlar.
    bool on_keydown(const KeyEvent& key) noexcept;

    void reset() noexcept { filled_ = 0; }

private:
    std::array<char, kMaxCodeLen> code_{};
    std::array<char, kMaxCodeLen> buffer_{};
    std::size_t len_ = 0;
    std::size_t filled_ = 0;
};

}  // namespace kk

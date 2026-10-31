#include "core/code_matcher.h"

#include <cstring>

namespace kk {

bool is_valid_code(std::string_view code) noexcept {
    if (code.size() < kMinCodeLen || code.size() > kMaxCodeLen) {
        return false;
    }
    for (const char c : code) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

bool CodeMatcher::set_code(std::string_view code) noexcept {
    reset();
    if (!is_valid_code(code)) {
        len_ = 0;
        return false;
    }
    std::memcpy(code_.data(), code.data(), code.size());
    len_ = code.size();
    return true;
}

bool CodeMatcher::on_digit(int digit) noexcept {
    if (!has_code()) {
        return false;
    }
    if (digit < 0 || digit > 9) {
        reset();
        return false;
    }
    if (filled_ == len_) {
        std::memmove(buffer_.data(), buffer_.data() + 1, len_ - 1);
        filled_ = len_ - 1;
    }
    buffer_[filled_++] = static_cast<char>('0' + digit);
    if (filled_ == len_ && std::memcmp(buffer_.data(), code_.data(), len_) == 0) {
        reset();
        return true;
    }
    return false;
}

bool CodeMatcher::on_keydown(const KeyEvent& key) noexcept {
    if (is_modifier(key.vk)) {
        return false;
    }
    if (const auto digit = digit_from_key(key)) {
        return on_digit(*digit);
    }
    reset();
    return false;
}

}  // namespace kk

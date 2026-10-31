#include <string>
#include <string_view>

#include "core/code_matcher.h"
#include "harness.h"

using kk::CodeMatcher;
using kk::KeyEvent;

namespace {

constexpr std::string_view kCode = "4721";

// Son rakamda eşleşme olup olmadığını döner.
bool type(CodeMatcher& m, std::string_view digits) {
    bool matched = false;
    for (const char c : digits) {
        matched = m.on_digit(c - '0');
    }
    return matched;
}

KeyEvent top(char digit) {
    return KeyEvent{static_cast<std::uint32_t>(digit), 0, false};
}

}  // namespace

TEST(code_validation) {
    CHECK(kk::is_valid_code("1234"));
    CHECK(kk::is_valid_code("1234567890123456"));
    CHECK(!kk::is_valid_code(""));
    CHECK(!kk::is_valid_code("123"));
    CHECK(!kk::is_valid_code("12345678901234567"));
    CHECK(!kk::is_valid_code("12a4"));
    CHECK(!kk::is_valid_code("12 4"));
}

TEST(no_code_never_matches) {
    CodeMatcher m;
    CHECK(!m.has_code());
    CHECK(!type(m, "0000"));
}

TEST(invalid_code_clears_existing) {
    CodeMatcher m;
    CHECK(m.set_code(kCode));
    CHECK(!m.set_code("12"));
    CHECK(!m.has_code());
    CHECK(!type(m, kCode));
}

TEST(exact_match) {
    CodeMatcher m;
    m.set_code(kCode);
    CHECK(!type(m, "472"));
    CHECK(m.on_digit(1));
}

TEST(wrong_leading_digits_do_not_block) {
    CodeMatcher m;
    m.set_code(kCode);
    CHECK(type(m, "9999994721"));
}

TEST(false_start_inside_code_prefix) {
    CodeMatcher m;
    m.set_code("1212");
    CHECK(type(m, "1211212"));
}

TEST(match_fires_once_at_earliest_digit) {
    CodeMatcher m;
    m.set_code("1212");
    CHECK(type(m, "1212"));
    CHECK(!type(m, "12"));
}

TEST(wrong_code_does_not_match) {
    CodeMatcher m;
    m.set_code(kCode);
    CHECK(!type(m, "4722"));
    CHECK(!type(m, "1274"));
}

TEST(buffer_resets_after_match) {
    CodeMatcher m;
    m.set_code(kCode);
    CHECK(type(m, kCode));
    CHECK(!type(m, "721"));
    CHECK(type(m, kCode));
}

TEST(max_length_code) {
    CodeMatcher m;
    CHECK(m.set_code("1234567890123456"));
    CHECK(!type(m, "123456789012345"));
    CHECK(m.on_digit(6));
}

TEST(reset_clears_progress) {
    CodeMatcher m;
    m.set_code(kCode);
    type(m, "472");
    m.reset();
    CHECK(!m.on_digit(1));
}

TEST(out_of_range_digit_resets) {
    CodeMatcher m;
    m.set_code(kCode);
    type(m, "472");
    CHECK(!m.on_digit(10));
    CHECK(!m.on_digit(1));
}

TEST(keydown_non_digit_resets) {
    CodeMatcher m;
    m.set_code(kCode);
    m.on_keydown(top('4'));
    m.on_keydown(top('7'));
    m.on_keydown(KeyEvent{'A', 0x1E, false});
    m.on_keydown(top('2'));
    CHECK(!m.on_keydown(top('1')));
}

TEST(keydown_modifier_does_not_reset) {
    CodeMatcher m;
    m.set_code(kCode);
    m.on_keydown(top('4'));
    m.on_keydown(KeyEvent{kk::vk::kLShift, 0x2A, false});
    m.on_keydown(top('7'));
    m.on_keydown(KeyEvent{kk::vk::kControl, 0x1D, false});
    m.on_keydown(top('2'));
    m.on_keydown(KeyEvent{kk::vk::kLWin, 0x5B, true});
    CHECK(m.on_keydown(top('1')));
}

TEST(keydown_mixed_top_row_and_numpad) {
    CodeMatcher m;
    m.set_code(kCode);
    m.on_keydown(top('4'));
    m.on_keydown(KeyEvent{kk::vk::kNumpad0 + 7, 0x47, false});
    m.on_keydown(KeyEvent{kk::vk::kDown, 0x50, false});
    CHECK(m.on_keydown(KeyEvent{kk::vk::kEnd, 0x4F, false}));
}

TEST(keydown_extended_navigation_resets) {
    CodeMatcher m;
    m.set_code(kCode);
    m.on_keydown(top('4'));
    m.on_keydown(top('7'));
    m.on_keydown(KeyEvent{kk::vk::kDown, 0x50, true});
    CHECK(!m.on_keydown(top('1')));
}

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "core/key_map.h"
#include "harness.h"

using kk::digit_from_key;
using kk::is_modifier;
using kk::KeyEvent;

// core Win32 include etmediği için değerler burada windows.h'a karşı doğrulanır.
static_assert(kk::vk::kClear == VK_CLEAR);
static_assert(kk::vk::kShift == VK_SHIFT);
static_assert(kk::vk::kControl == VK_CONTROL);
static_assert(kk::vk::kMenu == VK_MENU);
static_assert(kk::vk::kPrior == VK_PRIOR);
static_assert(kk::vk::kNext == VK_NEXT);
static_assert(kk::vk::kEnd == VK_END);
static_assert(kk::vk::kHome == VK_HOME);
static_assert(kk::vk::kLeft == VK_LEFT);
static_assert(kk::vk::kUp == VK_UP);
static_assert(kk::vk::kRight == VK_RIGHT);
static_assert(kk::vk::kDown == VK_DOWN);
static_assert(kk::vk::kInsert == VK_INSERT);
static_assert(kk::vk::kLWin == VK_LWIN);
static_assert(kk::vk::kRWin == VK_RWIN);
static_assert(kk::vk::kNumpad0 == VK_NUMPAD0);
static_assert(kk::vk::kNumpad9 == VK_NUMPAD9);
static_assert(kk::vk::kLShift == VK_LSHIFT);
static_assert(kk::vk::kRMenu == VK_RMENU);

namespace {
int digit(std::uint32_t vk, std::uint32_t scan, bool extended) {
    return digit_from_key(KeyEvent{vk, scan, extended}).value_or(-1);
}
}  // namespace

TEST(top_row_digits) {
    CHECK_EQ(digit('0', 0x0B, false), 0);
    CHECK_EQ(digit('5', 0x06, false), 5);
    CHECK_EQ(digit('9', 0x0A, false), 9);
}

TEST(numpad_numlock_on) {
    for (std::uint32_t d = 0; d <= 9; ++d) {
        CHECK_EQ(digit(VK_NUMPAD0 + d, 0x47, false), static_cast<int>(d));
    }
}

TEST(numpad_numlock_off_all_digits) {
    CHECK_EQ(digit(VK_INSERT, 0x52, false), 0);
    CHECK_EQ(digit(VK_END, 0x4F, false), 1);
    CHECK_EQ(digit(VK_DOWN, 0x50, false), 2);
    CHECK_EQ(digit(VK_NEXT, 0x51, false), 3);
    CHECK_EQ(digit(VK_LEFT, 0x4B, false), 4);
    CHECK_EQ(digit(VK_CLEAR, 0x4C, false), 5);
    CHECK_EQ(digit(VK_RIGHT, 0x4D, false), 6);
    CHECK_EQ(digit(VK_HOME, 0x47, false), 7);
    CHECK_EQ(digit(VK_UP, 0x48, false), 8);
    CHECK_EQ(digit(VK_PRIOR, 0x49, false), 9);
}

TEST(extended_navigation_is_not_digit) {
    CHECK_EQ(digit(VK_INSERT, 0x52, true), -1);
    CHECK_EQ(digit(VK_END, 0x4F, true), -1);
    CHECK_EQ(digit(VK_DOWN, 0x50, true), -1);
    CHECK_EQ(digit(VK_NEXT, 0x51, true), -1);
    CHECK_EQ(digit(VK_LEFT, 0x4B, true), -1);
    CHECK_EQ(digit(VK_RIGHT, 0x4D, true), -1);
    CHECK_EQ(digit(VK_HOME, 0x47, true), -1);
    CHECK_EQ(digit(VK_UP, 0x48, true), -1);
    CHECK_EQ(digit(VK_PRIOR, 0x49, true), -1);
}

TEST(numpad_recognized_by_scan_when_vk_is_remapped) {
    CHECK_EQ(digit(0xFF, 0x4C, false), 5);
    CHECK_EQ(digit(0xFF, 0x4C, true), -1);
}

TEST(other_keys_are_not_digits) {
    CHECK_EQ(digit('A', 0x1E, false), -1);
    CHECK_EQ(digit(VK_RETURN, 0x1C, false), -1);
    CHECK_EQ(digit(VK_SPACE, 0x39, false), -1);
    CHECK_EQ(digit(VK_ADD, 0x4E, false), -1);
    CHECK_EQ(digit(VK_SUBTRACT, 0x4A, false), -1);
    CHECK_EQ(digit(VK_DECIMAL, 0x53, false), -1);
    CHECK_EQ(digit(VK_DELETE, 0x53, true), -1);
}

TEST(modifiers) {
    for (const std::uint32_t vk : {VK_SHIFT, VK_CONTROL, VK_MENU, VK_LSHIFT, VK_RSHIFT, VK_LCONTROL,
                                   VK_RCONTROL, VK_LMENU, VK_RMENU, VK_LWIN, VK_RWIN}) {
        CHECK(is_modifier(vk));
    }
    CHECK(!is_modifier('A'));
    CHECK(!is_modifier(VK_CAPITAL));
    CHECK(!is_modifier(VK_NUMLOCK));
    CHECK(!is_modifier(VK_TAB));
}

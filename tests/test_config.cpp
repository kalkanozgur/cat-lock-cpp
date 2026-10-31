#include <fstream>
#include <sstream>
#include <string>

#include "core/config.h"
#include "harness.h"

using kk::Config;
using kk::ConfigIssueKind;
using kk::LockHotkey;
using kk::parse_config;

TEST(empty_text_gives_defaults) {
    const auto r = parse_config("");
    CHECK(r.issues.empty());
    CHECK(!r.config.has_code());
    CHECK(r.config.hotkey == LockHotkey::Both);
    CHECK(r.config.cat_detect);
    CHECK_EQ(r.config.cat_min_simultaneous, 4);
    CHECK_EQ(r.config.cat_window_ms, 250);
    CHECK_EQ(r.config.cat_min_distinct, 6);
    CHECK_EQ(r.config.idle_lock_min, 0);
    CHECK(r.config.pause_in_fullscreen);
    CHECK(r.config.overlay_enabled);
    CHECK_EQ(r.config.overlay_alpha, 77);
    CHECK(r.config.led_enabled);
}

TEST(all_keys_parse) {
    const auto r = parse_config(
        "[lock]\ncode=472118\nhotkey=pause\n"
        "[auto]\ncat_detect=0\ncat_min_simultaneous=5\ncat_window_ms=300\n"
        "cat_min_distinct=7\nidle_lock_min=15\npause_in_fullscreen=0\n"
        "[overlay]\nenabled=0\nalpha=120\n"
        "[led]\nenabled=0\n");
    CHECK(r.issues.empty());
    CHECK_EQ(r.config.code, std::string("472118"));
    CHECK(r.config.hotkey == LockHotkey::Pause);
    CHECK(!r.config.cat_detect);
    CHECK_EQ(r.config.cat_min_simultaneous, 5);
    CHECK_EQ(r.config.cat_window_ms, 300);
    CHECK_EQ(r.config.cat_min_distinct, 7);
    CHECK_EQ(r.config.idle_lock_min, 15);
    CHECK(!r.config.pause_in_fullscreen);
    CHECK(!r.config.overlay_enabled);
    CHECK_EQ(r.config.overlay_alpha, 120);
    CHECK(!r.config.led_enabled);
}

TEST(hotkey_values) {
    CHECK(parse_config("[lock]\nhotkey=ctrl_alt_l").config.hotkey == LockHotkey::CtrlAltL);
    CHECK(parse_config("[lock]\nhotkey=BOTH").config.hotkey == LockHotkey::Both);
    CHECK(parse_config("[lock]\nhotkey=Pause").config.hotkey == LockHotkey::Pause);
}

TEST(formatting_tolerance) {
    const auto r = parse_config(
        "\xEF\xBB\xBF; yorum\r\n# yorum\r\n\r\n  [ LOCK ]  \r\n  Code  =  1234  \r\n[AUTO]\r\nIDLE_LOCK_MIN=5");
    CHECK(r.issues.empty());
    CHECK_EQ(r.config.code, std::string("1234"));
    CHECK_EQ(r.config.idle_lock_min, 5);
}

TEST(empty_code_is_not_an_issue) {
    const auto r = parse_config("[lock]\ncode=\n");
    CHECK(r.issues.empty());
    CHECK(!r.config.has_code());
}

TEST(invalid_codes_are_rejected) {
    for (const char* code : {"123", "12345678901234567", "12a4", "kedi", "12 34"}) {
        const auto r = parse_config(std::string("[lock]\ncode=") + code + "\n");
        CHECK(!r.config.has_code());
        CHECK_EQ(r.issues.size(), std::size_t{1});
        CHECK(r.issues.at(0).kind == ConfigIssueKind::InvalidValue);
    }
}

TEST(later_invalid_code_clears_earlier) {
    const auto r = parse_config("[lock]\ncode=1234\ncode=12\n");
    CHECK(!r.config.has_code());
}

TEST(duplicate_key_last_wins) {
    const auto r = parse_config("[overlay]\nalpha=10\nalpha=20\n");
    CHECK_EQ(r.config.overlay_alpha, 20);
}

TEST(integer_bounds) {
    CHECK_EQ(parse_config("[overlay]\nalpha=0").config.overlay_alpha, 0);
    CHECK_EQ(parse_config("[overlay]\nalpha=255").config.overlay_alpha, 255);
    CHECK_EQ(parse_config("[auto]\ncat_min_simultaneous=3").config.cat_min_simultaneous, 3);
    CHECK_EQ(parse_config("[auto]\nidle_lock_min=1440").config.idle_lock_min, 1440);
}

TEST(invalid_values_keep_default) {
    for (const char* text :
         {"[overlay]\nalpha=256", "[overlay]\nalpha=-1", "[overlay]\nalpha=abc",
          "[overlay]\nalpha=60 ; yorum", "[overlay]\nalpha=", "[overlay]\nalpha=99999999999999"}) {
        const auto r = parse_config(text);
        CHECK_EQ(r.config.overlay_alpha, 77);
        CHECK_EQ(r.issues.size(), std::size_t{1});
    }
    CHECK_EQ(parse_config("[auto]\ncat_min_simultaneous=2").config.cat_min_simultaneous, 4);
    CHECK_EQ(parse_config("[auto]\ncat_min_distinct=2").config.cat_min_distinct, 6);
    CHECK_EQ(parse_config("[auto]\ncat_window_ms=49").config.cat_window_ms, 250);
    CHECK(parse_config("[led]\nenabled=2").config.led_enabled);
    CHECK(parse_config("[led]\nenabled=true").config.led_enabled);
    CHECK(parse_config("[lock]\nhotkey=f12").config.hotkey == LockHotkey::Both);
}

TEST(issues_report_kind_and_line) {
    const auto r = parse_config(
        "duz metin\n"
        "[lock\n"
        "[led]\n"
        "bilinmeyen=1\n"
        "enabled=9\n");
    CHECK_EQ(r.issues.size(), std::size_t{4});
    CHECK(r.issues.at(0).kind == ConfigIssueKind::MalformedLine);
    CHECK_EQ(r.issues.at(0).line, 1);
    CHECK(r.issues.at(1).kind == ConfigIssueKind::MalformedLine);
    CHECK_EQ(r.issues.at(1).line, 2);
    CHECK(r.issues.at(2).kind == ConfigIssueKind::UnknownKey);
    CHECK_EQ(r.issues.at(2).line, 4);
    CHECK(r.issues.at(3).kind == ConfigIssueKind::InvalidValue);
    CHECK_EQ(r.issues.at(3).line, 5);
}

TEST(key_outside_section_is_unknown) {
    const auto r = parse_config("code=1234\n");
    CHECK(!r.config.has_code());
    CHECK(r.issues.at(0).kind == ConfigIssueKind::UnknownKey);
}

TEST(issue_text_never_contains_values) {
    const std::string secret = "90817263";
    const auto r = parse_config(
        "[lock]\ncode=" + secret + "x\n" + secret + "\n" + secret + "=1\nkod=" + secret + "\n");
    CHECK_EQ(r.issues.size(), std::size_t{4});
    for (const auto& issue : r.issues) {
        CHECK(kk::describe(issue).find(secret) == std::string::npos);
    }
}

TEST(describe_names_known_key) {
    const auto r = parse_config("[overlay]\nalpha=abc\n");
    CHECK(kk::describe(r.issues.at(0)).find("overlay.alpha") != std::string::npos);
    CHECK(kk::describe(r.issues.at(0)).find("satir 2") != std::string::npos);
}

TEST(set_ini_replaces_value_and_keeps_other_lines) {
    const std::string text =
        "; yorum\n[lock]\ncode=90817263\n\n[auto]\ncat_detect=1\ncat_window_ms=300\n";
    CHECK_EQ(kk::set_ini_value(text, "auto", "cat_detect", "0"),
             std::string("; yorum\n[lock]\ncode=90817263\n\n[auto]\ncat_detect=0\ncat_window_ms=300\n"));
}

TEST(set_ini_toggle_roundtrip_keeps_code) {
    std::string text = "[lock]\r\ncode=90817263\r\n[auto]\r\ncat_detect=1\r\n";
    text = kk::set_ini_value(text, "auto", "cat_detect", "0");
    CHECK(!parse_config(text).config.cat_detect);
    CHECK_EQ(parse_config(text).config.code, std::string("90817263"));
    text = kk::set_ini_value(text, "auto", "cat_detect", "1");
    CHECK(parse_config(text).config.cat_detect);
    CHECK_EQ(text, std::string("[lock]\r\ncode=90817263\r\n[auto]\r\ncat_detect=1\r\n"));
}

TEST(set_ini_same_key_in_other_section_is_untouched) {
    CHECK_EQ(kk::set_ini_value("[led]\nenabled=1\n[overlay]\nenabled=1\n", "overlay", "enabled", "0"),
             std::string("[led]\nenabled=1\n[overlay]\nenabled=0\n"));
}

TEST(set_ini_ignores_commented_key) {
    CHECK_EQ(kk::set_ini_value("[auto]\n;cat_detect=1\n", "auto", "cat_detect", "0"),
             std::string("[auto]\ncat_detect=0\n;cat_detect=1\n"));
}

TEST(set_ini_replaces_every_duplicate) {
    const auto text = kk::set_ini_value("[auto]\ncat_detect=1\ncat_detect=1\n", "auto", "cat_detect", "0");
    CHECK(!parse_config(text).config.cat_detect);
    CHECK(parse_config(text).issues.empty());
}

TEST(set_ini_adds_missing_key_to_section) {
    CHECK_EQ(kk::set_ini_value("[auto]\nidle_lock_min=5\n", "auto", "cat_detect", "0"),
             std::string("[auto]\ncat_detect=0\nidle_lock_min=5\n"));
}

TEST(set_ini_two_keys_share_new_section) {
    std::string text = "[lock]\ncode=90817263\n";
    text = kk::set_ini_value(text, "overlay", "enabled", "1");
    text = kk::set_ini_value(text, "overlay", "alpha", "51");
    const auto parsed = parse_config(text);
    CHECK(parsed.issues.empty());
    CHECK(parsed.config.overlay_enabled);
    CHECK_EQ(parsed.config.overlay_alpha, 51);
    CHECK_EQ(parsed.config.code, std::string("90817263"));
}

TEST(set_ini_adds_missing_section) {
    CHECK_EQ(kk::set_ini_value("[lock]\r\ncode=90817263", "auto", "cat_detect", "0"),
             std::string("[lock]\r\ncode=90817263\r\n[auto]\r\ncat_detect=0\r\n"));
}

TEST(set_ini_section_header_on_last_line) {
    CHECK_EQ(kk::set_ini_value("[auto]", "auto", "cat_detect", "0"),
             std::string("[auto]\ncat_detect=0\n"));
}

TEST(set_ini_empty_text) {
    CHECK_EQ(kk::set_ini_value("", "auto", "cat_detect", "0"), std::string("[auto]\ncat_detect=0\n"));
}

TEST(set_ini_matches_case_and_spaces) {
    CHECK_EQ(kk::set_ini_value("[ AUTO ]\n  Cat_Detect = 1\n", "auto", "cat_detect", "0"),
             std::string("[ AUTO ]\n  Cat_Detect =0\n"));
}

TEST(set_ini_keeps_bom) {
    const auto text = kk::set_ini_value("\xEF\xBB\xBF[auto]\ncat_detect=1\n", "auto", "cat_detect", "0");
    CHECK_EQ(text, std::string("\xEF\xBB\xBF[auto]\ncat_detect=0\n"));
}

TEST(example_ini_matches_defaults) {
    std::ifstream file(KK_EXAMPLE_INI, std::ios::binary);
    CHECK(file.good());
    std::ostringstream text;
    text << file.rdbuf();
    const auto r = parse_config(text.str());
    const Config d;
    CHECK(r.issues.empty());
    CHECK(!r.config.has_code());
    CHECK(r.config.hotkey == d.hotkey);
    CHECK_EQ(r.config.cat_detect, d.cat_detect);
    CHECK_EQ(r.config.cat_min_simultaneous, d.cat_min_simultaneous);
    CHECK_EQ(r.config.cat_window_ms, d.cat_window_ms);
    CHECK_EQ(r.config.cat_min_distinct, d.cat_min_distinct);
    CHECK_EQ(r.config.idle_lock_min, d.idle_lock_min);
    CHECK_EQ(r.config.pause_in_fullscreen, d.pause_in_fullscreen);
    CHECK_EQ(r.config.overlay_enabled, d.overlay_enabled);
    CHECK_EQ(r.config.overlay_alpha, d.overlay_alpha);
    CHECK_EQ(r.config.led_enabled, d.led_enabled);
}

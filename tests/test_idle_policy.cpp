#include "core/idle_policy.h"
#include "harness.h"

using kk::IdlePolicy;

TEST(idle_disabled_never_due) {
    IdlePolicy p;
    CHECK(!p.due(4000000000u, 0));
}

TEST(idle_due_at_exact_threshold) {
    IdlePolicy p;
    CHECK(!p.due(179999, 3));
    CHECK(p.due(180000, 3));
}

TEST(idle_fires_once_per_idle_period) {
    IdlePolicy p;
    CHECK(p.due(180000, 3));
    p.mark_fired();
    CHECK(!p.due(181000, 3));
    CHECK(!p.due(500000, 3));
}

TEST(idle_input_rearms) {
    IdlePolicy p;
    CHECK(p.due(180000, 3));
    p.mark_fired();
    CHECK(!p.due(10, 3));
    CHECK(p.due(180000, 3));
}

TEST(idle_suspended_stays_due) {
    IdlePolicy p;
    CHECK(p.due(180000, 3));
    CHECK(p.due(240000, 3));
}

TEST(idle_disabling_rearms) {
    IdlePolicy p;
    CHECK(p.due(180000, 3));
    p.mark_fired();
    CHECK(!p.due(200000, 0));
    CHECK(p.due(200000, 3));
}

TEST(idle_max_setting_does_not_overflow) {
    IdlePolicy p;
    CHECK(!p.due(86399999u, 1440));
    CHECK(p.due(86400000u, 1440));
}

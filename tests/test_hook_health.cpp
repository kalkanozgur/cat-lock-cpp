#include "core/hook_health.h"
#include "harness.h"

using kk::hook_looks_dead;

TEST(hook_alive_when_it_saw_latest_input) {
    CHECK(!hook_looks_dead(1000, 1000));
    CHECK(!hook_looks_dead(1000, 1500));
}

TEST(hook_gap_threshold) {
    CHECK(!hook_looks_dead(3000, 1000));
    CHECK(hook_looks_dead(3001, 1000));
}

TEST(hook_gap_across_tick_wraparound) {
    CHECK(hook_looks_dead(5000, 0xFFFFF000u));
    CHECK(!hook_looks_dead(100, 0xFFFFFF00u));
    CHECK(!hook_looks_dead(0xFFFFFF00u, 100));
}

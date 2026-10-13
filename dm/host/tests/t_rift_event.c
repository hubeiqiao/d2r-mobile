// 对纯追逐事件契约做状态、计时、回调和奖励回归；不模拟原版怪物 AI。
// State, clock, callback and reward regressions for the pure chase-event contract; no native creature AI simulation.
#include "../dm_rift_event.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static rf_event live(void) {
    rf_event e = {0};
    CHECK(rf_event_begin(&e, 7, 1));
    CHECK(rf_event_request(&e, 7, 0, 0, 100, 1000));
    CHECK(rf_event_spawned(&e, 7, 42, 110));
    return e;
}
static void test_disabled_and_request(void) {
    rf_event e = {0};
    CHECK(rf_event_begin(&e, 1, 0));
    CHECK(e.state == RF_EVENT_DISABLED);
    CHECK(!rf_event_begin(&e, 1, 1));
    CHECK(!rf_event_begin(&e, 0, 1));
    CHECK(!rf_event_request(&e, 1, 0, 0, 0, 100));
    CHECK(rf_event_begin(&e, 2, 1));
    CHECK(!rf_event_request(&e, 1, 0, 0, 0, 100));
    CHECK(!rf_event_request(&e, 2, 1, 0, 0, 100));
    CHECK(!rf_event_request(&e, 2, 0, 1, 0, 100));
    CHECK(!rf_event_request(&e, 2, 0, 0, -1, 100));
    CHECK(!rf_event_request(&e, 2, 0, 0, 0, 0));
    CHECK(!rf_event_request(&e, 2, 0, 0, 0, RF_EVENT_MAX_DURATION_NS + 1));
    CHECK(!rf_event_request(&e, 2, 0, 0, INT64_MAX, 1));
    CHECK(rf_event_request(&e, 2, 0, 0, 100, 1000));
    CHECK(!rf_event_request(&e, 2, 0, 0, 100, 1000));
    CHECK(!rf_event_spawned(&e, 1, 42, 110));
    CHECK(!rf_event_spawned(&e, 2, 0, 110));
    CHECK(!rf_event_spawned(&e, 2, 42, 99));
    CHECK(rf_event_spawned(&e, 2, 42, 110));
    CHECK(!rf_event_spawned(&e, 2, 43, 110));
}
static void test_clock_and_death(void) {
    rf_event e = live();
    for (int i = 0; i < 1000; ++i) rf_event_tick(&e, 7, 110);
    CHECK(e.state == RF_EVENT_ACTIVE);
    CHECK(!rf_event_server_dead(&e, 7, 0, 1, 111));
    CHECK(!rf_event_server_dead(&e, 7, 43, 1, 111));
    CHECK(!rf_event_server_dead(&e, 7, 42, 0, 111));
    CHECK(!rf_event_server_dead(&e, 6, 42, 1, 111));
    CHECK(!rf_event_server_dead(&e, 7, 42, 1, 99));
    rf_event_tick(&e, 7, 1099);
    CHECK(e.state == RF_EVENT_ACTIVE);
    CHECK(!rf_event_server_dead(&e, 7, 42, 1, 1100));
    CHECK(e.state == RF_EVENT_ESCAPED);
    uint32_t unit = 0;
    CHECK(rf_event_take_cleanup(&e, &unit) && unit == 42);
    CHECK(!rf_event_take_cleanup(&e, &unit));
    CHECK(!rf_event_request(&e, 7, 0, 0, 1200, 100));
    CHECK(!rf_event_reward_begin(&e, 7));
}
static void test_cancel_and_new_run(void) {
    rf_event e = live();
    rf_event_cancel(&e, 6);
    CHECK(e.state == RF_EVENT_ACTIVE);
    CHECK(!rf_event_begin(&e, 8, 1));
    CHECK(e.state == RF_EVENT_CANCELLED);
    CHECK(!rf_event_begin(&e, 8, 1));
    CHECK(!rf_event_server_dead(&e, 7, 42, 1, 200));
    CHECK(rf_event_take_cleanup(&e, NULL));
    rf_event_cancel(&e, 7);
    CHECK(!rf_event_take_cleanup(&e, NULL));
    CHECK(rf_event_begin(&e, 8, 1));
    CHECK(!rf_event_spawned(&e, 7, 99, 200));
    CHECK(!rf_event_reward_result(&e, 7, 1, 1));
    rf_event_cancel(&e, 8);
    CHECK(!rf_event_take_cleanup(&e, NULL));
    CHECK(rf_event_begin(&e, 9, 1));
    CHECK(rf_event_request(&e, 9, 0, 0, 0, 100));
    rf_event_cancel(&e, 9);
    uint32_t unit = 99;
    CHECK(rf_event_take_cleanup(&e, &unit) && unit == 0);
    CHECK(!rf_event_spawned(&e, 9, 99, 1));
}
static void test_spawn_failure_and_timeout(void) {
    rf_event e = {0};
    CHECK(rf_event_begin(&e, 7, 1));
    CHECK(rf_event_request(&e, 7, 0, 0, 100, 1000));
    rf_event_spawn_failure(&e, 6);
    CHECK(e.state == RF_EVENT_SPAWN_REQUESTED);
    rf_event_spawn_failure(&e, 7);
    CHECK(e.state == RF_EVENT_ESCAPED && e.spawn_failed);
    CHECK(!rf_event_request(&e, 7, 0, 0, 110, 1000));
    CHECK(rf_event_take_cleanup(&e, NULL));
    CHECK(rf_event_begin(&e, 8, 1));
    CHECK(rf_event_request(&e, 8, 0, 0, 100, 1000));
    CHECK(!rf_event_spawned(&e, 8, 42, 1100));
    rf_event_tick(&e, 8, 1100);
    CHECK(e.state == RF_EVENT_ESCAPED && !e.spawn_failed);
    CHECK(rf_event_take_cleanup(&e, NULL));
}
static void test_reward_success_and_retry(void) {
    rf_event e = live();
    CHECK(rf_event_server_dead(&e, 7, 42, 1, 200));
    CHECK(!rf_event_server_dead(&e, 7, 42, 1, 201));
    CHECK(!rf_event_reward_begin(&e, 6));
    unsigned a = rf_event_reward_begin(&e, 7);
    CHECK(a == 1 && !rf_event_reward_begin(&e, 7));
    CHECK(!rf_event_reward_result(&e, 6, a, 1));
    CHECK(!rf_event_reward_result(&e, 7, a + 1, 1));
    CHECK(rf_event_reward_result(&e, 7, a, 0));
    CHECK(!rf_event_reward_result(&e, 7, a, 1));
    unsigned b = rf_event_reward_begin(&e, 7);
    CHECK(b == 2);
    CHECK(!rf_event_reward_result(&e, 7, a, 1));
    CHECK(rf_event_reward_result(&e, 7, b, 1));
    CHECK(e.state == RF_EVENT_RESOLVED && e.reward_claimed && !e.reward_failed);
    CHECK(!rf_event_reward_begin(&e, 7));
    CHECK(!rf_event_reward_result(&e, 7, b, 1));
    rf_event_cancel(&e, 7);
    CHECK(e.state == RF_EVENT_RESOLVED);
    CHECK(rf_event_take_cleanup(&e, NULL));
    CHECK(!rf_event_take_cleanup(&e, NULL));
    e = live();
    CHECK(rf_event_server_dead(&e, 7, 42, 1, 200));
    for (unsigned i = 1; i <= RF_EVENT_MAX_REWARDS; ++i) {
        CHECK(rf_event_reward_begin(&e, 7) == i);
        CHECK(rf_event_reward_result(&e, 7, i, 0));
    }
    CHECK(e.state == RF_EVENT_RESOLVED && e.reward_failed && !e.reward_claimed);
    CHECK(!rf_event_reward_begin(&e, 7));
    CHECK(rf_event_take_cleanup(&e, NULL));
    e = live();
    CHECK(rf_event_server_dead(&e, 7, 42, 1, 200));
    a = rf_event_reward_begin(&e, 7);
    CHECK(rf_event_reward_result(&e, 7, a, -1));
    CHECK(e.state == RF_EVENT_RESOLVED && e.reward_failed && !e.reward_claimed);
    CHECK(!rf_event_reward_begin(&e, 7));
    CHECK(!rf_event_reward_result(&e, 7, a, 1));
    CHECK(rf_event_take_cleanup(&e, NULL));
    CHECK(!rf_event_take_cleanup(&e, NULL));
    e = live();
    CHECK(rf_event_server_dead(&e, 7, 42, 1, 200));
    a = rf_event_reward_begin(&e, 7);
    rf_event_cancel(&e, 7);
    CHECK(!rf_event_reward_result(&e, 7, a, 1));
    CHECK(!rf_event_reward_begin(&e, 7));
    CHECK(rf_event_take_cleanup(&e, NULL));
}
int main(void) {
    test_disabled_and_request(); test_clock_and_death();
    test_cancel_and_new_run(); test_spawn_failure_and_timeout();
    test_reward_success_and_retry();
    puts("rift event: 5 behavioral suites passed (native adapter unavailable)");
    return 0;
}

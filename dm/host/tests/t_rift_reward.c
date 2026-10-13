// 奖励计划与发放结果的宿主策略回归；不模拟原生物品工厂。
// Host policy regression for reward plans and outcomes; no native item-factory simulation.
#include "../dm_rift_reward.h"
#include <stdio.h>

static int failures;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); ++failures; } } while (0)
struct fixture { unsigned random_calls, grants, fail_first; int material; };
static unsigned random_target(void *ctx) {
    struct fixture *f = ctx;
    ++f->random_calls;
    return 3;
}
static int grant_item(int material, void *ctx) {
    struct fixture *f = ctx;
    ++f->grants;
    f->material = material;
    return f->grants > f->fail_first;
}
static unsigned invalid_random(void *ctx) { (void)ctx; return 5; }
static int ambiguous_grant(int material, void *ctx) {
    struct fixture *f = ctx;
    (void)material;
    ++f->grants;
    return RF_GRANT_AMBIGUOUS;
}
struct recursive_fixture { rf_reward_queue *q; unsigned calls; };
static int recursive_grant(int material, void *ctx) {
    struct recursive_fixture *f = ctx;
    (void)material;
    ++f->calls;
    CHECK(rf_reward_step(f->q, recursive_grant, f) == RF_REWARD_IDLE);
    return 1;
}
int main(void) {
    rf_reward_queue q = {0};
    struct fixture f = {0};
    CHECK(!rf_reward_begin(&q, 1, 0, 0, 1, random_target, &f));
    CHECK(!rf_reward_begin(&q, 1, 101, 0, 1, random_target, &f));
    CHECK(!rf_reward_begin(&q, 1, 100, -1, 1, random_target, &f));
    CHECK(!rf_reward_begin(&q, 1, 100, 6, 1, random_target, &f));
    CHECK(!q.prepared && f.random_calls == 0);
    CHECK(!rf_reward_begin(&q, 1, 100, 0, 1, invalid_random, &f));
    CHECK(!q.prepared);
    CHECK(!rf_reward_begin(&q, 1, 100, 0, 1, 0, 0));
    for (int tier = 1; tier <= 100; ++tier) {
        q = (rf_reward_queue){0};
        CHECK(rf_reward_begin(&q, 1, tier, 0, 1, random_target, &f));
        CHECK(q.count == (tier <= 40 ? 1u : tier <= 80 ? 2u : 3u));
        CHECK(q.material == 4 && q.count <= 3);
    }
    q = (rf_reward_queue){0}; f = (struct fixture){0};
    CHECK(rf_reward_begin(&q, 2, 100, 5, 0, random_target, &f));
    CHECK(q.count == 1 && q.material == 4 && f.random_calls == 1);
    CHECK(!rf_reward_complete(&q, 9, 1));
    CHECK(!rf_reward_complete(&q, 2, 0));
    CHECK(!rf_reward_pending(&q) && q.count == 0);
    CHECK(!rf_reward_complete(&q, 2, 1));
    CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_IDLE && f.grants == 0);
    q = (rf_reward_queue){0}; f = (struct fixture){.fail_first = 1};
    CHECK(rf_reward_begin(&q, 3, 100, 5, 1, random_target, &f));
    CHECK(f.random_calls == 0 && q.material == 5);
    CHECK(!rf_reward_begin(&q, 3, 99, 2, 1, random_target, &f));
    CHECK(rf_reward_complete(&q, 3, 1));
    CHECK(rf_reward_step(&q, 0, &f) == RF_REWARD_IDLE && q.attempts == 0);
    CHECK(!rf_reward_begin(&q, 4, 99, 2, 1, random_target, &f));
    CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_RETRY);
    CHECK(q.index == 0 && q.attempts == 1 && f.material == 5);
    CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_SUCCESS);
    CHECK(q.index == 1 && q.attempts == 0);
    CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_SUCCESS);
    CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_SUCCESS);
    CHECK(q.successes == 7 && q.failures == 0 && f.grants == 4);
    CHECK(!rf_reward_complete(&q, 3, 1));
    CHECK(!rf_reward_begin(&q, 3, 100, 5, 1, random_target, &f));
    CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_IDLE && f.grants == 4);
    q = (rf_reward_queue){0}; f = (struct fixture){.fail_first = 99};
    CHECK(rf_reward_begin(&q, 4, 100, 0, 1, random_target, &f));
    CHECK(rf_reward_complete(&q, 4, 1));
    for (unsigned item = 0; item < 3; ++item) {
        CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_RETRY);
        CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_RETRY);
        CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_FAILED);
    }
    CHECK(!rf_reward_pending(&q) && q.failures == 7 && q.successes == 0);
    CHECK(f.grants == 9 && f.random_calls == 1 && f.material == 4);
    CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_IDLE && f.grants == 9);
    CHECK(rf_reward_begin(&q, 5, 41, 2, 1, random_target, &f));
    CHECK(rf_reward_complete(&q, 5, 1));
    rf_reward_cancel(&q);
    CHECK(!rf_reward_pending(&q));
    CHECK(rf_reward_step(&q, grant_item, &f) == RF_REWARD_IDLE && f.grants == 9);
    CHECK(!rf_reward_complete(&q, 5, 1));
    CHECK(!rf_reward_begin(&q, 5, 100, 2, 1, random_target, &f));
    q = (rf_reward_queue){0};
    struct recursive_fixture recursive = {.q = &q};
    CHECK(rf_reward_begin(&q, 6, 1, 1, 1, 0, 0));
    CHECK(rf_reward_complete(&q, 6, 1));
    CHECK(rf_reward_step(&q, recursive_grant, &recursive) == RF_REWARD_SUCCESS);
    CHECK(recursive.calls == 1 && q.successes == 1 && !q.busy);
    q = (rf_reward_queue){0}; f = (struct fixture){0};
    CHECK(rf_reward_begin(&q, 7, 1, 1, 1, 0, 0));
    CHECK(rf_reward_complete(&q, 7, 1));
    CHECK(rf_reward_step(&q, ambiguous_grant, &f) == RF_REWARD_FAILED);
    CHECK(q.failures == 1 && q.index == 1 && f.grants == 1);
    CHECK(rf_reward_step(&q, ambiguous_grant, &f) == RF_REWARD_IDLE && f.grants == 1);
    printf("rift reward policy: %d failures\n", failures);
    return failures != 0;
}

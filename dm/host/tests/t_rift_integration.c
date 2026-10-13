// 组合生产源码回归：奖励分类/队列、待发确认保护、单位一次增强；不调用原生地址。
// Combined production-source regression: reward classification/queue, pending-confirm guard, once-only scaling; no native calls.
#include "../dm_rift.c"
#include <assert.h>
void (*g_dm_pre_commit_hook[4])(const char *);
uint64_t dm_fs_commit_count(void) { return 0; }
static int saves;
int dm_request_save(void) { saves++; return 1; }
static int calls, outcome;
static int grant(int material, void *ctx) { (void)ctx; assert(material == 3); calls++; return outcome; }
static int scaled, scale_result = 1;
static int scale_apply(void *ctx) { (void)ctx; scaled++; return scale_result; }
int main(void) {
    uint8_t unit[0x40] = {0}; uint64_t it = (uint64_t)(uintptr_t)unit;
    assert(!reward_player_ready(0)); assert(!reward_player_ready(it));
    *(uint32_t *)(unit + 0xc) = 17; assert(!reward_player_ready(it));
    *(uint32_t *)(unit + 0xc) = 1; assert(reward_player_ready(it));
    assert(reward_item_result(0) == RF_GRANT_REJECTED);
    assert(reward_item_result(it) == RF_GRANT_AMBIGUOUS);
    *(uint32_t *)unit = 4; *(uint32_t *)(unit + 0xc) = 3; *(uint64_t *)(unit + 0x10) = 1;
    assert(reward_item_result(it) == RF_GRANT_CONFIRMED);
    *(uint32_t *)(unit + 0xc) = 0; assert(reward_item_result(it) == RF_GRANT_AMBIGUOUS);
    assert(rf_reward_begin(&g_rewards, 7, 100, 3, 1, 0, 0));
    assert(rf_reward_complete(&g_rewards, 7, 1)); atomic_store(&g_rewards_pending, 1); assert(dm_rift_menu_pending());
    rewards_pump(0, it); assert(!g_rewards.attempts && !saves);
    *(uint32_t *)(unit + 0xc) = 17; rewards_pump(0, it); assert(!g_rewards.attempts && !saves);
    atomic_store(&g_run_seq, 7); atomic_store(&K.st, ST_DONE); K.tier = R.unlock = 100; L.nd_lv = 125;
    dm_rift_menu_start(100); dm_rift_menu_confirm(); assert(!atomic_load(&M.req_start));
    dm_rift_menu_t m; dm_rift_menu_peek(&m); assert(m.reward_pending);
    outcome = RF_GRANT_REJECTED; assert(rewards_step(grant, 0) == RF_REWARD_RETRY);
    outcome = RF_GRANT_CONFIRMED; assert(rewards_step(grant, 0) == RF_REWARD_SUCCESS);
    outcome = RF_GRANT_AMBIGUOUS; assert(rewards_step(grant, 0) == RF_REWARD_FAILED);
    outcome = RF_GRANT_CONFIRMED; assert(rewards_step(grant, 0) == RF_REWARD_SUCCESS);
    assert(calls == 4 && !atomic_load(&g_rewards_pending) && !dm_rift_menu_pending()); assert(g_rewards.successes == 5 && g_rewards.failures == 2);
    assert(rewards_step(grant, 0) == RF_REWARD_IDLE && calls == 4);
    dm_rift_menu_confirm(); assert(atomic_exchange(&M.req_start, 0) == 100);
    atomic_store(&K.st, ST_RUN); K.nmaps = 1; K.maps[0] = 7;
    assert(!scale_once(1, 8, scale_apply, 0));
    scale_result = 0; assert(!scale_once(1, 7, scale_apply, 0)); assert(!g_scaled_ids.ids[(1u * 2654435761u) & 4095]); scale_result = 1; assert(scale_once(1, 7, scale_apply, 0)); assert(!scale_once(1, 7, scale_apply, 0));
    for (uint32_t i = 2; i <= 4096; i++) assert(scale_once(i, 7, scale_apply, 0));
    assert(!scale_once(4097, 7, scale_apply, 0) && g_scaled_ids.overflow); assert(!scale_once(1, 7, scale_apply, 0));
    atomic_fetch_add(&g_run_seq, 1); assert(scale_once(1, 7, scale_apply, 0));
    atomic_store(&K.st, ST_DONE); assert(!scale_once(2, 7, scale_apply, 0));
    setenv("DM_RIFT_ENCOUNTERS", "1", 1); setenv("DM_RIFT_REPEAT_REWARDS", "1", 1);
    atomic_store(&g_reward_target_chosen, 0); unsetenv("DM_RIFT_REWARD_TARGET"); dm_rift_menu_select();
    dm_rift_menu_reward_target(1); assert(dm_rift_menu(&m)); assert(m.reward_mode && m.reward_target == 1);
    dm_rift_menu_reward_target(-1); assert(dm_rift_menu(&m)); assert(m.reward_target == 0);
    dm_rift_menu_reward_target(-1); assert(dm_rift_menu(&m)); assert(m.reward_target == 5);
    dm_rift_menu_start(100); dm_rift_menu_reward_target(1); assert(reward_target() == 5);
    assert(m.reward_successes == 2 && m.reward_failures == 1);
    unsetenv("DM_RIFT_ENCOUNTERS"); unsetenv("DM_RIFT_REPEAT_REWARDS");
    puts("t_rift_integration OK: factory outcomes, pending confirmation, fixed rewards, ambiguity no retry, bounded scale-once");
    return 0;
}

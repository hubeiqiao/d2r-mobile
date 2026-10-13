// #25 安全结算回归：直接包含生产源码，不调用游戏地址；验证没有下一场门仍可返回旧场。
// #25 safe-completion regression: includes production source, calls no game addresses; checks loot return without a next-run portal.
#include "../dm_rift.c"
#include <assert.h>
void (*g_dm_pre_commit_hook[4])(const char *);
uint64_t dm_fs_commit_count(void) { return 0; }
int main(void) {
    dm_rift_menu_t m;
    atomic_store(&K.st, ST_DONE); K.tier = 100; L.nd_lv = 125; L.nd_x = 10; L.nd_y = 20; L.next_door = 0;
    dm_rift_menu_peek(&m);
    if (m.menu != MENU_CHOICE) { puts("FAIL: completed loot return must not depend on next_door"); return 1; }
    atomic_store(&g_run_seq, 4); R.unlock = 100;
    dm_rift_menu_select(); assert(atomic_load(&K.st) == ST_DONE && L.nd_lv == 125);
    dm_rift_menu_start(100); assert(atomic_load(&M.menu) == MENU_CONFIRM && atomic_load(&M.req_start) == 0);
    dm_rift_menu_select(); assert(!atomic_load(&M.confirm_tier) && L.nd_lv == 125);
    dm_rift_menu_start(100); dm_rift_menu_close(); dm_rift_menu_confirm(); assert(!atomic_load(&M.req_start));
    dm_rift_menu_start(100); atomic_fetch_add(&g_run_seq, 1); dm_rift_menu_confirm(); assert(!atomic_load(&M.req_start));
    dm_rift_menu_start(100); dm_rift_menu_confirm(); assert(atomic_exchange(&M.req_start, 0) == 100);
    dm_rift_menu_confirm(); assert(!atomic_load(&M.req_start));
    atomic_store(&K.st, ST_RUN); dm_rift_menu_start(50); assert(!atomic_load(&M.req_start));
    atomic_store(&K.st, ST_IDLE); dm_rift_menu_start(50); assert(atomic_exchange(&M.req_start, 0) == 50);
    atomic_store(&K.st, ST_DONE); L.town_door = 0; dm_rift_menu_peek(&m); assert(m.menu == MENU_CHOICE);
    puts("t_rift_completion OK: loot return, selection/cancel/close, stale confirmation, single start, active-run guard, missing portal"); return 0;
}

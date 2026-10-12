// 秘境计时回归：包含真实秘境读数代码，链接真实客体时钟；模拟后台与键盘冻结，验证 HUD、菜单及恢复后的耗时。
// 不调用原版游戏地址；仅为秘境构造器提供存盘钩子容器与计数桩，不能替代真机锁屏或结算验收。
// Rift timing regression: include the real rift readout code and link the real guest clock; simulate background/keyboard freezes and check HUD, menu and resumed elapsed time.
// No original game addresses are called; only the rift constructor's save-hook array and count are stubbed. This does not replace device lock-screen or completion validation.
#include "../dm_rift.c"
#include <assert.h>

void (*g_dm_pre_commit_hook[4])(const char *);
uint64_t dm_fs_commit_count(void) { return 0; }

static void wait_second(void) {
    struct timespec delay = { 1, 100000000 }, rest;
    while (nanosleep(&delay, &rest)) delay = rest;
}
static void read_left(int *hud, int *menu) {
    dm_rift_hud_t h; dm_rift_menu_t m;
    assert(dm_rift_hud(&h)); menu_fill(&m, MENU_CHOICE);
    *hud = h.secs_left; *menu = m.secs_left;
}
int main(void) {
    unsetenv("DM_DETCLOCK");
    K.t0_ns = guest_ns() - 120ull * 1000000000ull;
    atomic_store(&K.st, ST_RUN);
    guest_clock_freeze_for(1, 1);
    int h0, m0, h1, m1; read_left(&h0, &m0);
    wait_second(); read_left(&h1, &m1);
    if (h1 != h0 || m1 != m0) {
        fprintf(stderr, "FAIL background counted: HUD %d -> %d, menu %d -> %d\n", h0, h1, m0, m1);
        return 1;
    }
    // 键盘与后台重叠：解除后台原因后仍应冻结，全部解除才继续。
    // Overlapping keyboard/background: clearing background alone must not resume the clock; all reasons must clear.
    guest_clock_freeze_for(2, 1); guest_clock_freeze_for(1, 0);
    wait_second(); read_left(&h1, &m1); assert(h1 == h0 && m1 == m0);
    guest_clock_freeze_for(2, 0);
    read_left(&h1, &m1); assert(h1 == h0 && m1 == m0);
    wait_second(); read_left(&h1, &m1); assert(h1 < h0 && m1 == h1);
    // 完成态仍停在结算读数，新的时钟不能让它重新开始倒计时。
    // Completed runs still use the completion readout; the new clock must not restart their countdown.
    g_left_done = 321; atomic_store(&K.st, ST_DONE);
    read_left(&h1, &m1); assert(h1 == 321 && m1 == 321);
    puts("PASS rift HUD/menu freeze, nested reasons, resume and completed state (real guest clock)");
    return 0;
}

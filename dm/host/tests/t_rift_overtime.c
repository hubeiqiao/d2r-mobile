// 包含生产秘境读数与菜单API；固定客体时钟验证超时不是结算，不调用原版地址。
// Include production Rift readouts/menu API; fixed guest clock verifies overtime is not settlement, no native calls.
#include "../dm_rift.c"
#include <assert.h>
void (*g_dm_pre_commit_hook[4])(const char *);
uint64_t dm_fs_commit_count(void) { return 0; }
static uint64_t fixture_now = 2000000000000ull;
uint64_t guest_ns(void) { return fixture_now; }
static void check_running(int elapsed, int guard, int late) {
    atomic_store(&K.st,ST_RUN); K.tier=52; K.inc=0; K.guard=guard;
    atomic_store(&K.timeout,0); atomic_store(&K.guard_dead,0);
    atomic_store(&K.score,125); atomic_store(&K.need,250);
    K.t0_ns=fixture_now-(uint64_t)elapsed*1000000000ull;
    dm_rift_hud_t h={0}; dm_rift_menu_t m={0}; assert(dm_rift_hud(&h)); menu_fill(&m,MENU_CHOICE);
    assert(h.overtime==late && m.overtime==late && !h.done && !m.done);
    assert(h.secs_left==(elapsed>=900?0:900-elapsed) && h.secs_left==m.secs_left);
    assert(h.guardian==!!guard && h.progress==.5f && m.progress==.5f);
    assert(atomic_load(&K.st)==ST_RUN && atomic_load(&K.score)==125 && K.inc==0 && !atomic_load(&K.timeout));
}
int main(void) {
    check_running(899,0,0); check_running(900,0,1); check_running(930,77,1); check_running(1900,77,1);
    atomic_store(&K.st,ST_DONE); g_left_done=-30; K.inc=0;
    dm_rift_hud_t h={0}; dm_rift_menu_t m={0}; assert(dm_rift_hud(&h)); menu_fill(&m,MENU_CHOICE);
    assert(h.done && m.done && h.overtime && m.overtime && h.secs_left==0 && m.secs_left==0);
    K.inc=2; g_left_done=300; assert(dm_rift_hud(&h)); menu_fill(&m,MENU_CHOICE);
    assert(!h.overtime && !m.overtime && h.secs_left==300);
    atomic_store(&K.st,ST_RUN); K.t0_ns=0; assert(dm_rift_hud(&h)); menu_fill(&m,MENU_CHOICE);
    assert(!h.overtime && !m.overtime && h.secs_left==900);
    atomic_store(&K.st,ST_IDLE); assert(!dm_rift_hud(&h)); menu_fill(&m,MENU_SELECT); assert(!m.overtime);
    atomic_store(&M.req_cont,0); atomic_store(&M.req_abandon,0); atomic_store(&M.req_start,0);
    dm_rift_menu_continue(); assert(atomic_load(&M.req_cont)==1 && !atomic_load(&M.req_start) && !atomic_load(&M.req_abandon));
    dm_rift_menu_abandon(); assert(atomic_load(&M.req_abandon)==1 && !atomic_load(&M.req_start));
    puts("t_rift_overtime PASS: boundary, guardian, uncompleted/settled, stale state, read-only and existing menu actions");
    return 0;
}

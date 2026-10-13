// 真实生产HUD读数，固定客体时钟；不调用游戏地址。
// Real production HUD readout with fixed guest clock; no native game calls.
#include "../../../host/dm_rift.c"
void (*g_dm_pre_commit_hook[4])(const char *);
uint64_t dm_fs_commit_count(void) { return 0; }
uint64_t guest_ns(void) { return 2000000000000ull; }
void fixture_setup(int state, int elapsed, int guardian) {
    atomic_store(&K.st,state); K.tier=52; K.inc=state==ST_DONE && elapsed<900?2:0;
    K.t0_ns=guest_ns()-(uint64_t)elapsed*1000000000ull; K.guard=guardian?77:0;
    atomic_store(&K.guard_dead,0); g_left_done=900-elapsed;
    atomic_store(&K.need,250); atomic_store(&K.score,guardian||state==ST_DONE?250:125);
}

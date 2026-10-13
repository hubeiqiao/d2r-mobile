// 包含生产状态与读数；只替代时钟/文件系统，不调用原版地址。
// Include production state/readout; stub only clock/filesystem, never native game addresses.
#include "../../../host/dm_rift.c"
void (*g_dm_pre_commit_hook[4])(const char *);
uint64_t dm_fs_commit_count(void) { return 0; }
uint64_t guest_ns(void) { return 1000000000ull; }
void fixture_setup(int tier, int inc) {
    atomic_store(&K.st, ST_DONE); K.tier = tier; K.inc = inc; K.guard = 0;
    g_left_done = 629; atomic_store(&K.need, 250); atomic_store(&K.score, 250);
}

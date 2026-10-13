// 层数封顶回归：包含生产 dm_rift.c，验证 HUD 与实际下一层的读数一致，不调原版地址。
// Tier-cap regression: includes production dm_rift.c; verifies HUD agrees with the actual next tier, without native calls.
#ifndef DM_RIFT_SOURCE
#define DM_RIFT_SOURCE "../dm_rift.c"
#endif
#include DM_RIFT_SOURCE
void (*g_dm_pre_commit_hook[4])(const char *);
uint64_t dm_fs_commit_count(void) { return 0; }
int main(void) {
    int fails = 0, checks = 0;
    const int cases[][3] = {{97,3,100},{98,3,100},{99,3,100},{100,3,100},{100,2,100},{100,1,100},{100,0,100},{74,3,77},{74,2,76},{74,1,75},{74,0,74}};
    atomic_store(&K.st, ST_DONE); g_left_done = 629;
    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        K.tier = cases[i][0]; K.inc = cases[i][1]; dm_rift_hud_t h = {0};
        int ok = dm_rift_hud(&h); checks++;
        if (!ok || h.next_tier != cases[i][2] || next_tier() != cases[i][2] || h.inc != cases[i][1] || !h.done) {
            fprintf(stderr, "FAIL tier=%d inc=%d HUD=%d actual=%d expected=%d\n", K.tier, K.inc, h.next_tier, next_tier(), cases[i][2]); fails++;
        }
    }
    printf("t_rift_tier_cap: %d checks, %d failures\n", checks, fails); return fails != 0;
}

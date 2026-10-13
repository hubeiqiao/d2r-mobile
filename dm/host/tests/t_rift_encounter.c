// #26 宿主策略与真实区域写入回归：合成内存，不含游戏资源，不调用原版函数。
// #26 host policy and actual region-write regression: synthetic memory, no game content or native calls.
#ifndef DM_RIFT_SOURCE
#define DM_RIFT_SOURCE "../dm_rift.c"
#endif
#include DM_RIFT_SOURCE
#include "../dm_rift_encounter.h"
#include <assert.h>
#include <limits.h>
void (*g_dm_pre_commit_hook[4])(const char *);
uint64_t dm_fs_commit_count(void) { return 0; }
#ifdef RF_LEGACY
static rf_encounter g_encounter;
static int g_enc_density[3], g_enc_enabled;
#endif
static int failures;
static int route_zero(int n) { (void)n; return 0; }
static uint32_t route_seed = 1;
static int route_draw(int n) { route_seed = route_seed * 1664525u + 1013904223u; return (int)((route_seed >> 8) % (unsigned)n); }
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); failures++; } } while (0)
int main(void) {
#ifndef RF_LEGACY
    setenv("DM_RIFT_ENCOUNTERS", "0", 1); unsetenv("DM_RIFT_NEED"); encounter_prepare(); CHECK(atomic_load(&K.need) == 250 && !g_enc_enabled);
    unsetenv("DM_RIFT_ENCOUNTERS"); encounter_prepare(); CHECK(atomic_load(&K.need) == 250 && !g_enc_enabled);
    setenv("DM_RIFT_ENCOUNTERS", "1", 1); encounter_prepare(); CHECK(atomic_load(&K.need) == g_encounter.need && g_enc_enabled);
    setenv("DM_RIFT_NEED", "700", 1); encounter_prepare(); CHECK(atomic_load(&K.need) == 700);
    setenv("DM_RIFT_NEED", "-1", 1); encounter_prepare(); CHECK(atomic_load(&K.need) == g_encounter.need);
    unsetenv("DM_RIFT_NEED");
    for (int profile = 0; profile < 4; profile++) { char setting[2] = { (char)('0' + profile), 0 }; setenv("DM_RIFT_ENCOUNTER_PROFILE", setting, 1); encounter_prepare(); CHECK(g_encounter.profile == profile); }
    setenv("DM_RIFT_ENCOUNTERS", "0", 1); encounter_prepare(); CHECK(!g_enc_enabled && atomic_load(&K.need) == 250);
    unsetenv("DM_RIFT_ENCOUNTER_PROFILE"); unsetenv("DM_RIFT_ENCOUNTERS");
#endif
    int weights[4] = {0};
    for (unsigned r = 0; r < 100; r++) { rf_encounter e = rf_encounter_pick(r); weights[e.profile]++;
        for (unsigned v = 0; v < 5; v++) { int d = rf_density(e, v); CHECK(d >= 2340 && d <= 4620); CHECK(e.need >= 300 && e.need <= 425); }
    }
    CHECK(weights[0] == 50 && weights[1] == 25 && weights[2] == 20 && weights[3] == 5);
    const int open[] = {1,2,3}, maze[] = {11,12,13}; int out[3], first_open = 0, first_maze = 0;
    for (int p = 0; p < 4; p++) for (int i = 0; i < 1000; i++) {
        int n = rf_route_pick(p, open, 3, maze, 3, route_draw, out), oc = 0;
        CHECK(n >= 2 && n <= 3); if (out[0] < 10) first_open++; else first_maze++;
        for (int j = 0; j < n; j++) { CHECK((out[j] >= 1 && out[j] <= 3) || (out[j] >= 11 && out[j] <= 13)); oc += out[j] < 10;
            for (int k = 0; k < j; k++) CHECK(out[j] != out[k]); }
        CHECK(oc == ((p == RF_SWARM || p == RF_DREAM) ? 2 : 1)); CHECK(n == 3 || p == RF_BALANCED);
    }
    CHECK(first_open && first_maze);
    CHECK(rf_route_pick(RF_BALANCED, open, 3, maze, 1, route_zero, out) == 2);
    CHECK((out[0] < 10) + (out[1] < 10) == 1);
    CHECK(rf_route_pick(RF_SWARM, open, 1, maze, 2, route_draw, out) == 3);
    CHECK((out[0] < 10) + (out[1] < 10) + (out[2] < 10) == 1);
    CHECK(rf_route_pick(RF_HUNT, open, 0, maze, 3, route_draw, out) == 0);
    CHECK(rf_route_pick(RF_HUNT, open, 3, maze, 1, route_draw, out) == 3);
    CHECK(rf_route_pick(RF_HUNT, open, 1, maze, 1, route_draw, out) == 2);
    CHECK(rf_route_pick(RF_HUNT, open, 3, maze, 0, route_draw, out) == 0);
    CHECK(rf_scaled(INT_MAX, INT_MAX, 9000) == 9000); CHECK(rf_scaled(-1, 3000, 9000) == 0);
    rf_observed seen = {0};
    for (uint32_t i = 1; i <= 4096; i++) CHECK(rf_observed_new(&seen, i));
    for (uint32_t i = 1; i <= 4096; i++) CHECK(!rf_observed_new(&seen, i));
    CHECK(!rf_observed_new(&seen, 4097) && seen.overflow); CHECK(!rf_observed_new(&seen, 0));
    for (uint32_t i = 1; i <= 4096; i++) CHECK(!rf_observed_new(&seen, i));
    CHECK(!rf_observed_new(&seen, UINT32_MAX) && seen.overflow);
    memset(&seen, 0, sizeof seen); CHECK(rf_observed_new(&seen, 4097));

    B = calloc(1, 0x2d64a40); uint8_t *game = calloc(1, 0x2000), *table = calloc(1, 0x2000), *region = calloc(1, 0x300);
    assert(B && game && table && region); uint64_t G = (uint64_t)(uintptr_t)game;
    *(uint64_t *)(B + 0x2d64a18) = (uint64_t)(uintptr_t)table;
    *(uint64_t *)(game + 0x1d0 + 8 * 7) = (uint64_t)(uintptr_t)region;
    *(int *)(region + 0x2c4) = 2200; region[0x2c8] = 5; region[0x2c9] = 7;
    K.nmaps = 1; K.maps[0] = 7; K.tier = 90;
    g_enc_enabled = 1; g_encounter = rf_encounter_pick(80); g_enc_density[0] = rf_density(g_encounter, 2);
    rift_setup(G); CHECK(*(int *)(region + 0x2c4) == 5720); CHECK(region[0x2c8] == 30 && region[0x2c9] == 42);
    rift_setup(G); CHECK(*(int *)(region + 0x2c4) == 5720); CHECK(region[0x2c8] == 30);
    region_set(G, 1); CHECK(*(int *)(region + 0x2c4) == 0 && region[0x2c8] == 0);
    region_set(G, 0); CHECK(*(int *)(region + 0x2c4) == 2200 && region[0x2c8] == 5 && region[0x2c9] == 7);
    g_encounter = rf_encounter_pick(95); g_enc_density[0] = rf_density(g_encounter, 4);
    rift_setup(G); CHECK(*(int *)(region + 0x2c4) == 9000); CHECK(region[0x2c8] == 30 && region[0x2c9] == 42);
    g_enc_enabled = 0; rift_setup(G); CHECK(*(int *)(region + 0x2c4) == 6600); CHECK(region[0x2c8] == 20 && region[0x2c9] == 28);
    free(region); free(table); free(game); free(B);
    printf("t_rift_encounter: %d failures; weights, profiles, 4000 routes, overflow/dedup, real region writes, idempotence, restore, classic fallback\n", failures);
    return failures != 0;
}

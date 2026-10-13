// #26 遭遇方案：作者定义的权重、密度/精英/进度策略，使用 dm_rift.c 既有区域字段；不包含游戏派生表。
// #26 encounter plans: authored weights and density/elite/progress policy for dm_rift.c's established region fields; contains no game-derived tables.
#pragma once
#include <stdint.h>
enum { RF_BALANCED, RF_SWARM, RF_HUNT, RF_DREAM };
typedef struct { int profile, density, elite, need; } rf_encounter;
static inline rf_encounter rf_encounter_pick(unsigned roll) {
    // 提案权重 50/25/20/5，不宣称是 D3 原值；need 同时增长，避免加密度后进度反而更快。
    // Proposed weights 50/25/20/5, not asserted to be D3 values; need grows too so denser encounters do not fill progress faster.
    rf_encounter e = { RF_BALANCED, 3000, 1000, 300 };
    if (roll >= 100) return e;
    if (roll >= 95) { e.profile = RF_DREAM; e.density = 4200; e.elite = 1500; e.need = 425; }
    else if (roll >= 75) { e.profile = RF_HUNT; e.density = 2600; e.elite = 1500; e.need = 350; }
    else if (roll >= 50) { e.profile = RF_SWARM; e.density = 3800; e.elite = 1000; e.need = 350; }
    return e;
}
static inline int rf_density(rf_encounter e, unsigned variation) {
    // 每张图一次抽 ±10%，不随每次进入/每帧重抽；输入异常取中档。
    // Roll +/-10% once per map, not on every entry/frame; invalid input takes the middle band.
    int pct = variation <= 4 ? 90 + (int)variation * 5 : 100;
    return e.density * pct / 100;
}
static inline int rf_scaled(int base, int mult, int cap) {
    if (base <= 0 || mult <= 0 || cap <= 0) return 0;
    int64_t n = (int64_t)base * mult / 1000;
    return n > cap ? cap : (int)n;
}
// 客户端观察去重，不覆盖旧 id；满了停止记新单位，不能把观察数当全图生成数。
// Client observation deduplication never overwrites old IDs; once full, stop counting new units, and never equate observations with full-map generation.
typedef struct { uint32_t ids[4096]; int overflow; } rf_observed;
static inline int rf_observed_new(rf_observed *s, uint32_t id) {
    if (!id || s->overflow) return 0;
    unsigned h = (id * 2654435761u) & 4095;
    for (unsigned i = 0; i < 4096; i++) { unsigned k = (h + i) & 4095;
        if (s->ids[k] == id) return 0;
        if (!s->ids[k]) { s->ids[k] = id; return 1; }
    }
    s->overflow = 1; return 0;
}

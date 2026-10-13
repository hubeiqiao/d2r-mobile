// 实验性精英属性变体，非 D3 技能词缀；纯策略，不访问原生地址。
// Experimental elite stat variants, not D3 skill affixes; pure policy with no native address access.
#pragma once
#include <stdint.h>
#include <limits.h>

enum { RF_VARIANT_NONE, RF_VARIANT_HARDENED, RF_VARIANT_ARMORED, RF_VARIANT_AGGRESSIVE };
enum { RF_VARIANT_HP_CAP = 0x7fffff00 };
typedef struct {
    int tier, profile;
    unsigned native_flags;
    int guardian;
    uint32_t run_salt, unit_id;
    int hp_max, hp_current, damage_pct, defense;
} rf_variant_input;
typedef struct { int tag, hp_max, hp_current, damage_pct, defense; } rf_variant_plan;

static inline int rf_variant_eligible(rf_variant_input v) {
    // 超独特/守卫/随从排除；组合分类有歧义时也不改写。
    // Exclude superuniques, guardians and minions; ambiguous category combinations are left alone too.
    unsigned elite = v.native_flags & 12u;
    return v.tier >= 41 && v.tier <= 100 && v.profile >= 0 && v.profile <= 3 &&
        v.guardian == 0 && v.unit_id != 0 && v.native_flags <= 255u && !(v.native_flags & 18u) &&
        (elite == 4u || elite == 8u) && v.hp_max > 0 && v.hp_max <= RF_VARIANT_HP_CAP &&
        v.hp_current > 0 && v.hp_current <= v.hp_max && v.damage_pct >= 0 && v.defense >= 0;
}

static inline uint32_t rf_variant_hash(uint32_t salt, uint32_t id) {
    // 只依赖场次盐和单位 ID；不随每帧或再次接近重新抽取。
    // Depend only on run salt and unit ID, never on frame timing or repeated approaches.
    uint32_t x = salt ^ (id * UINT32_C(0x9e3779b9));
    x ^= x >> 16; x *= UINT32_C(0x7feb352d);
    x ^= x >> 15; x *= UINT32_C(0x846ca68b); x ^= x >> 16;
    return x;
}
static inline int rf_variant_scale(int base, int pct, int cap) {
    int64_t value = (int64_t)base * pct / 100;
    return value > cap ? cap : value < 1 ? 1 : (int)value;
}
static inline rf_variant_plan rf_variant_calculate(rf_variant_input v) {
    rf_variant_plan p = { RF_VARIANT_NONE, v.hp_max, v.hp_current, v.damage_pct, v.defense };
    if (!rf_variant_eligible(v)) return p;
    int tag = RF_VARIANT_HARDENED + (int)(rf_variant_hash(v.run_salt, v.unit_id) % 3u);
    // 高伤害原生精英不能再叠加攻击型变体；其他变体也不改变伤害。
    // Already high-damage native elites cannot gain an aggressive variant; the other variants preserve damage too.
    if (tag == RF_VARIANT_AGGRESSIVE && v.damage_pct > 100) return p;
    p.tag = tag;
    if (tag == RF_VARIANT_HARDENED) {
        p.hp_max = rf_variant_scale(v.hp_max, 115, RF_VARIANT_HP_CAP);
        p.defense = v.defense ? rf_variant_scale(v.defense, 120, INT_MAX) : 0;
    } else if (tag == RF_VARIANT_ARMORED) {
        p.defense = v.defense ? rf_variant_scale(v.defense, 135, INT_MAX) : 0;
    } else {
        p.hp_max = rf_variant_scale(v.hp_max, 90, RF_VARIANT_HP_CAP);
        p.damage_pct = v.damage_pct + 10;
    }
    // 保持当前/最大生命比例（整数向下取整），受伤单位不会被补满。
    // Preserve current/max life ratio (integer floor); injured units are never filled to full life.
    p.hp_current = (int)((int64_t)v.hp_current * p.hp_max / v.hp_max);
    // 比例舍入到零时放弃变体，避免属性调整本身杀死单位。
    // If ratio rounding would produce zero life, skip the variant rather than killing the unit by adjustment.
    if (!p.hp_current) { rf_variant_plan unchanged = { RF_VARIANT_NONE, v.hp_max, v.hp_current, v.damage_pct, v.defense }; return unchanged; }
    return p;
}

// 纯策略宿主测试，不宣称原生游戏体验已验证。
// Pure-policy host tests; these do not establish native gameplay validation.
#include <stdio.h>
#include <string.h>
#include "../dm_rift_variant.h"
static int failures;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)
static rf_variant_input sample(void) {
    rf_variant_input v = { 41, 0, 4u, 0, 1u, 7u, 256000, 123456, 25, 1000 };
    return v;
}
static rf_variant_input tagged(int tag) {
    rf_variant_input v = sample();
    while (rf_variant_calculate(v).tag != tag) v.run_salt++;
    return v;
}
static void rejected(rf_variant_input v) {
    rf_variant_plan p = rf_variant_calculate(v);
    CHECK(!rf_variant_eligible(v)); CHECK(p.tag == RF_VARIANT_NONE);
    CHECK(p.hp_max == v.hp_max && p.hp_current == v.hp_current && p.damage_pct == v.damage_pct && p.defense == v.defense);
}
int main(void) {
    rf_variant_input v = sample();
    CHECK(rf_variant_eligible(v));
    for (int tier = 41; tier <= 100; tier++) for (int profile = 0; profile < 4; profile++) {
        v.tier = tier; v.profile = profile; CHECK(rf_variant_eligible(v));
        rf_variant_plan a = rf_variant_calculate(v), b = rf_variant_calculate(v);
        CHECK(memcmp(&a, &b, sizeof(a)) == 0);
        CHECK(a.hp_current == (int)((int64_t)v.hp_current * a.hp_max / v.hp_max));
        CHECK(a.hp_current < a.hp_max);
    }
    v = sample(); v.native_flags = 8u; CHECK(rf_variant_eligible(v));
    unsigned excluded[] = { 0u, 2u, 16u, 6u, 10u, 20u, 24u, 12u, 30u };
    for (unsigned i = 0; i < sizeof(excluded)/sizeof(excluded[0]); i++) { v = sample(); v.native_flags = excluded[i]; rejected(v); }
    v = sample(); v.guardian = 1; rejected(v);
    v = sample(); v.guardian = -1; rejected(v);
    int tiers[] = { INT_MIN, -1, 0, 1, 40, 101, INT_MAX };
    for (unsigned i = 0; i < sizeof(tiers)/sizeof(tiers[0]); i++) { v = sample(); v.tier = tiers[i]; rejected(v); }
    v = sample(); v.profile = -1; rejected(v); v.profile = 4; rejected(v);
    v = sample(); v.unit_id = 0; rejected(v);
    v = sample(); v.native_flags = 260u; rejected(v);
    v = sample(); v.hp_max = 0; rejected(v); v.hp_max = -1; rejected(v); v.hp_max = INT_MAX; rejected(v);
    v = sample(); v.hp_current = 0; rejected(v); v.hp_current = -1; rejected(v); v.hp_current = v.hp_max + 1; rejected(v);
    v = sample(); v.damage_pct = -1; rejected(v);
    v = sample(); v.defense = -1; rejected(v);
    for (int tag = RF_VARIANT_HARDENED; tag <= RF_VARIANT_AGGRESSIVE; tag++) {
        v = tagged(tag); rf_variant_plan p = rf_variant_calculate(v);
        CHECK(p.tag == tag);
        CHECK(p.hp_max == v.hp_max * (tag == RF_VARIANT_HARDENED ? 115 : tag == RF_VARIANT_AGGRESSIVE ? 90 : 100) / 100);
        CHECK(p.defense == v.defense * (tag == RF_VARIANT_HARDENED ? 120 : tag == RF_VARIANT_ARMORED ? 135 : 100) / 100);
        CHECK(p.damage_pct == v.damage_pct + (tag == RF_VARIANT_AGGRESSIVE ? 10 : 0));
        v.hp_current = v.hp_max; p = rf_variant_calculate(v); CHECK(p.hp_current == p.hp_max);
        v.hp_max = RF_VARIANT_HP_CAP; v.hp_current = RF_VARIANT_HP_CAP - 1; v.defense = INT_MAX;
        p = rf_variant_calculate(v); CHECK(p.hp_max > 0 && p.hp_max <= RF_VARIANT_HP_CAP);
        CHECK(p.hp_current < p.hp_max); CHECK(p.defense == INT_MAX);
        v.hp_max = 1; v.hp_current = 1; v.defense = 0; p = rf_variant_calculate(v);
        CHECK(p.hp_max == 1 && p.hp_current == 1 && p.defense == 0);
        v = tagged(tag); v.damage_pct = INT_MAX; p = rf_variant_calculate(v);
        CHECK(p.damage_pct == INT_MAX); CHECK(p.tag == (tag == RF_VARIANT_AGGRESSIVE ? RF_VARIANT_NONE : tag));
    }
    for (int tag = RF_VARIANT_HARDENED; tag <= RF_VARIANT_AGGRESSIVE; tag++) {
        v = tagged(tag); v.damage_pct = 0; v.defense = 0;
        rf_variant_plan p = rf_variant_calculate(v); CHECK(p.tag == tag && p.defense == 0);
        CHECK(p.damage_pct == (tag == RF_VARIANT_AGGRESSIVE ? 10 : 0));
    }
    v = tagged(RF_VARIANT_AGGRESSIVE); v.damage_pct = 100; CHECK(rf_variant_calculate(v).damage_pct == 110);
    v.damage_pct = 101; CHECK(rf_variant_calculate(v).tag == RF_VARIANT_NONE);
    v = tagged(RF_VARIANT_AGGRESSIVE); v.hp_current = 1;
    rf_variant_plan tiny = rf_variant_calculate(v); CHECK(tiny.tag == RF_VARIANT_NONE && tiny.hp_current == 1 && tiny.hp_max == v.hp_max);
    v = sample(); v.run_salt = UINT32_MAX; v.unit_id = UINT32_MAX; CHECK(rf_variant_eligible(v));
    printf("rift variant policy: %d failure(s)\n", failures);
    return failures != 0;
}

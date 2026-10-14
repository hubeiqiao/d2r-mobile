// 守护者延迟波回归：包含实际生产代码，以日志的拍数检查回城、返回及结算边界；不调用原版入口。
// Guardian delayed-wave regression: includes production code, checking town, return and settlement boundaries at logged ticks; calls no native entry.
#include "../dm_rift.c"
void (*g_dm_pre_commit_hook[4])(const char *);
uint64_t dm_fs_commit_count(void) { return 0; }
static int checks, fails;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)
int main(void) {
    K.nmaps = 2; K.maps[0] = 111; K.maps[1] = 112;
    K.wave = 1; K.nwave = 3; K.wave_t = 21843; atomic_store(&K.st, ST_RUN);
    CHECK(!guard_wave_due(21917, 111));
    CHECK(!guard_wave_due(21918, LV_TOWN));
    CHECK(!guard_wave_due(23000, LV_TOWN));
    CHECK(!guard_wave_due(23000, 1));
    CHECK(!guard_wave_due(23000, 0));
    CHECK(!guard_wave_due(23000, 110));
    CHECK(K.wave_t == 21843 && K.wave == 1);
    CHECK(guard_wave_due(23000, 111));
    CHECK(guard_wave_due(23000, 112));
    CHECK(K.wave_t == 21843 && K.wave == 1);
    atomic_store(&K.st, ST_DONE); CHECK(!guard_wave_due(23000, 111));
    atomic_store(&K.st, ST_IDLE); CHECK(!guard_wave_due(23000, 111));
    atomic_store(&K.st, ST_RUN); K.wave_t = 0; CHECK(!guard_wave_due(23001, 111));
    K.wave = 2; K.wave_t = 24000;
    CHECK(!guard_wave_due(24075, LV_TOWN)); CHECK(guard_wave_due(24100, 111));
    printf("t_rift_wave: %d checks, %d failures\n", checks, fails); return fails != 0;
}

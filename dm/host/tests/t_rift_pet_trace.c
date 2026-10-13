// #23 随从诊断单测：包含真实 dm_rift.c，验证日志字段、封顶标记与限流 / 重置，不调用原版地址。
// 本测试不能证明生命反馈已修复，也不模拟原版光环、技能、序列化或存档提交。
// #23 minion diagnostic test: includes real dm_rift.c, verifies fields, cap flag and rate limiting / resets without calling native addresses.
// This test does not establish that life feedback is fixed and does not simulate native auras, skills, serialization or save commits.
#include "../dm_rift.c"
#include <unistd.h>
#include <assert.h>

void (*g_dm_pre_commit_hook[4])(const char *);
uint64_t dm_fs_commit_count(void) { return 7; }

int main(void) {
    FILE *capture = tmpfile(); assert(capture);
    fflush(stderr); int keep = dup(STDERR_FILENO); assert(keep >= 0);
    assert(dup2(fileno(capture), STDERR_FILENO) >= 0);
    K.tier = 77; atomic_store(&K.st, ST_RUN); atomic_store(&g_run_seq, 3);
    for (int i = 0; i < 64; i++) assert(pet_trace_allow(1));
    for (int i = 0; i < 1000; i++) assert(!pet_trace_allow(1));
    atomic_store(&K.st, ST_DONE); assert(pet_trace_allow(1));
    for (int i = 1; i < 64; i++) assert(pet_trace_allow(1));
    assert(!pet_trace_allow(1));
    atomic_store(&g_run_seq, 4); assert(pet_trace_allow(1));
    for (int i = 1; i < 64; i++) assert(pet_trace_allow(1));
    assert(!pet_trace_allow(1));
    assert(pet_trace_allow(2));
    // 输入故意不同，确保保留写入前基数、观察值、封顶前目标和读回值，而不是用最后一次写值冒充。
    // Deliberately distinct inputs ensure old base, observed values, pre-cap target and readback survive instead of being replaced by the last write.
    pet_trace_line(123, 264, 20, 744, 1, "rebase", 50, 921600, 3585792, 3891,
                   6347008, 1000000, 6347008, 3891, 24696207628LL, 0x7fffff00,
                   1000001, 0x7ffffe00, 1000002, 1);
    pet_trace_line(124, 265, 20, 744, 1, "restore", 50, 921600, 3585792, 3891,
                   3585792, 1000000, 921600, 1000, 921600, 921600,
                   257024, 921600, 257024, 0);
    fflush(stderr); assert(dup2(keep, STDERR_FILENO) >= 0); close(keep);
    rewind(capture); char output[8192]; size_t n = fread(output, 1, sizeof output - 1, capture); output[n] = 0; fclose(capture);
    int suppressed = 0; for (char *p = output; (p = strstr(p, "trace suppressed")); p++) suppressed++;
    assert(suppressed == 3);
    assert(strstr(output, "run=4 state=2 tier=77 tick=123 id=264 type=20 class=744 mode=1 reason=rebase level=50"));
    assert(strstr(output, "prev_base=921600 prev_last=3585792 prev_mult=3891 observed_max=6347008 observed_hp=1000000 base=6347008 mult=3891"));
    assert(strstr(output, "requested=24696207628 write_max=2147483392 write_hp=1000001 read_max=2147483136 read_hp=1000002 capped=1 auto_buff=1 commits=7"));
    assert(strstr(output, "reason=restore"));
    assert(strstr(output, "capped=0 auto_buff=0 commits=7"));
    puts("t_rift_pet_trace OK: fields, fixed-point, cap, restore, 64-record limit, single suppression, state/floor/game resets");
    return 0;
}

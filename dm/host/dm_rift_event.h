// 可选追逐事件的有界状态机；依据宿主事件契约，不读取原版程序地址或内容。
// Bounded optional chase-event state machine; based on a host contract, with no native addresses or content.
#ifndef DM_RIFT_EVENT_H
#define DM_RIFT_EVENT_H
#include <stdint.h>
#include <string.h>

enum rf_event_state {
    RF_EVENT_DISABLED, RF_EVENT_ELIGIBLE, RF_EVENT_SPAWN_REQUESTED,
    RF_EVENT_ACTIVE, RF_EVENT_KILLED, RF_EVENT_ESCAPED,
    RF_EVENT_CANCELLED, RF_EVENT_RESOLVED
};
enum { RF_EVENT_MAX_REWARDS = 3 };
#define RF_EVENT_MAX_DURATION_NS INT64_C(120000000000)
typedef struct {
    enum rf_event_state state;
    uint32_t run_seq, unit_id;
    int64_t start_ns, deadline_ns;
    unsigned requested, reward_attempts, reward_pending;
    unsigned reward_claimed, reward_failed, spawn_failed;
    unsigned cleanup_pending, cleanup_consumed;
} rf_event;

// 所有调用在同一服务端线程；只有适配器验证通过且功能开启才能启用。
// All calls belong to one server thread; enable only after adapter verification AND feature opt-in.
static inline int rf_event_begin(rf_event *e, uint32_t seq, int enabled) {
    if (!seq || e->run_seq == seq) return 0;
    if (e->state == RF_EVENT_ACTIVE || e->state == RF_EVENT_SPAWN_REQUESTED ||
        e->state == RF_EVENT_KILLED) {
        e->state = RF_EVENT_CANCELLED;
        e->reward_pending = 0;
        if (e->requested && !e->cleanup_consumed) e->cleanup_pending = 1;
    }
    if (e->cleanup_pending) return 0;
    memset(e, 0, sizeof(*e));
    e->run_seq = seq;
    e->state = enabled ? RF_EVENT_ELIGIBLE : RF_EVENT_DISABLED;
    return 1;
}
static inline int rf_event_matches(const rf_event *e, uint32_t seq) {
    return e->run_seq == seq;
}
static inline void rf_event_cleanup_schedule(rf_event *e) {
    if (!e->cleanup_consumed && e->requested) e->cleanup_pending = 1;
}
// unit=0 也要撤销尚未完成的生成请求；适配器对过期生成回调负责清理。
// unit=0 still cancels an outstanding spawn request; adapters must clean stale spawn callbacks themselves.
static inline int rf_event_take_cleanup(rf_event *e, uint32_t *unit) {
    if (!e->cleanup_pending) return 0;
    if (unit) *unit = e->unit_id;
    e->cleanup_pending = 0;
    e->cleanup_consumed = 1;
    return 1;
}
static inline void rf_event_cancel(rf_event *e, uint32_t seq) {
    if (!rf_event_matches(e, seq) || e->state == RF_EVENT_DISABLED ||
        e->state == RF_EVENT_CANCELLED || e->state == RF_EVENT_RESOLVED) return;
    e->state = RF_EVENT_CANCELLED;
    e->reward_pending = 0;
    rf_event_cleanup_schedule(e);
}
// 在守护者出现前且远离受保护入口才可请求，每场至多一次；时间来自 guest_ns。
// Request only before the guardian and outside protected entrances, once per run; time comes from guest_ns.
static inline int rf_event_request(rf_event *e, uint32_t seq, int guardian,
                                    int protected_entrance, int64_t now_ns,
                                    int64_t duration_ns) {
    if (!rf_event_matches(e, seq) || e->state != RF_EVENT_ELIGIBLE ||
        e->requested || guardian || protected_entrance || now_ns < 0 ||
        duration_ns <= 0 || duration_ns > RF_EVENT_MAX_DURATION_NS ||
        now_ns > INT64_MAX - duration_ns) return 0;
    e->requested = 1;
    e->start_ns = now_ns;
    e->deadline_ns = now_ns + duration_ns;
    e->state = RF_EVENT_SPAWN_REQUESTED;
    return 1;
}
// 返回 0 的生成回调不取得单位所有权，调用者必须立即清理该单位。
// A rejected spawn callback transfers no unit ownership; the caller must immediately clean its unit.
static inline int rf_event_spawned(rf_event *e, uint32_t seq, uint32_t unit,
                                    int64_t now_ns) {
    if (!rf_event_matches(e, seq) || e->state != RF_EVENT_SPAWN_REQUESTED ||
        !unit || now_ns < e->start_ns || now_ns >= e->deadline_ns) return 0;
    e->unit_id = unit;
    e->state = RF_EVENT_ACTIVE;
    return 1;
}
static inline void rf_event_spawn_failure(rf_event *e, uint32_t seq) {
    if (!rf_event_matches(e, seq) || e->state != RF_EVENT_SPAWN_REQUESTED) return;
    e->spawn_failed = 1;
    e->state = RF_EVENT_ESCAPED;
    rf_event_cleanup_schedule(e);
}
// 不存在单位绝不代表死亡；守护者、完成或放弃应先调用 cancel。
// A missing unit never means death; guardian, completion, and abandonment must first call cancel.
static inline void rf_event_tick(rf_event *e, uint32_t seq, int64_t now_ns) {
    if (!rf_event_matches(e, seq)) return;
    if ((e->state == RF_EVENT_ACTIVE || e->state == RF_EVENT_SPAWN_REQUESTED) &&
        now_ns >= e->deadline_ns) {
        e->state = RF_EVENT_ESCAPED;
        rf_event_cleanup_schedule(e);
    }
}
static inline int rf_event_server_dead(rf_event *e, uint32_t seq, uint32_t unit,
                                       int confirmed_dead, int64_t now_ns) {
    if (!rf_event_matches(e, seq) || e->state != RF_EVENT_ACTIVE ||
        unit != e->unit_id || !confirmed_dead || now_ns < e->start_ns) return 0;
    rf_event_tick(e, seq, now_ns);
    if (e->state != RF_EVENT_ACTIVE) return 0;
    e->state = RF_EVENT_KILLED;
    return 1;
}
// 每次领取返回递增令牌；不能并行创建奖励，安全重试必须保证未产生任何物品。
// Each claim returns an increasing token; reward creation cannot overlap, and safe retry must mean no items were created.
static inline unsigned rf_event_reward_begin(rf_event *e, uint32_t seq) {
    if (!rf_event_matches(e, seq) || e->state != RF_EVENT_KILLED ||
        e->reward_pending || e->reward_claimed || e->reward_attempts >= RF_EVENT_MAX_REWARDS) return 0;
    e->reward_pending = 1;
    return ++e->reward_attempts;
}
// 结果：1=确认成功，0=确认零物品可重试，其他值=结果不确定，终止且不领取。
// Result: 1=confirmed success, 0=confirmed zero items and retryable, other values=uncertain, terminal without claiming.
static inline int rf_event_reward_result(rf_event *e, uint32_t seq,
                                         unsigned token, int success) {
    if (!rf_event_matches(e, seq) || e->state != RF_EVENT_KILLED ||
        !e->reward_pending || !token || token != e->reward_attempts) return 0;
    e->reward_pending = 0;
    if (success == 1) {
        e->reward_claimed = 1;
        e->state = RF_EVENT_RESOLVED;
        rf_event_cleanup_schedule(e);
    } else if (success != 0 || e->reward_attempts == RF_EVENT_MAX_REWARDS) {
        e->reward_failed = 1;
        e->state = RF_EVENT_RESOLVED;
        rf_event_cleanup_schedule(e);
    }
    return 1;
}
#endif

/* 有界进程内材料队列；无原生调用，不更改存档格式。
 * Bounded per-process material queue; no native calls or persistent schema. */
#ifndef DM_RIFT_REWARD_H
#define DM_RIFT_REWARD_H

#include <stdint.h>

/* 随机源返回0..4，对应xa1..xa5。 | RNG returns 0..4, mapping to xa1..xa5. */
typedef unsigned (*rf_reward_rng)(void *ctx);
/* 0：未创建可重试；1：确认发放；-1：创建状态不明，禁止重试。
 * 0: safe retry, no item created; 1: confirmed grant; -1: ambiguous creation. */
enum { RF_GRANT_REJECTED = 0, RF_GRANT_CONFIRMED = 1, RF_GRANT_AMBIGUOUS = -1 };
typedef int (*rf_reward_grant)(int material, void *ctx);

typedef struct {
    uint64_t run_id;
    unsigned prepared, completed, cancelled, busy;
    unsigned count, index, attempts, failures, successes;
    int material;
} rf_reward_queue;

enum rf_reward_result {
    RF_REWARD_IDLE = 0,
    RF_REWARD_SUCCESS = 1,
    RF_REWARD_RETRY = 2,
    RF_REWARD_FAILED = 3
};

static inline unsigned rf_reward_quantity(int tier, int enabled) { return !enabled || tier <= 40 ? 1u : tier <= 80 ? 2u : 3u; }

static inline int rf_reward_pending(const rf_reward_queue *q) {
    return q && q->prepared && q->completed && !q->cancelled && q->index < q->count;
}

/* 开场时固定计划；未发完时禁止静默覆盖奖励。
 * Prepare once at run start; reject replacement of unfinished rewards. */
static inline int rf_reward_begin(rf_reward_queue *q, uint64_t run_id,
                                 int tier, int target, int enabled,
                                 rf_reward_rng rng, void *ctx) {
    if (!q || q->busy || tier < 1 || tier > 100 || target < 0 || target > 5)
        return 0;
    if (q->prepared && (q->run_id == run_id || rf_reward_pending(q)))
        return 0;
    unsigned count = rf_reward_quantity(tier, enabled);
    int material = target;
    if (!enabled || !target) {
        if (!rng) return 0;
        unsigned roll = rng(ctx);
        if (roll >= 5) return 0;
        material = (int)roll + 1;
    }
    *q = (rf_reward_queue){.run_id = run_id, .prepared = 1,
                           .count = count, .material = material};
    return 1;
}

/* 限时完成仅激活一次；失败或超时不发奖励。
 * A timed clear arms the plan once. Failed/untimed completion grants nothing. */
static inline int rf_reward_complete(rf_reward_queue *q, uint64_t run_id,
                                    int timed_clear) {
    if (!q || !q->prepared || q->run_id != run_id || q->completed || q->cancelled)
        return 0;
    q->completed = 1;
    if (!timed_clear) q->count = 0;
    return timed_clear != 0;
}

/* 每件最多尝试三次；失败记位后继续，避免无限阻塞。
 * At most three attempts per item; failed items do not block indefinitely. */
static inline enum rf_reward_result rf_reward_step(rf_reward_queue *q,
                                                   rf_reward_grant grant,
                                                   void *ctx) {
    if (!rf_reward_pending(q) || !grant || q->busy) return RF_REWARD_IDLE;
    unsigned bit = 1u << q->index;
    ++q->attempts;
    q->busy = 1;
    int success = grant(q->material, ctx);
    q->busy = 0;
    if (success == RF_GRANT_CONFIRMED) {
        q->successes |= bit;
        ++q->index;
        q->attempts = 0;
        return RF_REWARD_SUCCESS;
    }
    if (success == RF_GRANT_REJECTED && q->attempts < 3) return RF_REWARD_RETRY;
    q->failures |= bit;
    ++q->index;
    q->attempts = 0;
    return RF_REWARD_FAILED;
}

/* 显式取消停止后续原生调用，保留已发和失败记录。
 * Explicit cancellation prevents further native attempts, retains evidence. */
static inline void rf_reward_cancel(rf_reward_queue *q) {
    if (q) q->cancelled = 1;
}

#endif

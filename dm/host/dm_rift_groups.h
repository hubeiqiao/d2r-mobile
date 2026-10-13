#pragma once
#include <stdint.h>
/* 同图原生怪种的有界重组；只保存 ID，不保存原生指针。
 * Bounded recombination of native species on the same map; retain IDs, never native pointers. */
void dm_rift_groups_begin(int seq, int profile, int enabled);
void dm_rift_groups_tick(uint8_t *B, uint64_t G, uint64_t player, int level, uint32_t tick, uint32_t guardian);
void dm_rift_groups_death(int seq, uint32_t id);
void dm_rift_groups_cancel(int seq);
/* 离开秘境图时重置入场保护，不释放存活名额、不改总预算。
 * Leaving a rift map resets entry protection, not live capacity or run budget. */
void dm_rift_groups_leave(int seq);
/* 生产与测试共同使用的策略入口；回调保留原生读取/生成边界。
 * Shared production/test policy entry; callbacks preserve native read/spawn boundaries. */
typedef struct { uint32_t id, cls; int level, hostile, owned, boss, flags, dead; } dm_rg_candidate;
typedef struct {
 void *ctx;
 int (*collect)(void *, dm_rg_candidate *, int);
 int (*spot)(void *, int *, int *);
 /* 0 = definite failure, positive ID = success, UINT32_MAX = ambiguous creation. */
 uint32_t (*spawn)(void *, uint32_t, int, int);
 unsigned (*random)(void *, unsigned);
} dm_rg_ops;
void dm_rift_groups_step(int seq, int level, uint32_t tick, int x, int y, uint32_t guardian, const dm_rg_ops *ops);
int dm_rift_groups_alive(void);
int dm_rift_groups_created(void);
int dm_rift_groups_disabled(void);

struct ds_grid;
int dm_rift_groups_reachable(const struct ds_grid *grid, int dx, int dy);
/* 生成后原生字段检查，测试使用合成内存；不调用生成器。
 * Post-spawn native field checks; tests use synthetic memory, not the generator. */
int dm_rift_groups_validate(uint64_t unit, uint32_t cls, int level, uint64_t room);

# Overtime HUD UIKit regression

Run from repository root on a Mac with Xcode:

```sh
make -C dm/host/tests build/t_rift_overtime build/t_rift_clock
dm/host/tests/build/t_rift_overtime
dm/host/tests/build/t_rift_clock
zsh dm/ios/tests/rift_overtime_catalyst/run.sh
```

Actual production dm_rift.c producer, unchanged rendering infrastructure in dm_rift_hud.m, authored localization table and actual Mac Catalyst UIKit CALayers are executed. Five states in both languages: running899s, running900s before guardian, running930s with guardian, overtime guardian defeated, timed guardian defeated. The script dynamically discovers the scoped Xcode SDK, uses temporary outputs/system font fallback, and changes no simulator state. PNGs/results are local, not committed. This uses no game content.

Before UI change:10 cases,8 failed assertions (overtime state/rule labels missing for two running cases per language). After change:10 cases,0 failures. EN/ZH guardian-overtime PNGs opened and readable. These are actual UIKit offscreen results, not native game or iPad tests. Host test also checks boundary/long overtime/uninitialized/idle states, read-only state, and existing Continue/Quit request dispatch; real guest-clock freeze test remains passing6/6.

Maintainer native acceptance:

1. Both languages,11/13-inch iPad: cross15min before guardian and during guardian battle. Persistent overtime title/rule,0:00,live progress/guardian state. No forced return/purge/settlement and no hidden timer restart.
2. Continue to fill progress/kill guardian. Normal native drop path remains, no timed-clear/unlock/first-clear reward; safe completion/loot return follows existing code. Verify actual loot; host/UI cannot prove drops.
3. Use existing town scroll/free-town-portal action, open town Rift menu. Overtime title, continue-for-loot explanation, continue/quit rule. Continue preserves run/position; Quit retains existing confirmation/hold semantics. This patch does not add an automatic return portal or direct in-map quit action.
4. Timed clear and overtime settled screens retain their distinct text. Background/lock-screen freezes guest clock. No labels clipped and no extra focus interception.
5. With PR31, retain its capped next_tier/reward queue/old-loot safeguards and merge overtime fields/branches; do not replace full files with one PR's snapshot. #29 tier103 is independently tracked.

Full native build/rewrite/graphics/smoke/device acceptance requires locally owned3.3.94012 resources and is not run here. Keep draft until these pass. Primary reference: Blizzard2.3.0 restricts gem upgrades to timed clears;2.7.3 supports manually closing an active single-player GR. This patch clarifies existing project rules, does not import a new D3 reward system.

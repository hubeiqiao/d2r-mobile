# Delayed Guardian wave after returning to town

Based on public main `b94670c`, player build `10150343`, iPad17,1 / iPadOS 27.2. Priority: P1, blocks the current Rift.

## Evidence and cause

The provided diagnostic log records wave 1 (class 156, id 583) at tick 21530, a free town-portal cast, its death at 21843, and wave 2 scheduled 75 ticks later. The client then reports town level 109. At tick 21918, classes 544/id 584 and 243/id 585 spawn. The client later returns to level 111. The log has no server spawn-map field, so town placement is strongly supported, rather than independently proven by this log alone.

The production delayed-wave branch had no map restriction. `guard_wave()` places units using the player's current room. This is a confirmed source defect consistent with the report; it does not prove an existing Boss physically followed the player.

## Change and automated verification

Require an active run and a server level belonging to its map pool before dispatching a delayed wave. Keep the pending tick when outside that pool. Check the server map and room again inside the spawn entry. Record server/client level and coordinates once per spawned Guardian. No changes to waves, rewards, difficulty, timer, saves or unloaded-Boss death handling.

`t_rift_wave` includes the production source and exercises the actual scheduling predicate. The original predicate failed 6/14 checks; the fixed predicate passes 15/15, including town, other maps, deadline, return, completion/idle, cleared pending state and third wave. UBSan passes. `t_rift_next`, `t_rift_oldrun`, literal/i18n, native-widget-write and mode scans pass. This is host scheduling coverage, not native spawning or teleport coverage.

## Maintainer AI / real-device acceptance

1. Build with game files and run the full applicable repository checks. Use a 95+ two-wave and a 100 three-wave run.
2. Kill wave 1 while casting the free return portal; stay in town beyond 75 ticks. Repeat returning immediately before the next wave's deadline.
3. Confirm no Guardian in town, pending wave retained, no clear/reward prematurely granted. Return through Continue; confirm the pending wave appears once in a Rift map and can be fought.
4. Repeat between waves 2 and 3, several town round trips, and while a living Boss's room unloads. Unloaded Bosses must not count as dead.
5. Confirm the new spawn diagnostic always has a server level in the current run's map pool. Distinguish client lag from the authoritative server level.
6. Complete the run; verify rewards, completion and return portal still work. Abandon an inter-wave run and start another; ensure no old pending wave survives.

No game executable/RomFS or usable native runtime is available here. iOS build, native spawn, town teleport and real iPad verification remain required. Already affected active runs are not migrated/repaired by this patch. Existing all-spawn-failed handling is a separate risk: this patch preserves its behavior and does not introduce unbounded retries with uncertain native factory outcomes.

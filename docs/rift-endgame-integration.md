# Rift endgame integration and maintainer AI handoff

Integrates #25/#27 loot safety, #26/#28 encounters and #29/#30 capped tier display, with material targeting, same-map native groups, diagnostics and reproducible checks. Not a complete endgame release. Includes public `feb2bb9` (v1.0.8): the maintainer closed #23; verify compatibility, do not reimplement that fix. The maintainer imports into the internal source tree before exporting publicly. PR #31 is an integrated alternative, not additional patches to stack on #27/#28/#30.

## Implementation and evidence

| Feature | Implemented | Verified / pending |
|---|---|---|
|Completion/loot|Remove next-run portal, keep town portal, return to old loot from town; independent old-run close confirmation and cancel|Production-source flow tests; native portals/pickup/placement/save pending|
|Tier100|HUD/menu/settlement share capped next tier; overtime remains same tier|11 host assertions;8 actual UIKit EN/ZH cases:100+3→100,99+3→100,74+3→77,100+0→same|
|Routes/density|Four profiles, per-map variation, distinct routes, candidate fallback, unused/unbuilt restrictions retained|4000 synthetic routes and production region writes/restoration; native generation/progress pending|
|Native same-map groups|Existing native factory recombines observed ordinary hostile species; excludes bosses/elites/owned/nonhostile units|Production policy and synthetic native-field validation; actual spawning/AI/drops pending|
|Target materials|Random/1–5 category menu,1/2/3-item plan, locked at start; touch/controller dispatch|Production menu/API and9 dispatch assertions; in-game control layout/hit testing pending|
|Reward queue|Pending blocks replacing run, dead player deferred, ambiguous result never retried, visible results, save request|Policy/native-result classification/death branch; native placement/pickup/durable save pending|
|Scaling safety|Commit ID only after success; reject invalid level/maxlife/table reads;4096 saturation explicit|Combined host regression and strict compile; actual native writes pending|
|Elite variants|Pure policy and preview only, applied=0|Caps/ratios/exclusions tested; actual native modifiers/application protocol missing|
|Optional chase|Start/tick/finish/abandon/newgame state machine, native adapter disabled|Death/expiry/stale/once-cleanup/tri-state reward tests; no actual goblin/flee AI|

Ground items are not promised across application exit. Save requested is not save committed. The queue is process-local, cancels remaining slots on new game, and is not a crash-recovery transaction.

## Flags and safety budgets

Classic by default. Exact `DM_RIFT_ENCOUNTERS=1` enables profile experiments; additionally `DM_RIFT_REPEAT_REWARDS=1` enables target materials. `DM_RIFT_REWARD_TARGET=1..5` is a preset, overridden by menu choice, locked at start. `DM_RIFT_VARIANT_PREVIEW=1` diagnoses without writing.

Actual groups additionally require exact `DM_RIFT_EXTRA_GROUPS=1` and registered OBS17 death gate. Max3 groups/run,2 or3 units/group,4 alive,8 total. First entry and leave/reentry require125 ticks plus movement16; attempts at least250 ticks apart. Leaving retains lifetime/alive budgets. Disabled/stopped/guardian states return before native reads.

Positions require reachable loaded rooms on the same map10–24 tiles away, no feet fallback. Per group: at most one2000-node room traversal,64 intersecting rectangles,49×49 raster queries,16 native free-position adjustments and post-adjustment checks. Incomplete/overflow room chains fail closed. Validate returned unit type/class/ID/map/room; null/uncertain creation permanently stops additional groups for the run. Only authoritative death releases capacity; unload is not death. Cancellation stops spawning, does not invent a despawn interface.

Authored experimental rewards: tiers1–40 one,41–80 two,81–100 three; classic one random material, overtime none. **Null native factory result is ambiguous and must not retry**, because zero side effects are unverified. The pure queue retries at most3 times only when an adapter explicitly confirms no creation; the current native factory cannot make that claim from null. Nonnull invalid type/ground-mode/data also never retries. Creation≠pickup≠durable save commit.

## Reproducible checks

From repository root:

```sh
bash dm/host/tests/rift_source_checks.sh
zsh dm/ios/tests/rift_hud_catalyst/run.sh
```

Executed2026-10-13:13 host programs,8 iOS-shell programs without game fixtures,4 UBSan programs, strict C11 and literal/i18n/native-write/mode/touch-receipt source gates passed. HUDGEN repository gate passed; actual generation skipped without RomFS. UIKit harness uses production dm_rift.c, unchanged dm_rift_hud.m and authored localization; actual Mac Catalyst offscreen execution passed8 cases. EN/ZH100 PNGs were opened. Script prints temporary screenshots/results path; outputs stay out of git.

iPhoneOS SDK exists and UIKit syntax compilation passes. No Simulator runtimes installed. No executable/RomFS: full host/graphics/rewrite suites, native build/smoke, iPad gameplay not run. Directly running fixture-dependent binding programs lacks arguments; two skill source checks lack generated dm_touch_native_art.c; these are not passes. ASan hung/stopped, not passed. The selected script clearly skips resource-dependent checks; it does not replace full CONTRIBUTING acceptance.

Agents researched interfaces/design, implemented and independently audited source/host/actual UIKit, not native gameplay. Fixed server/client classifier mismatch, null-retry assumption, invalid maxlife, reentry protection, room-query work, paused pending queue, combined compilation and candidate fallback.

## Maintainer AI: implement remaining work, not discussion alone

Import integrated code, run both commands, then use legally owned3.3.94012 resources internally for `./setup.sh check` and full CONTRIBUTING build/rewrite/graphics/iOS/smoke acceptance. Never upload game files, derived tables, keys or packages.

| Order | Why blocked here | Required implementation/acceptance |
|---|---|---|
|1 Native safety|No executable/RomFS/iPad, host cannot execute original addresses|Classic/all profiles, outdoor/maze: spawn/AI/collision/drops/purge/reentry/death-gate coverage/budgets. iPad p95 input/frame/memory/thermal measurements; repair failures, attach logs/screenshots|
|2 Elite packs/affixes|Missing native elitepack creation, modifier enumeration/application/name/skill/drop initialization|Version-verified interfaces, never naked flags; native modifier reuse, type restrictions/exclusions/risk budgets and actual names. Telegraph before damage and retain safe movement. v1.0.8 now provides gsb base-stat reader, not modifier application or universal unit compatibility proof|
|3 Cross-map species/behavior|Only observed ordinary same-map species conservatively reusable; no AI/animation/resource/quest compatibility catalog|Locally verify melee/ranged/support-summon/durable-elite templates and compatibility whitelist; main plus at most one support species, no NPC/boss/quest pollution; derived catalogs stay local|
|4 Optional goblin|No verified chase/flee/cast/escape/despawn lifecycle|Connect native adapter to once/run/deadline/authoritative-death/once-reward/cleanup policy; test death/escape/pause/finish/abandon/newgame/unload. Must not block guardian/new run; custom optional event, not claimed standard D3 GR behavior|
|5 Difficulty/progress/economy|Configuration/few logs cannot establish actual generation, reachability or all-build balance|v1.0.8/#23 baseline, starter/endgame builds70/80/90/100, classic/all profiles: actual generation vs client observation, elites/progress/travel/trash/boss time/caps/deaths/yield/hour. Tune from evidence then implement target exchanges/long-term challenges; no invented103tiers|
|6 Durable items/rewards|No character-item transactions/persistent runID-slot ledger/native commit receipt|Validate factory side effects; model created/placed/pickedup/committed. Character+durable runID+slot ledger and reconciliation, inject termination after creation/pickup/during commit, no loss/duplication. More save requests are not transactions|
|7 Device UI|Offscreen UIKit checks HUD strings, not game controls/safe areas|11/13-inch iPad EN/ZH touch/controller: target materials/old return/confirmation/tier100/reward waiting-failure/rapid newrun. Actual game screenshots; background/lock-screen guest clock and effect duration tests|

Record commit/package/device/OS/runID/tier/profile/map sequence/configuration/actual modifiers/group-created-alive-dead/progress sufficiency/phase times/HP caps/reward-pickup-commit receipts/p95. Distinguish generated from observed units. Pinned map/profile switches are reproduction aids; randomness needs separate batch statistics.

Safety/data/input before diversity/fun. Report unexecuted items and reasons. Keep #23 resolved, do not reopen player-confirmed resolved cow-level feedback. Primary design evidence: [D3 research](d3-rift-design.md). Keep draft/experimental gates until native interfaces and device acceptance pass.

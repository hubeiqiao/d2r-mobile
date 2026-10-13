# Rift endgame integration and native acceptance

Combines #27 safe completion, #28 encounters/routes and #30 tier cap, adding repeat materials and diagnostics. Not a complete endgame release. Native executable/RomFS/device execution are unavailable. Xcode iPhoneOS SDK exists; UIKit syntax compilation passes. Host checks are not gameplay acceptance.

| System | Production connection | Evidence |
|---|---|---|
|Completion/cap|Town-only portal, old-loot return, explicit close, shared capped HUD|Combined source tests; native placement/render/pickup pending|
|Encounters/routes|Four profiles, map density, non-repeating routes and pool fallback|4000 synthetic draws, real region write/restore tests; native spawns/progress/performance pending|
|Material target|In-game random/1–5 selection and quantity preview, touch/controllerL/R|9 actual message-dispatch checks/menu API; rendering/hit areas pending|
|Material delivery|Connected native factory;1/2/3 bounded items, fixed target, retry only known noncreation, pending guard, no grants to dead player|Queue/classification/actual dead branch tests; native factory/placement/pickup/save pending|
|Elite variants|Pure policy complete; production preview only, applied=0|Bounds/caps/ratios/exclusions tested; native base/actual-modifier risk reads unverified, no writes|
|Optional goblin|Lifecycle hooked into start/tick/finish/abandon/new game, adapter explicitly disabled|Clock/death/stale/cleanup/tri-state reward tests; no creature or fleeingAI|
|Species/skill affixes/newboss/long-term gear exchange|Not connected|Native compatibility, economy and durable transaction acceptance missing|

Classic by default. Explicit `DM_RIFT_ENCOUNTERS=1` enables routing/density experiments; `DM_RIFT_REPEAT_REWARDS=1` additionally enables target choice and tier-based1/2/3 materials. `DM_RIFT_REWARD_TARGET=1..5` presets a category; in-menu choice overrides it. Target locks at start; retries do not reroll. `DM_RIFT_VARIANT_PREVIEW=1` only diagnoses. No goblin enable switch exists without verified adapter. Prior #28 was default-on; this integration corrects it to opt-in.

Authored reward prototype:1–40 one,41–80 two,81–100 three; not D3 values. Classic retains one random material; overtime grants none. Null native result means rejected creation, max3 attempts; nonnull unexpected type/mode/data means ambiguous, terminal for that slot, no guessed deletion/regrant. Save requested after creation; **creation is not pickup or successful durable commit**. Ground loot can still vanish across exit. Queue idempotence is process-local; new game cancels remaining slots, no crash recovery guarantee.

## Independent review

Seven agents handled interface investigation, D3/Last Epoch design, independent safety audit, variant/event/reward policies and UI. They reviewed source/host tests/scripted dispatch, not native gameplay.

Resolved findings include releasing completed-panel pause while rewards remain pending (the waiting panel must not stall its own queue). Other fixes: combined#27/#30 undefined `next_tier` reproduced by strict compilation then fixed; unverified progress sufficiency gates default-off experiments; valid one-maze routes retained, Balanced two-map fallback preserves maze; dead modes0/17 defer factory calls; failed stat/table reads do not commit scaledID; per-run/unit dedup with4096 saturation and explicit incomplete coverage; ambiguous rewards do not retry, event-1 is not success; visible terminal reward counts and old-loot confirmation without permanent masking of failures.

12 selected programs pass: variant/reward/event/integration/reward_ui/encounter/completion/tier_cap/clock/unit_level/doorspot/widget. Three policy tests pass UBSan. Strict C11 syntax, literal/i18n333 rows, native-write+selftest, mode registry, Python layout compile and diff pass. HUDGEN repository check passes; native generation skipped (noRomFS). ASan attempts hung/stopped, not passed. Full host suite/rewrite/gfx/iOS builds/native/device/screenshots not run.

## Native AI handoff

Use owned3.3.94012 files locally; never upload them or derived tables.

1. Validate version/call boundaries; identify species-pool bounds/counts/map compatibility, without guessing D2R fields from oldD2.
2. Verify native modifier reading/application, champion/unique distinctions, base-stat getter, stacked modifiers and removal. Preview only until verified; read actual combinations before exclusion/risk budgets.
3. Verify chase creature/escape/despawnAI, death confirmation and removal. Adapt bounded once/run lifecycle with cleanup; reuse stale/tri-state tests.
4. Check whether null factory results can leave units; verify created/placed tri-state. Reliable recovery requires(character,durable runID,reward slot) ledger and native save commit receipts; current implementation does not guarantee it.
5. #23 first, then fixed starter/endgame builds at70/80/90/100, all profiles/classic, outdoor/maze. Separate generated/loaded/elites/progress, travel/trash/Boss times, deaths, HPcaps, rewards and save receipts.
6. Verify2/3-map progress, exits/boundaries/reentry/pause/exhaustion/oldreturn, reward rejection/ambiguity/death/rapid newrun, both languages/controller; attach after-change screenshots. Measure iPad p95 frame time/thermal/input; reject unsafe density.

Design learning: D3 varies map/group/affix/progress value. [Last Epoch's2021 official retrospective](https://forum.lastepoch.com/t/monolith-overhaul/37245) emphasizes target farming/choice; [1.0 notes](https://forum.lastepoch.com/t/last-epoch-1-0-patch-notes/62536/3) reduce random modifiers' influence relative to base difficulty. Project inference: predictable baseline, clear material targets and compatibility/telegraph budgets; do not copy historical numbers. More evidence in `d3-rift-design.md`.


## 2026-10-13 actual group integration / 实际怪群接入

`DM_RIFT_ENCOUNTERS=1` plus exact `DM_RIFT_EXTRA_GROUPS=1` and the registered OBS17 death gate enable actual native ordinary-monster recombination from distinct hostile species already observed on the current map. This is not a cross-Act curated pool, champion pack or native skill affix. Default remains off. Server alignment uses the existing completion-sweep reader; no client targeting function is called on server units. Begin/tick/death/finish/abandon/new-game lifecycle is connected. At most 3 groups/run, 2 or 3 units/group, 4 alive, 8 created; first 125 ticks and movement16, then cooldown250. No feet fallback: reachable same-map loaded room, distance10..24. Null/ambiguous native creation permanently stops further attempts. Only authoritative death releases capacity; unloading does not. Result logs are bounded. Native spawn/AI/collision, return-entry safety and p95 performance remain unverified; the grid scans a49x49 window and requires a device frame-time check. Cancellation stops new spawning; it does not invent a despawn interface.

同时开启上述两个精确开关且产物带 OBS17 死亡门，才启用当前地图已观察普通敌对怪种的真实原生重组。默认关闭；不是跨幕怪种池、冠军怪群或技能词缀。接入开场、每拍、权威死亡、结算、放弃与换局；最多3群、同时4只、总计8只。冷却和入场保护、可达落点及异常停用有宿主回归。原生 AI、刷怪、返回入口安全及性能必须真机验收；取消只停止新增，不臆造销毁接口。

Selected host tests now total13, including production groups policy and authoritative quest-death argument bridge. Strict C11 and UBSan groups tests pass. Actual production HUD producer + unchanged UIKit renderer + localization table ran under Mac Catalyst:8 EN/ZH cases,0 failures (100+3→100,99+3→100,74+3→77,100+0→same). No Simulator runtimes are installed, so this is real UIKit execution, not iPad/game validation. Reproduction harness and screenshots: `/private/tmp/rift-uikit-validation` in the author workspace. Earlier statement that the iPhoneOS SDK was unavailable was incorrect; scoped DEVELOPER_DIR resolved it.

宿主13组验证通过；怪群 UBSan 通过。生产 HUD 和本地化表的真实 UIKit 离屏运行8个中英案例零失败。没有已安装的模拟器运行时，因此明确标注为 Mac Catalyst，不声称 iPad 或游戏通过。此前“iPhoneOS SDK 不可用”的判断已纠正。

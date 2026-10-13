# D3 Greater Rifts: randomness, affixes and D2 adaptation

2026-10-13. Evidence is Blizzard's historical releases/PTR notes, version-labelled. Old infinite-tier/trial-keystone systems are not asserted as current rules. Internal probabilities are unavailable. Proposals below are project design, not D3 facts. #28 remains experimental, not a complete implementation.

## Layers of variation

| Dimension | Official evidence | Proposed D2 lesson |
|---|---|---|
|Maps/layout|[2.4.3](https://news.blizzard.com/en-us/article/20426802/patch-2-4-3-now-live) tunes floor size/distribution, removes next-floor same-tileset bias, ensures a safe entrance|Measure route length, dead ends, empty space and entrance risk, not only Act; the21-map pool is limited|
|Monster sets|[2.7.3 PTR](https://news.blizzard.com/en-us/article/23774698/diablo-3-ptr-2-7-3-has-ended) separately adjusts map/group probabilities and curates Dream sets|Choose compatible maps and groups, not unconstrained mixing; density is one dimension|
|Progress value|2.7.3 changes specific monster progress; [2.4.0 PTR](https://news.blizzard.com/en-us/article/19941042/patch-2-4-0-ptr-patch-notes) changes elite orbs and total target|Compare time/risk/progress value, avoiding elite-only or near-instant clears; do not copy orb numbers blindly|
|Elite affixes|[2.0.1](https://news.blizzard.com/en-gb/article/12671560/patch-2-0-1-now-live) describes Frozen, pools, rotating Arcane, Mortar, displacement and telegraph changes|Alter positioning/timing, not only stats; these are elite abilities, distinct from a run-wide modifier|
|Constraints|[2.4.3](https://news.blizzard.com/en-us/article/20426802/patch-2-4-3-now-live) tunes Shielding/Reflects and restricts Juggernaut to rare elites|Use type restrictions, exclusions and a shared risk budget, not unrestricted stacking|
|Pacing nodes|[2.4.0 PTR](https://news.blizzard.com/en-us/article/19941042/patch-2-4-0-ptr-patch-notes) tunes Conduit/Shield/Speed; [2.6.1](https://news.blizzard.com/en-gb/article/21115840/patch-2-6-1-now-live) removes Shield reflection for high-density performance|Temporary advantages can affect routes, but exact spawn probabilities are unverified; density/chain effects require performance budgets|
|Boss/long-term goals|[2.1.2](https://news.blizzard.com/en-us/article/17561388/patch-2-1-2-now-live) fixes unfair boss damage/control overlap; [2.1.0](https://news.blizzard.com/en-us/article/15487814/patch-2-1-0-now-live) relates gem upgrades to tier/gem rank|Challenge should be answerable and rewards cumulative; do not copy obsolete keystone progression|

Design inference: build, space, monster behavior, affixes and route/temporary advantages jointly vary decisions. Controlled randomness should create different answerable situations, not merely random multipliers or punishing fishing. Sources prove multi-dimensional tuning, not complete algorithms. Exact affix weights/slots and full pylon spawning rules remain unverified.

## Current gaps

Original `dm_rift.c` fixes wilderness-first,3x ordinary density and250 progress. #28 adds density/elite ratios, variation, shuffled route composition and observations; species still use native map pools. Its first-version comment explicitly excludes champion/unique affix counts and resistance/immunity weights. Existing native random elites are not a completed Rift affix system.

At100, monster level already caps and HP caps at8388607, while character rewards increase power. Fix #23 and measure trash/boss phases and cap incidence before curves; increasing configured HP alone cannot fix this. HUD100→103 is a separate display defect addressed by #29/#30, without changing caps/rewards.

## Implementation sequence and acceptance

1. **Group library:** melee swarm, ranged pressure, support/summoner and durable elite role templates (design categories, not verified IDs). Locally verify animation/AI/room/quest dependencies; maintain map compatibility whitelist. Propose one main group plus at most one support group and a safe entrance. Validate native distribution, AI, quest events, corpse/exit cleanup and caps; never publish game-derived tables.
2. **Affix whitelist/risk budget:** use verified native D2 unique-modifier generation entry points, not raw flags pretending to add behavior. Categorize mobility, ground danger, burst/projectiles, control, reinforcement and defense. Propose at most one hard-control category per pack. Control+fast burst, narrow-space blockage and immunity combinations are candidate exclusions requiring all-class validation. Start with verified readable combinations; no untelegraphed lethal effects. This policy is proposed, not runtime code.
3. **Readability:** show actual modifiers, telegraph before damage, maintain maneuverable safe space. iPad occlusion/frame time/control response gate acceptance. Do not copy D3 art or claim safe native behavior from dummy tests.
4. **Progress/repeat value:** distinguish speed farming, progression and capped challenge. Keep100 truthful; best times/deathless records offer goals without privately opening103. Reuse existing shard/currency purposes; propose targeted category/slot exchange and bounded periodic rewards. Assess output/hour and inflation, not guaranteed perfect items or unlimited skill-point growth.
5. **Goblin:** explicitly a custom D2 chase event, not asserted as a standard D3 GR rule. Propose optional, at most once/run, bounded escape time, exactly-once kill/escape reward and exit/pause/new-run cleanup; must not block guardian/new run. Creature/AI, exclusion rules, loot/save consistency remain unverified and unimplemented. Author decides competitive-mode inclusion.

## Executable validation plan

Fixed starter/endgame builds, #23 first, multiple70/80/90/100 runs. Record map/group/affix, policy version/random result, configuration, native generation, loaded observation, elite encounters/skips, progress types, movement/trash/boss time, death cause, effective HP caps and output/hour. Separate common/rare scenarios; one geared character with few samples cannot establish global balance.

Compare old/new at each tier. Reentry must not reroll; paused game time stays frozen; completion/abandon restores originals. Map exhaustion must preserve safe loot flow; boss rewards must be collectible/saved. Measure iPad p95 frame time/thermal/input; reduce density or reject unsafe scenarios. Probability tests verify selection, not actual native generation or enjoyment.

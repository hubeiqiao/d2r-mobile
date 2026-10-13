# Rift encounter profiles (#26)

This PR implements encounter variation, not the full Diablo III monster-set system. The reference is Blizzard's historical Season 26 Orek's Dream (curated maps/compositions), 2.4.3 distribution changes, and 2.7.6 map/monster-set/density updates:
- https://news.blizzard.com/en-gb/article/23785083/season-26-the-fall-of-the-nephalem-patch-2-7-3-preview-blog
- https://news.blizzard.com/en-us/article/20426802/patch-2-4-3-now-live
- https://news.blizzard.com/en-us/article/23987087/diablo-iii-ptr-2-7-6-has-concluded

## Implemented policy

A run rolls once with system randomness; each map rolls its density once, retaining the values on re-entry. These are authored prototype settings, not Blizzard's weights:

| Profile | Weight | Density vs native | Elite multiplier vs existing tier rule | Progress target |
|---|---:|---:|---:|---:|
| Balanced |50%|3.0x|1.0x|300|
| Swarm |25%|3.8x|1.0x|350|
| Elite Hunt |20%|2.6x|1.5x|350|
| Dense Dream |5%|4.2x|1.5x|425|

Per-map density varies by -10/-5/0/+5/+10%. Density is capped at9000, elite limits at255, calculations use64-bit intermediates and original region values so repeated reconciliation does not compound. Progress rises alongside density/elite opportunity. This is not a guarantee of longer or harder fights; native generation/performance must be measured.

`DM_RIFT_ENCOUNTERS=0` restores old density3x, existing elite rule and250 progress. `DM_RIFT_NEED` remains an explicit override, now validated within1..100000. `DM_RIFT_ENCOUNTER_PROFILE=0..3` pins a profile for native testing; disabling takes precedence. A localized start notice names the profile.

Species still come from each original map's native monster pool. The implemented mix is ordinary density versus elite-group frequency. No unverified monster-table fields or class IDs are written, and Dense Dream does not yet use a curated species/map subset. Changing composition by species requires native compatibility checks for animation, AI, room sizing and quest events. No treasure goblin model/escape AI has been found in the public host code; native resources are unavailable here. A future D2 adaptation needs a locally verified creature and lifecycle, bounded spawning, clean escape/kill reward handling, and explicit identification as an adaptation; do not claim that a goblin exists just because a dummy host test passes.

## Route composition (follow-up implementation)

When enabled, Balanced uses one wilderness plus one/two mazes; Swarm/Dream prefer two wilderness plus one maze; Hunt uses one wilderness plus two mazes. Shuffle order, so wilderness is no longer mandatory first. Draw without replacement. With only one wilderness remaining, Swarm falls back to one wilderness/two mazes. Existing used/prebuilt candidate filtering remains; no new IDs or species are added. Disabling profiles preserves original routing. A4000-route host simulation checks composition, candidate membership, uniqueness, either starting type and insufficient pools; native entrances/exits/pet placement remain unverified.

See [d3-rift-design.md](d3-rift-design.md) for full research and proposed affix work.

## Measurements

`encounter-start` records roll/profile/target. `encounter-map` records region configuration and says `configured_only=1`. `encounter-summary` logs recognized loaded hostile units and kills using the existing client scan, grouped by the player's map at observation/death, with normal/minion/champion/unique/superunique counts; guardian is excluded. These are not full-map spawns or exact elite packs and can include adjacent loaded rooms. Different visits/movement change coverage. Unit-ID dedup sets stop adding new IDs at4096 rather than overwriting them; `overflow=1` marks partial counts. The Rift sequence must match; stale summaries print unavailable. The final client kill may arrive after the completion snapshot, so the snapshot is not an authoritative total.

## Acceptance before release

First incorporate/verify the pet-life fix #23 and safe completion #25. Build with owned resources, generate the new string table/overlay, and check Chinese/English start text. For fixed gear/skills at70/80/90/100, force all four profiles and classic control across wilderness/maze layouts. Compare server-generated elites (additional native instrument required), loaded observations, kills/progress, travel/clear/guardian time and deaths. Verify sufficient progress on two/three-map runs, original AI/loot, cap behavior, no re-entry reroll, completion/abandon restores values, and map exhaustion/new game creates fresh plans. Check iPad frame-time/p95, thermal state and touch response under Swarm/Dream; reject unsafe density settings. Do not infer actual spawn density from `configured_only`.

Host tests enumerate exact policy weights/variation bounds, overflow-safe clamping and4096-ID saturation; they also include production `dm_rift.c` and call real `rift_setup`/`region_set` against synthetic memory to verify region writes, non-compounding and restoration. The new region assertions fail6 times on untouched production code and pass on this implementation. They do not call native factory/AI or prove balance. Full app/native/iPad tests and screenshots are pending because game executables are missing. Rewards/health curve/species/goblin assets are not changed in this first PR.

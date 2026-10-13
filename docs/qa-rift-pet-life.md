# Rift minion life: Issue #23 verification handoff

This is a diagnostic change, not a fix for the feedback loop. The owner confirmed that `pet_hp_pump` can treat a native recalculation containing the previous Rift boost as a fresh base. Whether an observed value is unscaled or already includes that boost still needs native evidence. Do not blindly divide every changed value by the previous multiplier: a fresh native base would then lose legitimate life.

## Capturing evidence

Build this PR with locally owned game files. No special environment variable is required for event records. Reproduce with the player's summon or bound demon and buffs, then share the ordinary diagnostic log. Record the summon skill / demon species and active auras / Battle Orders; `auto_buff` records the applied setting, not which buffs actually affected the unit. Keep game files, saves and derived tables local.

Search for `pet-life`. Each event includes Rift sequence (`run`, process-local), state (0 idle, 1 running, 2 done), tier, server tick, unit ID, pet type, class, mode, unit level, previous cached base / last write / multiplier, observed maximum / current life, selected base, multiplier (1000 = x1), target before clamping, written values and immediate native readback. All life values are raw fixed point x256. `commits` is the save commit counter; it does not prove that these particular values reached disk.

Reasons: `new` = new cache entry, `sidecar` = existing class/value matched the sidecar, `rebase` = observed maximum differed from the cached write, `scale` / `restore` = existing entry while boosted / unboosted. Invalid nonpositive maxima get a separate `invalid` record. Stable entries emit no event unless the existing `DM_RIFT_TEST_PETDUMP=1` diagnostic is enabled.

Extra records and stat reads are bounded to 64 events per game pointer / Rift sequence / state, followed by one suppression line. Completion or abandonment refreshes the budget so restoration remains visible. Existing life-change logs and optional periodic dumps are unchanged. The immediate readback is not a trace of the native recalculation between ticks; add instrumentation at the actual recalculation site only after locating it with locally owned binaries.

## Native verification matrix

1. Start with clean summoned units; capture before entry, first scaling, and several native buff refreshes. Compare auto-buff off / on, no aura / aura, Battle Orders application / expiration, weapon swap, skill level-up and resummon. Record at least two same-class units separately.
2. Follow `observed_max` against `prev_last` and `prev_base`. Determine whether recalculation uses the original base or the host-modified maximum. Record the actual native stat source before and after buff application; avoid treating effective stat 7 as an unscaled base without proof.
3. Reproduce the cap-to-one cycle if possible. Distinguish a nonpositive read, a positive raw value 256 (one life), native arithmetic behavior, and the host cap. Host multiplication already uses `long long`; these records do not prove where overflow occurs.
4. Complete, abandon, return to town, change tiers, kill/resummon units, and start a new game. Check that legitimate buffs remain, current-life proportion is valid, and restored maxima match a clean control with the same skills and buffs.
5. Save/exit/reload with scaling on and off; verify the character save together with `.dm_pets` locally. Test duplicate classes and overlapping save commits. Do not infer successful persistence from a counter alone.

A gameplay fix needs a proven unscaled source or a distinction between fresh-base recalculation and recalculation of host-scaled values, with regressions for both. Repeated buff refreshes must not compound the Rift multiplier; skill changes must still work. Preserve sidecar compatibility and item/character data. Do not close #23 on the diagnostic PR alone.

## Scope of host checks

`t_rift_pet_trace` includes the production source and checks diagnostic fields, raw precision, cap flag, restoration records, rate limit and reset boundaries. It calls no native addresses. It does not exercise pet-table traversal, the full pump, buffs, save serialization or the original game's stat implementation. Strict syntax checking covers the production pump. Full build and native / iPad execution require game files unavailable in this environment.

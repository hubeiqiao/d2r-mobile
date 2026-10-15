# Act II minimap and town-portal diagnostics (#49, #47)

This patch collects missing runtime evidence. It does not fix or claim to reproduce either reported failure. No game UI visibility, geometry, teleport destinations or interaction retries are changed.

## Minimap

The existing 30-frame minimap poll now records a snapshot only when it changes, including when the AutoMap object is absent or hidden. Fields: latest server HUD area/validity, in-game/loading state, configured map mode, runtime mini/full flag, widget presence/rebuild/visibility and rectangle, UI rectangle and real screen dimensions. A negative UI Y alone is not evidence of offscreen placement; reconcile with the shifted UI rectangle. This is not a GPU/scissor trace, and the server HUD area may lag the client.

## Portal

Object interactions get a process-local sequence, frame, client player numeric id, source level, target class/id/coordinates and native return value. At 3, 30 and 120 guest frames, or a client-level change, record player level/coordinates/mode and latest server HUD area. These are observations, not arrival-success verdicts. A new object interaction supersedes the old trace explicitly; player exit/change/frame reset ends it. There is no logging cap that silently expires after the first forty clicks. No character names or new private paths are logged.

The existing OBS_LEVEL_WARP observer also logs ordinary-adventure server requests before Rift eligibility filtering, with server tick, numeric player id, source, requested destination, method and Rift state. Existing Rift rewrites retain their existing separate logs. Availability depends on the built shell having that observation gate. This is a request, not a confirmed arrival; correlate chronological player/level/tick evidence, not the process-local client sequence as an exact cross-thread transaction ID. No unverified portal-owner/paired-endpoint offsets were introduced.

## Checks and native handoff

Production-source fixtures: `t_map_diag` 115 checks (including 100 unchanged-state dedup checks), `t_interaction_diag` 15, `t_warp_diag` 12; all pass normally and under UBSan. The warp test verifies ordinary destinations remain unchanged and existing Rift routing still works. Existing unit-level (14), Rift-next and old-run tests pass. Syntax, literal/i18n, widget-write, input-mode and diff checks pass.

No game executable/RomFS is available. Full resource-dependent builds, native gameplay/device validation, rendered visibility and performance were not run. Maintainer AI should build with these diagnostics and reproduce A1 -> A2 travel, town -> sewers -> town, open/close inventory and lock/unlock. Share the first state snapshot where the minimap disappears, along with screenshots, and distinguish a hidden/missing object from visible-but-undrawn content.

For portals, compare free skill 411 versus scroll/tome, A1/A2/A5, close/far taps, immediate versus delayed use after casting, fresh versus long sessions and ordinary adventure after Rift/Boss Rush. On the first failed return tap retain the interaction sequence, server request and subsequent client samples; repeat the second tap. If no server request exists, investigate input/interaction; if destination is wrong, inspect the native pairing/overrides; if request is correct but arrival differs, inspect native transition/arrival. Do not add blind retries. After finding the cause, require failing-before/passing-after native evidence and repeat travel/save checks.

#!/bin/bash
# 无游戏资源的大秘境回归入口；只跑明确不依赖原版夹具的检查，不替代真机验收。
# Resource-free Rift regression entry; runs only checks independent of game fixtures, not device acceptance.
set -euo pipefail
TASK_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
cd "$TASK_ROOT"
HOST_TESTS=(t_rift_groups t_rift_variant t_rift_reward t_rift_event t_rift_integration t_rift_reward_ui t_rift_encounter t_rift_completion t_rift_tier_cap t_rift_clock t_unit_level t_doorspot t_widget)
HOST_TARGETS=()
for task_test in "${HOST_TESTS[@]}"; do HOST_TARGETS+=("build/$task_test"); done
make -C dm/host/tests "${HOST_TARGETS[@]}"
for task_test in "${HOST_TESTS[@]}"; do "dm/host/tests/build/$task_test"; done
IOS_TESTS=(t_touch_controls t_binding_freshness t_binding_group t_native_hud_identity t_portal_sequence t_skill_aim t_joy_travel t_metal_cache_guard)
make -C dm/ios/tests "${IOS_TESTS[@]}"
for task_test in "${IOS_TESTS[@]}"; do "dm/ios/tests/$task_test"; done
python3 dm/host/tests/native_write_scan.py --selftest dm/host
python3 dm/host/tests/mode_scan.py dm
python3 dm/ios/tools/dm_i18n.py --check
python3 dm/i18n/scan_literals.py
python3 dm/ios/tests/test_touch_receipt.py --source dm/ios/Sources/front_ios.m
bash dm/ios/tools/check_hud_gen.sh
clang -fsyntax-only -std=c11 -Wall -Wextra -Werror -I dm/host dm/host/dm_rift.c dm/host/dm_rift_groups.c dm/host/hle_target.c dm/host/dm_rift_ui.c
TASK_UBSAN="$(mktemp -d "${TMPDIR:-/tmp}/rift-ubsan.XXXXXX")"
trap 'rm -rf "$TASK_UBSAN"' EXIT
for task_test in t_rift_groups t_rift_variant t_rift_reward t_rift_event; do
    TASK_EXTRA=("dm/host/tests/$task_test.c")
    if [[ "$task_test" == t_rift_groups ]]; then TASK_EXTRA+=(dm/host/dm_rift_groups.c); fi
    clang -O1 -std=c11 -fsanitize=undefined -fno-sanitize-recover=undefined -Wall -Wextra -Werror -ffunction-sections -fdata-sections -Wl,-dead_strip -I dm/host -o "$TASK_UBSAN/$task_test" "${TASK_EXTRA[@]}"
    "$TASK_UBSAN/$task_test"
done
printf '%s\n' 'RIFT SOURCE CHECKS PASS: 13 host + 8 iOS shell programs, source gates, 4 UBSan programs.'
printf '%s\n' 'SKIP: game/native binding fixtures, generated skill art checks, Metal rewrite/cache suites, full build/smoke, device gameplay. These require local game resources/ground truth.'

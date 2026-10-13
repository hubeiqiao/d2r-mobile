#!/bin/zsh
# 独立 Catalyst 验证：产物仅在临时目录，不操作模拟器或游戏资源。
# Isolated Catalyst validation: temporary outputs only, no simulator/game-resource changes.
set -euo pipefail
TASK_SOURCE_DIR="${0:A:h}"
TASK_REPO_DIR="${TASK_SOURCE_DIR}/../../../.."
export DEVELOPER_DIR="${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}"
TASK_SDK="$(xcrun --sdk macosx --show-sdk-path)"
TASK_OUTPUT="$(mktemp -d "${TMPDIR:-/tmp}/rift-overtime-catalyst.XXXXXX")"
TASK_APP="$TASK_OUTPUT/HudValidation.app"
mkdir -p "$TASK_APP/Contents/MacOS"
TASK_TARGET="$(uname -m)-apple-ios16.0-macabi"
TASK_FLAGS=(-target "$TASK_TARGET" -isysroot "$TASK_SDK" -isystem "$TASK_SDK/System/iOSSupport/usr/include" -I "$TASK_REPO_DIR/dm/host" -O1 -Wall -Wextra -Werror -Wno-unused-function)
xcrun clang "${TASK_FLAGS[@]}" -ffunction-sections -fdata-sections -c "$TASK_SOURCE_DIR/fixture.c" -o "$TASK_OUTPUT/fixture.o"
xcrun clang "${TASK_FLAGS[@]}" -iframework "$TASK_SDK/System/iOSSupport/System/Library/Frameworks" -fobjc-arc -Wl,-dead_strip -framework UIKit -framework Foundation -framework QuartzCore -framework CoreText -framework CoreGraphics "$TASK_SOURCE_DIR/main.m" "$TASK_REPO_DIR/dm/ios/Sources/dm_rift_hud.m" "$TASK_OUTPUT/fixture.o" -o "$TASK_APP/Contents/MacOS/HudValidation"
cat > "$TASK_APP/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"><plist version="1.0"><dict><key>CFBundleExecutable</key><string>HudValidation</string><key>CFBundleIdentifier</key><string>org.d2rmobile.rifthudvalidation</string><key>CFBundleName</key><string>Rift HUD Validation</string><key>CFBundlePackageType</key><string>APPL</string><key>LSUIElement</key><true/><key>NSHighResolutionCapable</key><true/></dict></plist>
PLIST
codesign -s - "$TASK_APP"
printf 'UIKit output: %s\n' "$TASK_OUTPUT"
# 不加载本机用户资源；验证渲染器自带的系统字体回退。 | Do not load user resources; validate built-in font fallback.
env -u DM_ROMFS RIFT_HUD_OUTPUT="$TASK_OUTPUT" "$TASK_APP/Contents/MacOS/HudValidation"

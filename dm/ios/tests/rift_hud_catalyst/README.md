# Rift HUD: actual UIKit rendering / 秘境 HUD：真实 UIKit 渲染

## Run / 执行

On a Mac with Xcode and the Mac Catalyst UIKit SDK installed, run from the repository root:

在安装 Xcode 与 Mac Catalyst UIKit SDK 的 Mac 上，从仓库根目录运行：

```sh
zsh dm/ios/tests/rift_hud_catalyst/run.sh
```

The script scopes `DEVELOPER_DIR` to `/Applications/Xcode.app/Contents/Developer` by default; set it explicitly to use another Xcode installation. It does not change `xcode-select`, install runtimes, create/boot simulators, or use a game save/resource pack. Builds and screenshots remain in a new temporary directory printed as `UIKit output:`. No binaries or screenshots belong in the repository.

脚本仅为本进程默认设置 `DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer`；使用其他 Xcode 时可显式设置。它不修改 `xcode-select`、不安装运行时、不创建或启动模拟器，也不使用存档或游戏资源包。构建和截图保存在新建临时目录，路径以 `UIKit output:` 输出。不要把二进制或截图提交到仓库。

## What executes / 实际执行的内容

- `fixture.c` includes production `dm/host/dm_rift.c`, sets its actual completed-run state, and calls the actual HUD producer. It supplies only clock/filesystem hooks; no original game address is invoked.
- `main.m` links the unchanged production `dm/ios/Sources/dm_rift_hud.m`, selects strings directly from the authored production `dm_i18n_tab.h`, walks actual `CATextLayer` labels, and renders the real layer tree with `UIGraphicsImageRenderer`.
- The script clears `DM_ROMFS` for this executable, so the existing system-font fallback is tested. The background is neutral and the logical viewport is 1194×834. This is an isolated offscreen renderer, with no application window or gameplay.

- `fixture.c` 包含生产 `dm/host/dm_rift.c`，设置其真实已完成状态，并使用真实 HUD 数据生产函数。只替代时钟与文件系统 hook，不调用原版地址。
- `main.m` 链接未改动的生产 `dm/ios/Sources/dm_rift_hud.m`，从自主维护的生产 `dm_i18n_tab.h` 选择文字，检查真实 `CATextLayer`，并通过 `UIGraphicsImageRenderer` 绘制真实图层树。
- 脚本清除该程序的 `DM_ROMFS`，验证原有系统字体回退。背景中性，逻辑视口为 1194×834。这是独立离屏渲染，没有应用窗口或游戏运行。

## Assertions and evidence / 断言和证据

Each of four completion cases runs in English and Chinese, for eight cases total:

四种结算情况分别验证英文和中文，共八项：

| Completed tier / 完成层数 | Increase / 增量 | Expected / 预期 |
| --- | --- | --- |
| 100 | 3 | Next 100 / 下次100 |
| 99 | 3 | Next 100 / 下次100 |
| 74 | 3 | Next 77 / 下次77 |
| 100 | 0 | Same level / 仍是这一层 |

The executable checks the actual next-tier label and rejects any label containing 101, 102, or 103. Screenshot/results write errors fail the run. Exit status 0 and `8 checks, 0 failures` mean those renderer checks passed. `results.txt` captures the labels; `hud-{en,zh}-{tier}-{inc}.png` contains each real UIKit rendering. Open the PNGs to assess readable layout; a successful write alone is not visual approval.

程序检查真实下一层文字，并拒绝包含101、102或103的标签。截图或结果写入失败也会使测试失败。退出码0且显示 `8 checks, 0 failures` 表示这些渲染断言通过。`results.txt` 记录标签，`hud-{en,zh}-{tier}-{inc}.png` 保存每项真实 UIKit 渲染。需打开图片检查可读性，成功写入图片不代表已完成视觉审核。

Executed on 2026-10-13: strict `-Wall -Wextra -Werror` build (unused production static functions suppressed for this bounded fixture), eight checks passed. English and Chinese 100+3 screenshots were opened and inspected.

2026-10-13 已执行：严格 `-Wall -Wextra -Werror` 构建（针对有界 fixture 抑制未使用的生产 static 函数），八项断言通过。已打开并检查中英文100+3截图。

## Scope / 验证边界

This validates actual Mac Catalyst UIKit execution, the production HUD data path, and authored localization. It does not validate native Diablo II gameplay, iPad device/simulator behavior, the original font, viewport/safe-area adaptation, or interaction/portal/reward mechanics. Those require separate in-game/device acceptance. No simulator runtime is needed for this harness.

验证范围是实际 Mac Catalyst UIKit 执行、生产 HUD 数据路径及自主维护的本地化文字。未验证原生 Diablo II 游戏、iPad 真机或模拟器、原版字体、视口与安全区适配，或交互、传送门和奖励机制。上述内容需另做游戏内和设备验收。本 harness 不需要模拟器运行时。

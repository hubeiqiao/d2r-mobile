# 大秘境终局整合与原生验收

本分支整合#27安全结算、#28随机路线/遭遇、#30层数封顶，并加入重复材料奖励与诊断。它不是完整终局系统的发布版。当前无游戏可执行文件、RomFS或真机运行。已找到 Xcode iPhoneOS SDK，UIKit 语法编译通过；不能把宿主验证当成真实游戏体验。

## 当前实现状态

| 系统 | 生产接入 | 验证状态 |
|---|---|---|
|安全结算/100层上限|只保留回城门，旧场返回与独立关闭确认，HUD同源封顶|组合生产源码测试通过；原生门/布局/拾取待验|
|随机遭遇与路线|四方案、每图密度、去重路线和候选池不足回退|4000条合成路线及真实区域写入/恢复测试；原生生成/进度可达/性能待验|
|重复材料目标|游戏内随机/1–5类别切换，数量预览，触屏与手柄L/R|实际消息分发9项与菜单API测试通过；控件渲染/热区未运行|
|重复材料发放|已接原生物品工厂；1/2/3件上限，固定目标，明确未创建才重试；待发阻止新场覆盖，死者不发|队列/生产结果分类/实际死亡分支测试通过；原生创建、落点、拾取、存档待验|
|精英属性变体|纯策略完成，生产只打印预览，`applied=0`|边界/上限/比例/排除测试；缺原生基数与实际词缀风险读回，禁止写入|
|可选哥布林事件|状态机完成并接场次开始、tick、完成/放弃/新局；原生适配器固定关闭|计时/死亡/过期/一次清理/三态奖励策略通过；无真实单位或逃跑AI|
|品种重组、技能型词缀、Boss新机制、长期装备兑换|未接入|原生接口、兼容库、经济与持久化验收仍欠缺|

默认经典规则。验证包明确设置`DM_RIFT_ENCOUNTERS=1`才启用路线/密度试验；再设`DM_RIFT_REPEAT_REWARDS=1`启用重复材料目标选择和1/2/3件。`DM_RIFT_REWARD_TARGET=1..5`可预设，玩家在选层页切换后优先使用玩家选择。每次开局锁定目标，重试不重抽。`DM_RIFT_VARIANT_PREVIEW=1`仅诊断、不施加属性。没有启用哥布林的开关，因为适配器未验证。旧版#28默认开启，整合版已更正为明确opt-in。

奖励按1–40层1件、41–80层2件、81–100层3件的作者试验值，不是D3权重。经典仍随机1件。超时不发。原生工厂返回空＝确认未创建，最多3次；非空但类型/模式/数据状态不符＝不确定，该项停止，不猜测删除或重发。确认创建后再请求保存；**创建≠拾取≠保存提交成功**，地面物品退出应用仍可能丢失。队列是进程内防重，不是崩溃恢复事务；新局取消未发余项，不承诺自动补发。

## 多agent审核及已修正项

7个agents承担接口调查、D3/Last Epoch设计研究、独立安全审核、属性策略、事件策略、奖励策略、奖励UI。审核者没有游戏资源，执行的是源码审查/宿主测试/脚本流程，不是真机游戏体验。

- 确认复现：#27删除`next_tier`与#30新增调用组合后严格编译失败；整合保留统一函数，所有组合测试通过。
- 进度可达性不能由配置推断：默认关闭新方案，保留原生验收阻塞；未伪造“所有图一定足够”的结论。
- 每方案按可用野外/迷宫编排，不再因只剩一张迷宫而丢弃合法路线；修正Balanced两图回退必须保留迷宫。
- 玩家死亡模式0/17不调用奖励工厂，等待可操作状态。
- 属性读数/表未就绪不记成已增强；按场次与单位ID去重，只在成功/明确无需改时记入。4096-ID满后停止增强新单位，汇总标不完整，需原生规模验收。
- 奖励待发时自动放开完成页面的游戏暂停，避免“请稍候”本身阻止队列完成。
- 模糊创建禁止重复发放；事件奖励`-1`不再当成功。奖励终态可见，开启新场确认保留旧场，不靠永久阻塞掩盖失败。

## 已运行检查

12个测试程序：variant、reward、event、integration、reward_ui、encounter、completion、tier_cap、clock、unit_level、doorspot、widget。全部通过。3个策略测试用UBSan通过。C11严格语法、字面量/333行串表、native-write+selftest、模式登记、布局Python编译、diff检查通过。HUDGEN入库检查通过；真实生成检查因缺RomFS跳过。ASan曾挂起并中止，不算通过。完整宿主套件、重写/图形回归、iOS构建、原生运行、真机截图未执行。

## 原生AI所需接口与验收交接

必须在拥有合法原版3.3.94012文件的环境完成，不上传文件或派生表：

1. 确认版本与调用边界；读取地图怪物品种池，提供有效条目范围/数量/地图兼容性，不根据旧D2偏移猜D2R结构。
2. 确认精英原生修饰的读取与生成入口、冠军/金怪差别、单位基数读取、层叠修饰及移除语义。未验证前只允许属性预览；需读回实际组合并做互斥/风险预算。
3. 核验可追逐单位、原生逃跑/消失AI、死亡确认和删除入口，建立有时限、每场一次、完成/放弃清理的适配器。当前事件接口三态奖励与过期回调测试可直接复用。
4. 核验物品工厂是否可能返回空却留下单位；确认创建/落点三态。若要可靠崩溃恢复，需要角色+持久runID+奖励槽的台账及原生保存提交回执。当前代码不承诺此保证。
5. 先#23随从修正，再固定开荒/成型两套装备，在70/80/90/100、四方案/经典、野外/迷宫采样。记录实际生成、已加载、精英类型、进度、移动/清怪/Boss耗时、死亡、HP封顶、奖励产出与保存回执，不能混为同一个计数。
6. 检查两/三图进度足够；所有出口/边界、首次进入、重进、暂停、图耗尽重开、旧场返回、奖励失败/不明/死亡、快速新场、界面中英与手柄，附修复后截图。iPad测p95帧时间/温控/输入，未达标否决高密度。

设计学习：D3的地图×怪群×词缀×进度价值是多维变化；[Last Epoch 2021官方复盘](https://forum.lastepoch.com/t/monolith-overhaul/37245)强调目标刷取与玩家选择，[1.0补丁](https://forum.lastepoch.com/t/last-epoch-1-0-patch-notes/62536/3)降低随机修饰相对基础难度的影响。适配推论是保留可预测基础挑战、明确目标材料，并在兼容与预警预算内随机；不是把这些历史数值照搬。更多D3证据见`d3-rift-design.zh-CN.md`。


## 2026-10-13 actual group integration / 实际怪群接入

`DM_RIFT_ENCOUNTERS=1` plus exact `DM_RIFT_EXTRA_GROUPS=1` and the registered OBS17 death gate enable actual native ordinary-monster recombination from distinct hostile species already observed on the current map. This is not a cross-Act curated pool, champion pack or native skill affix. Default remains off. Server alignment uses the existing completion-sweep reader; no client targeting function is called on server units. Begin/tick/death/finish/abandon/new-game lifecycle is connected. At most 3 groups/run, 2 or 3 units/group, 4 alive, 8 created; first 125 ticks and movement16, then cooldown250. No feet fallback: reachable same-map loaded room, distance10..24. Null/ambiguous native creation permanently stops further attempts. Only authoritative death releases capacity; unloading does not. Result logs are bounded. Native spawn/AI/collision, return-entry safety and p95 performance remain unverified; the grid scans a49x49 window and requires a device frame-time check. Cancellation stops new spawning; it does not invent a despawn interface.

同时开启上述两个精确开关且产物带 OBS17 死亡门，才启用当前地图已观察普通敌对怪种的真实原生重组。默认关闭；不是跨幕怪种池、冠军怪群或技能词缀。接入开场、每拍、权威死亡、结算、放弃与换局；最多3群、同时4只、总计8只。冷却和入场保护、可达落点及异常停用有宿主回归。原生 AI、刷怪、返回入口安全及性能必须真机验收；取消只停止新增，不臆造销毁接口。

Selected host tests now total13, including production groups policy and authoritative quest-death argument bridge. Strict C11 and UBSan groups tests pass. Actual production HUD producer + unchanged UIKit renderer + localization table ran under Mac Catalyst:8 EN/ZH cases,0 failures (100+3→100,99+3→100,74+3→77,100+0→same). No Simulator runtimes are installed, so this is real UIKit execution, not iPad/game validation. Reproduction harness and screenshots: `/private/tmp/rift-uikit-validation` in the author workspace. Earlier statement that the iPhoneOS SDK was unavailable was incorrect; scoped DEVELOPER_DIR resolved it.

宿主13组验证通过；怪群 UBSan 通过。生产 HUD 和本地化表的真实 UIKit 离屏运行8个中英案例零失败。没有已安装的模拟器运行时，因此明确标注为 Mac Catalyst，不声称 iPad 或游戏通过。此前“iPhoneOS SDK 不可用”的判断已纠正。

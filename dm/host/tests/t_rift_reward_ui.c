// 生产UI消息分发回归；不生成原生布局，也不宣称真机手感。
// Production UI message-dispatch regression; no native layout generation or device feel claims.
#include <assert.h>
#include "../dm_rift_ui.c"
int main(void) {
 atomic_store(&g_act, 0);
 dm_rift_ui_on_uimsg("Other", "ClosePanel", "TouchRF_RewardPrev"); assert(!atomic_load(&g_act));
 dm_rift_ui_on_uimsg("PanelManager", "OpenPanel", "TouchRF_RewardPrev"); assert(!atomic_load(&g_act));
 dm_rift_ui_on_uimsg("PanelManager", "ClosePanel", "TouchRF_RewardPrev"); assert(atomic_exchange(&g_act, 0)==7);
 dm_rift_ui_on_uimsg("PanelManager", "ClosePanel", "TouchRF_RewardNext"); assert(atomic_exchange(&g_act, 0)==8);
 dm_rift_ui_on_uimsg("PanelManager", "ClosePanel", "TouchRF_Start"); assert(atomic_exchange(&g_act, 0)==1);
 dm_rift_ui_on_uimsg("PanelManager", "ClosePanel", "TouchRF_Confirm"); assert(atomic_exchange(&g_act, 0)==3);
 dm_rift_ui_on_uimsg("PanelManager", "ClosePanel", "TouchRF_Cancel"); assert(atomic_exchange(&g_act, 0)==2);
 dm_rift_ui_on_uimsg("PanelManager", "ClosePanel", "TouchRF_M1"); assert(atomic_exchange(&g_step, 0)==-1);
 dm_rift_ui_on_uimsg("PanelManager", "ClosePanel", "TouchRF_P10"); assert(atomic_exchange(&g_step, 0)==10);
 puts("Reward UI production action dispatch: 9 checks passed");
 return 0;
}

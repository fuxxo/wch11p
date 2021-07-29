#ifndef __MAC80211__OPS_H__
#define __MAC80211__OPS_H__

#include "wireless-stack.h"

void WCH11P_tx(struct ieee80211_hw *hw, struct ieee80211_tx_control *control, struct sk_buff *pSkb);
int WCH11P_start(struct ieee80211_hw *hw);
void WCH11P_stop(struct ieee80211_hw *hw);
int WCH11P_add_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif);
void WCH11P_remove_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif);
int WCH11P_set_antenna(struct ieee80211_hw *hw, u32 tx_ant, u32 rx_ant);
int WCH11P_get_antenna(struct ieee80211_hw *hw, u32 *tx_ant, u32 *rx_ant);
void WCH11P_bss_info_changed(struct ieee80211_hw *hw,
			 struct ieee80211_vif *vif,
			 struct ieee80211_bss_conf *info,
			      u32 changed);
int WCH11P_config(struct ieee80211_hw *hw, u32 changed);
void WCH11P_configure_filter(struct ieee80211_hw *hw,
			     unsigned int changed_flags,
			     unsigned int *total_flags,
			      u64 multicast);
int WCH11P_add_chanctx(struct ieee80211_hw *hw, struct ieee80211_chanctx_conf *ctx);
void WCH11P_remove_chanctx(struct ieee80211_hw *hw, struct ieee80211_chanctx_conf *ctx);
void WCH11P_change_chanctx(struct ieee80211_hw *hw, struct ieee80211_chanctx_conf *ctx, u32 changed);
int WCH11P_assign_vif_chanctx(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_chanctx_conf *ctx);
void WCH11P_unassign_vif_chanctx(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_chanctx_conf *ctx);

#endif

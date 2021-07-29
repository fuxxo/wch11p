#ifndef __DEBUG_IEEE80211_H__
#define __DEBUG_IEEE80211_H__

#include "wireless-stack.h"

typedef struct {
  enum nl80211_iftype type;
  char *str;
} nl80211_iftype_debug_t;

char *display_struct_mac_address(const struct mac_address* mac);
char *display_struct_ieee80211_rate(const struct ieee80211_rate*);
char *display_struct_ieee80211_supported_band(const struct ieee80211_supported_band *band);
char *display_struct_ieee80211_channel( const struct ieee80211_channel *channel);
char *display_struct_cfg80211_chan_def( const struct cfg80211_chan_def *chandef);
char *display_struct_ieee80211_tx_info_1_1( const struct ieee80211_tx_info * tx_info);
char *display_struct_ieee80211_tx_control( const struct ieee80211_tx_control * txc);
char *display_struct_ieee80211_conf( const struct ieee80211_conf *conf);
char *display_struct_ieee80211_chanctx_conf(const struct ieee80211_chanctx_conf* ctx);
char *display_struct_ieee80211_txrx_stypes( const struct ieee80211_txrx_stypes* stypes );
//char *display_nl80211_iftype(enum nl80211_iftype * type);
char *display_nl80211_iftype(const u16 * type);
char *display_struct_ieee80211_sta_rates( const struct ieee80211_sta_rates * sta_rates);
char *display_struct_ieee80211_sta(const struct ieee80211_sta *sta);
char *display_struct_ieee80211_iface_limit(const struct ieee80211_iface_limit* limit);
char *display_struct_ieee80211_iface_combination(const struct ieee80211_iface_combination* comb);
char *display_struct_ieee80211_vif(const struct ieee80211_vif * vif );
char *display_struct_wiphy( const struct wiphy * wiphy);
char *display_struct_ieee80211_hw( const struct ieee80211_hw *hw);

#endif

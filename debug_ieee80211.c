#include "debug.h"
#include "debug_ieee80211.h"

const nl80211_iftype_debug_t nl80211_iftype_debug[] = {
  {.type=NL80211_IFTYPE_UNSPECIFIED, .str="NL80211_IFTYPE_UNSPECIFIED"},
  {.type=NL80211_IFTYPE_ADHOC, .str="NL80211_IFTYPE_ADHOC"},
  {.type=NL80211_IFTYPE_STATION, .str="NL80211_IFTYPE_STATION"},
  {.type=NL80211_IFTYPE_AP, .str="NL80211_IFTYPE_AP"},
  {.type=NL80211_IFTYPE_AP_VLAN, .str="NL80211_IFTYPE_AP_VLAN"},
  {.type=NL80211_IFTYPE_WDS, .str="NL80211_IFTYPE_WDS"},
  {.type=NL80211_IFTYPE_MONITOR, .str="NL80211_IFTYPE_MONITOR"},
};

char *display_struct_mac_address(const struct mac_address* mac) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  if (mac == NULL) {
    layout = "(struct mac_address){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }
  
  layout = "(struct mac_address){.addr:(u8){%02X:%02X:%02X:%02X:%02X:%02X}}";
  size = strlen(layout)-12+1;
  info = kmalloc(size, GFP_KERNEL);

  snprintf( info, size, layout, mac->addr[0], mac->addr[1], mac->addr[2], mac->addr[3], mac->addr[4], mac->addr[5]);
  
  return info;
  
};

char *display_struct_ieee80211_rate(const struct ieee80211_rate *rate) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  if (rate == NULL) {
    layout = "(struct ieee80211_rate){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  layout = "(struct ieee80211_rate){.flags:(u32){%08X} .bitrate:(u16){%u} .hw_value:(u16){%u} .hw_value_short:(u16){%u}}";
  info = kmalloc( 1024, GFP_KERNEL);
  sprintf( info, layout, rate->flags, rate->bitrate, rate->hw_value, rate->hw_value_short );
  info = shrink( info );
  
  return info;
}

char *display_struct_ieee80211_supported_band(const struct ieee80211_supported_band *band) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;
  
  char * channels = NULL;
  char * channel_temp = NULL;
  char * bitrates = NULL;
  char * bitrate_temp = NULL;

  char * layout_array = "[%d]%s";
  unsigned int index = 0;
  unsigned int i = 0;
  
  if (band == NULL) {
    layout = "(struct ieee80211_supported_band){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  layout = "(struct ieee80211_supported_band){.channels:%s .bitrates:%s .band:(enum ieee80211_band){%u} .n_channels:(int){%d} .n_bitrates:(int){%d} .ht_cap:(struct ieee80211_sta_ht_cap){TODO} .vht_cap:(struct ieee80211_sta_vht_cap){TODO} }";

  channels = kmalloc(2048, GFP_KERNEL); channels[0] = '\0';
  index = 0;
  for( i=0; i<band->n_channels; i++ ) {
    channel_temp = display_struct_ieee80211_channel( &band->channels[i] );
    index += sprintf( &channels[index], layout_array, i, channel_temp);
    if (channel_temp != NULL) kfree(channel_temp);
    if (index >= 2048) break;
  }

  bitrates = kmalloc(2048, GFP_KERNEL); bitrates[0] = '\0';
  index = 0;
  for( i=0; i<band->n_bitrates; i++) {
    bitrate_temp = display_struct_ieee80211_rate( &band->bitrates[i] );
    index += sprintf( &bitrates[index], layout_array, i, bitrate_temp);
    if (bitrate_temp != NULL) kfree(bitrate_temp);
    if (index >= 2048) break;
  }

  info = kmalloc( 2048, GFP_KERNEL);
  sprintf( info, layout, channels, bitrates, (u32)band->band, band->n_channels, band->n_bitrates);
  if (channels != NULL) kfree(channels);
  if (bitrates != NULL) kfree(bitrates);
  info = shrink( info );

  return info;
};


char *display_struct_ieee80211_channel( const struct ieee80211_channel *channel) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  if (channel == NULL) {
    layout = "(struct ieee80211_channel){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  layout = "(struct ieee80211_channel){.band:(enum ieee80211_band){%08X} .center_freq:(u16){%u} .hw_value:(u16){%u} .flags:(u32){%08X} .max_antenna_gain:(int){%d} .max_power:(int){%d} .max_reg_power:(int){%d} .beacon_found:(bool){%u} .orig_flags:(u32){%u} .orig_mag:(int){%d} .orig_mpwr:(int){%d} .dfs_state:(enum nl80211_dfs_state){%08X} .dfs_state_entered:(unsigned long){%u} .dfs_cac_ms:(unsigned int){%u}}";
  info = kmalloc(2048, GFP_KERNEL);
  sprintf( info, layout, channel->band, channel->center_freq, channel->hw_value, channel->flags, channel->max_antenna_gain, channel->max_power, channel->max_reg_power, channel->beacon_found, channel->orig_flags, channel->orig_mag, channel->orig_mpwr, channel->dfs_state, channel->dfs_state_entered, channel->dfs_cac_ms);
  info = shrink( info );

  return info;
};

char *display_struct_cfg80211_chan_def( const struct cfg80211_chan_def *chandef) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * chandef_chan = NULL;
  
  if (chandef == NULL) {
    layout = "(struct cfg80211_chan_def){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  chandef_chan = display_struct_ieee80211_channel( chandef->chan );
  layout = "(struct cfg80211_chan_def){.chan:%s .width:(enum nl80211_chan_width){%08X} .center_freq1:(u32){%u} .center_freq2:(u32){%u}}";
  info = kmalloc(2048, GFP_KERNEL);
  sprintf( info, layout, chandef_chan, chandef->width, chandef->center_freq1, chandef->center_freq2);

  if (chandef_chan != NULL) kfree(chandef_chan);
  info = shrink( info );

  return info;
  
};

char *display_struct_ieee80211_chanctx_conf(const struct ieee80211_chanctx_conf* ctx) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * def = NULL;
  char * min_def = NULL;
  
  if (ctx == NULL) {
    layout = "(struct ieee80211_chanctx_conf){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  def = display_struct_cfg80211_chan_def( &ctx->def );
  min_def = display_struct_cfg80211_chan_def( &ctx->min_def );
  layout = "(struct ieee80211_chanctx_conf){.def:%s .min_def:%s .rx_chains_static:(u8){%u} .rx_chains_dynamic:(u8){%u} .radar_enabled:(bool){%u} .drv_priv:(u8[0]){%p}}";
  info = kmalloc(2048, GFP_KERNEL);
  sprintf( info, layout, def, min_def, ctx->rx_chains_static, ctx->rx_chains_dynamic, (unsigned int)ctx->radar_enabled, ctx->drv_priv);
  if (def != NULL) kfree(def);
  if (min_def != NULL) kfree(min_def);
  info = shrink( info );

  return info;
  
};

char *display_struct_ieee80211_tx_info_1_1( const struct ieee80211_tx_info * tx_info) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * vif = NULL;
  
  if (tx_info == NULL) {
    layout = "(struct ieee80211_tx_info){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  vif = display_struct_ieee80211_vif( tx_info->control.vif );
  layout = "(struct ieee80211_tx_info){.flags:(u32){%08X} .band:(u8){%u} .hw_queue:(u8){%u} .ack_frame_id:(u16){%u} .control:(struct){.jiffies:(unsigned long){%u} .*vif@%p:%s .*hw_key@%p:%s .flags:(u32){%08X}}}";
  info = kmalloc(2048, GFP_KERNEL);
  sprintf( info, layout, tx_info->flags, tx_info->band, tx_info->hw_queue, tx_info->ack_frame_id, tx_info->control.jiffies, tx_info->control.vif, vif, tx_info->control.hw_key, "??", tx_info->control.flags);
  
  if (vif != NULL) kfree(vif);
  info = shrink( info );

  return info;
};

char *display_struct_ieee80211_tx_control( const struct ieee80211_tx_control * txc) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * sta = NULL;
  
  if (txc == NULL) {
    layout = "(struct ieee80211_tx_control){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  sta = display_struct_ieee80211_sta( txc->sta );
  layout = "(struct ieee80211_tx_control){.sta:%s}";
  info = kmalloc(2048, GFP_KERNEL);
  sprintf( info, layout, sta);
  if (sta != NULL) kfree(sta);
  info = shrink( info );

  return info;
  
};

char *display_struct_ieee80211_conf( const struct ieee80211_conf *conf) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  //char * conf_flags = NULL;
  char * conf_chandef = NULL;
  //char * conf_radar_enabled = NULL;
  //char * conf_smps_mode = NULL;
  //char * temp = NULL;
  
  if (conf == NULL) {
    layout = "(struct ieee80211_conf){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }
  
  //conf_flags = "??"; //display((enum ieee80211_conf_flags)conf->flags);
  conf_chandef = display_struct_cfg80211_chan_def( &conf->chandef );
  //conf_radar_enabled = "??"; //display(conf->radar_enabled);
  //conf_smps_mode = "??"; //display((enum ieee80211_smps_mode)conf->smps_mode);
  layout = "(struct ieee80211_conf){.flags:(enum ieee80211_conf_flags|u32){%08X} .power_level:(int){%d} .dynamic_ps_timeout:(int){%d} .listen_interval:(u16){%u} .ps_dtim_period:(u8){%u} .long_frame_max_tx_count:(u8){%u} .short_frame_max_tx_count:(u8){%u} .chandef:%s .radar_enabled:(bool){%d} .smps_mode:(enum ieee80211_smps_mode){%08X}}";
  info = (char *)kmalloc(2048, GFP_KERNEL);

  sprintf( info, layout, conf->flags, conf->power_level, conf->dynamic_ps_timeout, /*conf->max_sleep_period,*/ conf->listen_interval, conf->ps_dtim_period, conf->long_frame_max_tx_count, conf->short_frame_max_tx_count, conf_chandef, (int)conf->radar_enabled, (unsigned int)conf->smps_mode);

  //if (conf_flags != NULL) kfree(conf_flags);
  if (conf_chandef != NULL) kfree(conf_chandef);
  //if (conf_radar_enabled != NULL) kfree(conf_radar_enabled);
  //if (conf_smps_mode != NULL) kfree(conf_smps_mode);
  info = shrink( info );
  
  return info;
};

char *display_struct_ieee80211_txrx_stypes( const struct ieee80211_txrx_stypes* stypes ) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;
  
  if (stypes == NULL) {
    layout = "(struct ieee80211_txrx_stypes){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  layout = "(struct ieee80211_txrx_stypes){.tx:(u16){%u} .rx:(u16){%u}}";
  size = strlen(layout)+2*5+1;
  info = kmalloc(size, GFP_KERNEL);
  snprintf( info, size, layout, stypes->tx, stypes->rx );
  return info;
  
};

//char *display_nl80211_iftype(enum nl80211_iftype * type) {
char *display_nl80211_iftype(const u16 * type) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * temp = NULL;
  unsigned int index = 0;
  unsigned int n_types = 0;
  int i = 0;
  
  if (type == NULL) {
    layout = "(enum nl80211_iftype){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  layout = "(enum nl80211_iftype){%s}";
  info = kmalloc(2048, GFP_KERNEL);
  temp = kmalloc(2048, GFP_KERNEL);
  temp[0] = '\0';
  index = 0;
  n_types = ARRAY_SIZE(nl80211_iftype_debug);
  
  for(i=0;i<n_types;i++)
    if ((long unsigned int)type & BIT(nl80211_iftype_debug[i].type))
      index += sprintf( &temp[index], " %s ", nl80211_iftype_debug[i].str);

  if (strlen(temp) == 0) sprintf( temp, " %s ", "(none)");
  sprintf( info, layout, temp);
  if (temp != NULL) kfree(temp);
  info = shrink( info );
  
  return info;

}

char *display_struct_ieee80211_sta_rates( const struct ieee80211_sta_rates * sta_rates) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * rate_temp = NULL;
  char * rate_layout = NULL;
  unsigned int index = 0;
  unsigned int i = 0;
  
  if (sta_rates == NULL) {
    layout = "(struct ieee80211_sta_rates){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  rate_layout = "[%u](struct){.idx:(s8){%d} .count:(u8){%u} .count_cts:(u8){%u} .count_rts:(u8){%u} .flags:(u16){%04X}}";
  rate_temp = kmalloc( 2048, GFP_KERNEL); rate_temp[0] = '\0';
  index = 0;
  for( i=0; i<IEEE80211_TX_RATE_TABLE_SIZE ; i++) {
    index += sprintf( &rate_temp[index], rate_layout, i, sta_rates->rate[i].idx, sta_rates->rate[i].count, sta_rates->rate[i].count_cts, sta_rates->rate[i].count_rts, sta_rates->rate[i].flags);

  }
  
  layout = "(struct ieee80211_sta_rates){.rcu_head:(struct rcu_head*){%p} .rate:%s}";
  info = kmalloc(2048, GFP_KERNEL);
  sprintf( info, layout, &sta_rates->rcu_head, rate_temp);

  if (rate_temp != NULL) kfree(rate_temp);
  info = shrink( info );
  
  return info;
  
};

char *display_struct_ieee80211_sta(const struct ieee80211_sta *sta) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * sta_rates = NULL;
  char * sta_addr = NULL;
  
  if (sta == NULL) {
    layout = "(struct ieee80211_sta){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  layout = "(struct ieee80211_sta){.supp_rates:(u32[]){[0]{%08X}[1]{%08X}[2]{%08X}} .addr:%s .aid:(u16){%u} .ht_cap:(ieee80211_sta_ht_cap){TODO} .vht_cap:(ieee80211_sta_vht_cap){TODO} .max_rx_aggregation_subframes:(u8){%u} .wme:(bool){%u} .uapsd_queues:(u8){%u} .max_sp:(u8){%u} .rx_nss:(u8){%u} .bandwidth:(enum ieee80211_sta_rx_bandwidth){%u} .smps_mode:(enum ieee80211_smps_mode){%u} .rates:%s .drv_priv:(u8[0]){%p}}";

  sta_addr = display_struct_mac_address( (struct mac_address*)sta->addr );
  sta_rates = display_struct_ieee80211_sta_rates( sta->rates);
  info = kmalloc(2048,GFP_KERNEL);
  sprintf( info, layout, sta->supp_rates[0], sta->supp_rates[1], sta->supp_rates[2], sta_addr, sta->aid, /* ht_cap, vht_cap, */ sta->max_rx_aggregation_subframes, sta->wme, sta->uapsd_queues, sta->max_sp, sta->rx_nss, sta->bandwidth, sta->smps_mode, sta_rates, sta->drv_priv);

  info = shrink(info);
  return info;
  
};

char *display_struct_ieee80211_iface_limit(const struct ieee80211_iface_limit* limit) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * temp = NULL;
  
  if (limit == NULL) {
    layout = "(struct ieee80211_iface_limit){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  temp = display_nl80211_iftype( &limit->types );
  layout = "(struct ieee80211_iface_limit){.max:(u16){%u} .types:%s}";
  info = kmalloc(2048, GFP_KERNEL);
  sprintf(info , layout, limit->max, temp );
  if (temp != NULL) kfree(temp);
  info = shrink( info );
  
  return info;
  
};

char *display_struct_ieee80211_iface_combination(const struct ieee80211_iface_combination* comb) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * comb_limits = NULL;
  char * comb_limits_temp = NULL;
  char * temp = NULL;

  unsigned int index = 0;
  unsigned int i = 0;
  
  if (comb == NULL) {
    layout = "(struct ieee80211_iface_combination){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  layout = "(struct ieee80211_iface_combination){.*limits:%s .num_different_channels:(u32){%u} .max_interfaces:(u16){%u} .n_limits:(u8){%u} .beacon_int_infra_match:(bool){%d} .radar_detect_widths:(u8){%u}}";

  comb_limits = kmalloc( 2048, GFP_KERNEL ); comb_limits[0] = '\0';
  index = 0;
  temp = "[%d]%s";
  for ( i=0 ; i<comb->n_limits ; i++) {
    comb_limits_temp = display_struct_ieee80211_iface_limit( &comb->limits[i] );
    size = strlen(comb_limits_temp)+strlen(temp)+0-2+1; // lets assume i<100
    index += snprintf( &comb_limits[index], size, temp, i, comb_limits_temp);
    if (comb_limits_temp != NULL) kfree(comb_limits_temp);
    if (index >= 2048) break;
  }

  info = (char *)kmalloc(2048, GFP_KERNEL);
  snprintf( info, 2048, layout, comb_limits, comb->num_different_channels, comb->max_interfaces, comb->n_limits, (char)comb->beacon_int_infra_match, comb->radar_detect_widths);
  info = shrink( info );
  
  return info;
}

char *display_struct_ieee80211_vif(const struct ieee80211_vif * vif ) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * type = NULL;
  char * bss = NULL;
  char * addr = NULL;
  char * ctx = NULL;
  struct ieee80211_chanctx_conf *chanctx_conf;
  
  if (vif == NULL) {
    layout = "(struct ieee80211_vif){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  type = display_nl80211_iftype( (u16*)&vif->type );
  bss = "??";//display_struct_ieee80211_bss_conf( &vif->bss_conf );
  addr = display_struct_mac_address((struct mac_address*)&vif->addr);
  chanctx_conf = rcu_dereference(vif->chanctx_conf);
  ctx = display_struct_ieee80211_chanctx_conf( chanctx_conf );

  layout = "(struct ieee80211_vif){.type:%s .bss_conf:%s .addr:%s .p2p:(bool){%u} .csa_active:(bool){%u} .cab_queue:(u8){%u} .hw_queue:(u8[4]){[0]{%u}[1]{%u}[2]{%u}[3]{%u}} .chanctx_conf:%s .driver_flags:(u32){%08X} .drv_priv:(u8[0]){%p}}";
  info = kmalloc(2048, GFP_KERNEL);
  snprintf( info, 2048, layout, type, bss, addr, vif->p2p, vif->csa_active, vif->cab_queue, vif->hw_queue[0], vif->hw_queue[1], vif->hw_queue[2], vif->hw_queue[3], ctx, vif->driver_flags, vif->drv_priv);

  if (type != NULL) kfree(type);
  //if (bss != NULL) kfree(bss);
  if (addr != NULL) kfree(addr);
  if (ctx != NULL) kfree(ctx);
  info = shrink( info );

  return info;
};

char *display_struct_wiphy( const struct wiphy * wiphy) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;
  
  char * wiphy_addresses = NULL;
  char * wiphy_perm_addr = NULL;
  char * wiphy_addr_mask = NULL;
  char * wiphy_mgmt_stypes = NULL;
  char * wiphy_iface_combinations_temp = NULL;
  char * wiphy_iface_combinations = NULL;
  char * temp = NULL;
  char * wiphy_software_iftypes = NULL;
  char * wiphy_interface_modes = NULL;
  char * wiphy_bands_2ghz = NULL;
  char * wiphy_bands_5ghz = NULL;
  char * wiphy_bands_60ghz = NULL;

  unsigned int index = 0;
  unsigned int i = 0;
  
  if (wiphy == NULL) {
    layout = "(struct wiphy){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }

  wiphy_perm_addr = display_struct_mac_address( (struct mac_address *)wiphy->perm_addr );
  wiphy_addr_mask = display_struct_mac_address( (struct mac_address *)wiphy->addr_mask );
  wiphy_addresses = display_struct_mac_address( (struct mac_address *)wiphy->addresses );
  //struct mac_address *addresses;

  //u16 n_addresses;

  wiphy_mgmt_stypes = display_struct_ieee80211_txrx_stypes( wiphy->mgmt_stypes );
  //const struct ieee80211_txrx_stypes *mgmt_stypes;

  wiphy_iface_combinations = kmalloc( 2048, GFP_KERNEL ); wiphy_iface_combinations[0] = '\0';
  index = 0;
  temp = "[%d]%s";
  for ( i=0 ; i<wiphy->n_iface_combinations ; i++) {
    wiphy_iface_combinations_temp = display_struct_ieee80211_iface_combination( &wiphy->iface_combinations[i] );

    //printk("\n\n%s\n\n", wiphy_iface_combinations_temp);
    
    size = strlen(wiphy_iface_combinations_temp)+strlen(temp)+0-2+1; // lets assume i<100
    index += snprintf( &wiphy_iface_combinations[index], size, temp, i, wiphy_iface_combinations_temp);

    //printk("\n\n%s\n\n", wiphy_iface_combinations);
    
    if (wiphy_iface_combinations_temp != NULL) kfree(wiphy_iface_combinations_temp);
    if (index >= 2048) break;
  }
  //temp = shrink( wiphy_iface_combinations );
  //if (temp!=NULL) wiphy_iface_combinations = temp;
  //const struct ieee80211_iface_combination *iface_combinations;

  //int n_iface_combinations;

  wiphy_software_iftypes = display_nl80211_iftype( &wiphy->software_iftypes );
  //u16 software_iftypes;

  //u16 n_addresses;

  wiphy_interface_modes = display_nl80211_iftype( &wiphy->interface_modes );
  /* Supported interface modes, OR together BIT(NL80211_IFTYPE_...) */
  //u16 interface_modes;

  //u32 available_antennas_tx;
  //u32 available_antennas_rx;

  wiphy_bands_2ghz = "??";//display_struct_ieee80211_supported_band( &wiphy->bands[IEEE80211_BAND_2GHZ] );
  wiphy_bands_5ghz = "??";//display_struct_ieee80211_supported_band( &wiphy->bands[IEEE80211_BAND_5GHZ] );
  wiphy_bands_60ghz = "??";//display_struct_ieee80211_supported_band( &wiphy->bands[IEEE80211_BAND_60GHZ] );
  //struct ieee80211_supported_band *bands[IEEE80211_NUM_BANDS];

  /* dir in debugfs: ieee80211/<wiphyname> */
  //struct dentry *debugfsdir;

  //const struct wiphy_vendor_command *vendor_commands;
  //const struct nl80211_vendor_cmd_info *vendor_events;
  //int n_vendor_commands, n_vendor_events;

  layout = "(struct wiphy){.perm_addr:%s .addr_mask:%s .addresses:%s .*mgmt_stypes:%s .*iface_combinations:%s .n_iface_combinations:(int){%d} .software_iftypes:%s .n_addresses:(u16){%u} .interface_modes:%s .flags:(u32){%08X} .regulatory_flags:(u32){%08X} .features:(u32){%08X} .signal_type:(enum cfg80211_signal_type){%u} .available_antennas_tx:(u32){%u} .available_antennas_rx:(u32){%u} .privid:(const void*){%p} .bands:(ieee80211_supported_band[3]){[2Ghz]:%s [5Ghz]:%s [60Ghz]:%s} .priv:(char[0]){%p}}";

  info = (char *)kmalloc(2048, GFP_KERNEL);
  sprintf( info, layout, wiphy_perm_addr, wiphy_addr_mask, wiphy_addresses, wiphy_mgmt_stypes, wiphy_iface_combinations, wiphy->n_iface_combinations, wiphy_software_iftypes, wiphy->n_addresses, wiphy_interface_modes, wiphy->flags, wiphy->regulatory_flags, wiphy->features, wiphy->signal_type, wiphy->available_antennas_tx, wiphy->available_antennas_rx, wiphy->privid, wiphy_bands_2ghz, wiphy_bands_5ghz, wiphy_bands_60ghz, wiphy->priv);

  if (wiphy_addresses != NULL) kfree(wiphy_addresses);
  if (wiphy_perm_addr != NULL) kfree(wiphy_perm_addr);
  if (wiphy_addr_mask != NULL) kfree(wiphy_addr_mask);
  if (wiphy_mgmt_stypes != NULL) kfree(wiphy_mgmt_stypes);
  if (wiphy_iface_combinations != NULL) kfree(wiphy_iface_combinations);
  if (wiphy_software_iftypes != NULL) kfree(wiphy_software_iftypes);
  if (wiphy_interface_modes != NULL) kfree(wiphy_interface_modes);
  //if (wiphy_bands_2ghz != NULL) kfree(wiphy_bands_2ghz);
  //if (wiphy_bands_5ghz != NULL) kfree(wiphy_bands_5ghz);
  //if (wiphy_bands_60ghz != NULL) kfree(wiphy_bands_60ghz);

  //printk("\n\n");pr_info("%s", info);printk("\n\n");

  info = shrink( info );

  //printk("\n\nlen:%d => %s\n\n", strlen(info),info);
  
  return info;
};

char *display_struct_ieee80211_hw( const struct ieee80211_hw *hw) {

  char * layout = NULL;
  size_t size = 0;
  char * info = NULL;

  char * hwconf = NULL;
  char * hwwiphy = NULL; //display(hw->wiphy);
  char * hwnetdev_features = NULL; //display(&hw->netdev_features);
  char * hwcipher_schemes = NULL; //display(hw->cipher_schemes);
  
  if (hw == NULL) {
    layout = "(struct ieee80211_hw){null}";
    size = strlen(layout)+1;
    info = kmalloc(size, GFP_KERNEL);
    snprintf( info, size, layout );
    return info;
  }
  
  hwconf = display_struct_ieee80211_conf(&hw->conf);
  hwwiphy = display_struct_wiphy(hw->wiphy);
  hwnetdev_features = "??"; //display(&hw->netdev_features);
  hwcipher_schemes = "??"; //display(hw->cipher_schemes);
  
  layout = "(struct ieee80211_hw){.conf:%s .*wiphy:%s .const* rate_control_algorithm:(char){%p} .priv:(void*){%p} .flags:(u32){%08X} .extra_tx_headroom:(unsigned int){%u} .extra_beacon_tailroom:(unsigned int){%u} .vif_data_size:(int){%d} .sta_data_size:(int){%d} .chanctx_data_size:(int){%d} .queues:(u16){%u} .max_listen_interval:(u16){%u} .max_signal:(s8){%d} .max_rates:(u8){%u} .max_report_rates:(u8){%u} .max_rate_tries:(u8){%u} .max_rx_aggregation_subframes:(u8){%u} .max_tx_aggregation_subframes:(u8){%u} .offchannel_tx_hw_queue:(u8){%u} .radiotap_mcs_details:(u8){%u} .radiotap_vht_details:(u16){%u} .netdev_features:%s .uapsd_queues:(u8){%u} .uapsd_max_sp_len:(u8){%u} .n_cipher_schemes:(u8){%u} .const* cipher_schemes:%s}";

  info = (char *)kmalloc(4096, GFP_KERNEL);
  
  snprintf( info, 4096, layout, hwconf, hwwiphy, hw->rate_control_algorithm, hw->priv, hw->flags, hw->extra_tx_headroom, hw->extra_beacon_tailroom, hw->vif_data_size, hw->sta_data_size, hw->chanctx_data_size, hw->queues, hw->max_listen_interval, hw->max_signal, hw->max_rates, hw->max_report_rates, hw->max_rate_tries, hw->max_rx_aggregation_subframes, hw->max_tx_aggregation_subframes, hw->offchannel_tx_hw_queue, hw->radiotap_mcs_details, hw->radiotap_vht_details, hwnetdev_features, hw->uapsd_queues, hw->uapsd_max_sp_len, hw->n_cipher_schemes, hwcipher_schemes);

  if (hwconf != NULL) kfree(hwconf);
  if (hwwiphy != NULL) kfree(hwwiphy);
  //if (hwnetdev_features != NULL) kfree(hwnetdev_features);
  //if (hwcipher_schemes != NULL) kfree(hwcipher_schemes);
  info = shrink( info );
  
  return info;
  
};

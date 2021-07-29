#include "mac80211-ops.h"
#include "intech/llc/llc.h"

#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "debug.h"
#include "debug_ieee80211.h"
#include "radio.h"
#include "util.h"

// As mandatory to use "radio.h" MACROs
#define pRadio(...) ((struct radio*)((((struct radio_priv*)(hw->priv)))->pThisRadio))

/// The dev's work descriptor
typedef struct devWork
{
  /// Overaching radio handle
  struct radio *pRadio;
  /// The buffer to 'transmit'
  struct sk_buff *pSkb;
  /// The kernel work structure
  struct work_struct Work;
} tDevWork;

/////////////////////////////////////////////////////////////////////////
// IEEE80211 MAC interface. Functions and callbacks
/////////////////////////////////////////////////////////////////////////

void WCH11P_TxWorkCallback(struct work_struct *pWork) {

  int Res = -ENOSYS;
  struct devWork *pDevWork = NULL;
  struct radio *pRadio = NULL;
  struct sk_buff *pSkb = NULL;
  tMKxTxPacket *pTxPkt = NULL;
  struct ieee80211_tx_info *txi = NULL;
  struct ieee80211_rate *pRate = NULL;

  pDevWork = container_of(pWork, struct devWork, Work);
  GOTO_ASSERT( (pDevWork != NULL), , Error);
  pRadio = pDevWork->pRadio;
  pSkb = pDevWork->pSkb;
  GOTO_ASSERT( ((pRadio != NULL) && (pSkb != NULL)), , Error);
  skb_orphan(pSkb);

  if (adaptBSSID) 
    memset(&pSkb->data[16], 0xFF , 1);

  if (pSkb != NULL) txi = IEEE80211_SKB_CB(pSkb);
  pRate = ieee80211_get_tx_rate( pRadio->pMacDev, txi);
  
  // Push reserved headroom & build MKxIF header
  pTxPkt = (tMKxTxPacket *)skb_push( pSkb, sizeof(tMKxTxPacket));
  GOTO_ASSERT(pTxPkt != NULL, , Error);

  memset(pTxPkt, 0, sizeof(tMKxTxPacket));
  *pTxPkt = (tMKxTxPacket){
    .Hdr = {
      .Type = OBUIF_TXPACKET,
      .Len = pSkb->len,
      .Seq = 44,
      .Ret = 0,      
    },
    .TxPacketData = {
      .RadioID = pRadio->ID,
      .ChannelID = OBU_CHANNEL_0,
      .TxAntenna = OBU_ANT_DEFAULT,
      .MCS = pRate->hw_value,
      .TxPower = OBU_POWER_TX_DEFAULT,
      .Expiry = 0ULL,
      .TxFrameLength = (pSkb->len - sizeof(tMKxTxPacket))
    }
  };

  // Call MKx handler
  Res = (*(pRadio->ppMKx))->API.Functions.TxReq((*(pRadio->ppMKx)), pTxPkt, pSkb);
  GOTO_ASSERT(Res == OBUSTATUS_SUCCESS, , Error);

  goto Exit;
  
 Error:
  dev_kfree_skb(pSkb);
 Exit:
  kfree(pDevWork);
  // pSkb freed in TxCnf() callback

  return;
}

void WCH11P_tx(struct ieee80211_hw *hw, struct ieee80211_tx_control *control, struct sk_buff *pSkb){
  
  int Res = NETDEV_TX_OK;
  struct ieee80211_tx_info *txi = NULL;

  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);

  d_create(hw);
  d_create(control);
  d_create(txi);

  if (pSkb != NULL) txi = IEEE80211_SKB_CB(pSkb);
  d_stringify(struct_ieee80211_hw, hw);
  d_stringify(struct_ieee80211_tx_control, control);
  d_stringify(struct_ieee80211_tx_info_1_1, txi);

  //spin_lock_irqsave(SLOCK, SLOCK_FLAGS);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print(" *control@%p:", control); d_dump(control); d_print("\n");
  d_print(" *txi@%p:", txi); d_dump(txi); d_print("\n");
  d_print(" *pSkb@%p:(struct sk_buff){TODO}", pSkb); d_print("\n");
  //spin_unlock_irqrestore(SLOCK, SLOCK_FLAGS);  
  
  d_destroy(hw);
  d_destroy(control);
  d_destroy(txi);
  
  chanctx_conf = rcu_dereference(txi->control.vif->chanctx_conf);
  if (chanctx_conf)
    channel = chanctx_conf->def.chan;
  else {
    channel = NULL;
    dev_kfree_skb(pSkb);
    return;
  }
  
  // We are on interrupt context and TxReq might sleep,
  // so we do the rest in the kernel thread
  {
    struct devWork *pDevWork = NULL;
    GOTO_ASSERT( (pDevWork = kzalloc(sizeof(struct devWork), gfp_any())) != NULL, Res = NETDEV_TX_BUSY, Error);
    pDevWork->pRadio = pRadio();
    pDevWork->pSkb = pSkb;

    INIT_WORK(&(pDevWork->Work), WCH11P_TxWorkCallback);
    GOTO_ASSERT( ((Res = schedule_work(&(pDevWork->Work))) == 1), , Error);
  }

  goto Exit;
  
 Error:
  dev_kfree_skb(pSkb);
  
 Exit:
  
  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);

};

int WCH11P_start(struct ieee80211_hw *hw){

  tMKxRadioConfig Cfg;
  tMKxRadioConfigData *pCfg = &(Cfg.RadioConfigData);
  
  d_create(hw);
  d_stringify(struct_ieee80211_hw, hw);

  //spin_lock_irqsave(SLOCK, SLOCK_FLAGS);
  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);
  //spin_unlock_irqrestore(SLOCK, SLOCK_FLAGS);
  
  // Setup radio as {A|B}0 on frequency <default>
  d_print("Starting radio %d\n", pRadio()->ID);
  
  memcpy(pCfg, &pobu()->Config.Radio[pRadio()->ID], sizeof(*pCfg));
  pCfg->Mode = OBU_MODE_CHANNEL_0;
  pCfg->ChanConfig[OBU_CHANNEL_0].PHY.Bandwidth = OBUBW_10MHz;
  pCfg->ChanConfig[OBU_CHANNEL_0].PHY.ChannelFreq = pRadio()->ChanFreq;
  pCfg->ChanConfig[OBU_CHANNEL_0].PHY.TxAntenna = pRadio()->TxAntenna;
  pCfg->ChanConfig[OBU_CHANNEL_0].PHY.RxAntenna = pRadio()->RxAntenna;
  pCfg->ChanConfig[OBU_CHANNEL_0].PHY.DefaultMCS = OBUMCS_R12QPSK;
  //pCfg->ChanConfig[OBU_CHANNEL_0].PHY.DefaultMCS = OBUMCS_DEFAULT;
  pCfg->ChanConfig[OBU_CHANNEL_0].PHY.DefaultTxPower = 20.0 * 0.5;
  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.DualTxControl = OBU_TXC_TXRX;

  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[0].Addr = 0XFFFFFFFFFFFFULL;
  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[0].Mask = 0XFFFFFFFFFFFFULL;
  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[0].MatchCtrl = OBU_ADDRMATCH_ENABLE_CTRL;

  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[1].Addr =
    ((uint64_t)pRadio()->def_addr[0]) &
    ((uint64_t)pRadio()->def_addr[1] << 8) &
    ((uint64_t)pRadio()->def_addr[2] << 16) &
    ((uint64_t)pRadio()->def_addr[3] << 24) &
    ((uint64_t)pRadio()->def_addr[4] << 32) &
    ((uint64_t)pRadio()->def_addr[5] << 40);
  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[1].Mask = 0XFFFFFFFFFFFFULL;
  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[1].MatchCtrl = OBU_ADDRMATCH_RESPONSE_ENABLE | OBU_ADDRMATCH_ENABLE_CTRL;

  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[2].Addr = 0XF21234567890ULL;
  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[2].Mask = 0XFFFFFFFFFFFFULL;
  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[2].MatchCtrl = OBU_ADDRMATCH_ENABLE_CTRL;

  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[3].Addr = 0XF21234567899ULL;
  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[3].Mask = 0XFFFFFFFFFFFFULL;
  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.AMSTable[3].MatchCtrl = OBU_ADDRMATCH_ENABLE_CTRL;

  pCfg->ChanConfig[OBU_CHANNEL_0].MAC.MaxRetries = 4;
  pCfg->ChanConfig[OBU_CHANNEL_0].LLC.IntervalDuration = 50 * 1000; // 50 ms
  pCfg->ChanConfig[OBU_CHANNEL_0].LLC.GuardDuration = 0;
  
  result() = pobu()->API.Functions.Config(pobu(), pRadio()->ID, &Cfg);

  d_destroy(hw);

  return (result());
  
};

void WCH11P_stop(struct ieee80211_hw *hw){
  
  tMKxRadioConfig Cfg;
  tMKxRadioConfigData *pCfg = &(Cfg.RadioConfigData);

  d_create(hw);
  d_stringify(struct_ieee80211_hw, hw);

  //spin_lock_irqsave(SLOCK, SLOCK_FLAGS);
  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");

  // Disable radio
  memcpy(pCfg, &pobu()->Config.Radio[pRadio()->ID], sizeof(*pCfg));
  pCfg->Mode = OBU_MODE_OFF;
  result() = pobu()->API.Functions.Config(pobu(), pRadio()->ID, &Cfg);

  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);
  //spin_unlock_irqrestore(SLOCK, SLOCK_FLAGS);
  
  d_destroy(hw);
  
};

int WCH11P_add_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif){
  
  d_create(hw);
  d_create(vif);
  d_stringify(struct_ieee80211_hw, hw);
  d_stringify(struct_ieee80211_vif, vif);

  //spin_lock_irqsave(SLOCK, SLOCK_FLAGS);
  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print(" *vif@%p:", vif); d_dump(vif); d_print("\n");
  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);
  //spin_unlock_irqrestore(SLOCK, SLOCK_FLAGS);
  
  vif->cab_queue = 0;
  vif->hw_queue[IEEE80211_AC_VO] = 0;
  vif->hw_queue[IEEE80211_AC_VI] = 1;
  vif->hw_queue[IEEE80211_AC_BE] = 2;
  vif->hw_queue[IEEE80211_AC_BK] = 3;
  
  d_destroy(hw);
  d_destroy(vif);
 
  return 0;
  
};

void WCH11P_remove_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif){
  
  d_create(hw);
  d_create(vif);

  d_stringify(struct_ieee80211_hw, hw);
  d_stringify(struct_ieee80211_vif, vif);

  //spin_lock_irqsave(SLOCK, SLOCK_FLAGS);
  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print(" *vif@%p:", vif); d_dump(vif); d_print("\n");
  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);
  //spin_unlock_irqrestore(SLOCK, SLOCK_FLAGS);
  
  d_destroy(hw);
  d_destroy(vif);

};

static int WCH11P_get_stats(struct ieee80211_hw *hw,
			     struct ieee80211_low_level_stats *stats)
{

  *stats = (struct ieee80211_low_level_stats){
    .dot11ACKFailureCount = pobu()->State.Stats.RadioStatsData.Chan[OBU_CHANNEL_0].Blablabla;
    .dot11RTSFailureCount = 0;
    .dot11FCSErrorCount = 0;
    .dot11RTSSuccessCount = 0;
  };
  
  return 0;
}

int WCH11P_set_antenna(struct ieee80211_hw *hw, u32 tx_ant, u32 rx_ant) {

  tMKxRadioConfig Cfg;
  tMKxRadioConfigData *pCfg = &(Cfg.RadioConfigData);

  d_print("txa %08X rxa %08X available %d cond %08X\n", tx_ant, rx_ant, *(pRadio()->AvailableAntennas), ((tx_ant|rx_ant) & (*(pRadio()->AvailableAntennas))));

  // Table for final AvailableAtennas value given Requested antennas (tx_ant|rx_ant)... not implemented
  //                Available
  //               00 01 10 11
  // Requested  00 gg g1 1g 11
  //            01 XX g0 XX 10
  //            10 XX XX 0g 01
  //            11 XX XX XX 00
  //
  // g = give back the antenna
  // X = not possible
  // 0 = take antenna
  // 1 = set antenna

  memcpy(pCfg, &pobu()->Config.Radio[pRadio()->ID], sizeof(*pCfg));
  pCfg->ChanConfig[OBU_CHANNEL_0].PHY.TxAntenna = tx_ant;
  pCfg->ChanConfig[OBU_CHANNEL_0].PHY.RxAntenna = rx_ant;
  TEST_ASSERT( ((result() = pobu()->API.Functions.Config( pobu(), pRadio()->ID, &Cfg)) == OBUSTATUS_SUCCESS), );
  //pRadio()->pMacDev->wiphy->available_antennas_tx = *(pRadio()->AvailableAntennas);
  //pRadio()->pMacDev->wiphy->available_antennas_rx = *(pRadio()->AvailableAntennas);

  pRadio()->TxAntenna = pobu()->Config.Radio[pRadio()->ID].ChanConfig[OBU_CHANNEL_0].PHY.TxAntenna;
  pRadio()->RxAntenna = pobu()->Config.Radio[pRadio()->ID].ChanConfig[OBU_CHANNEL_0].PHY.RxAntenna;
    
  //Exit:
  return result();
  
};

int WCH11P_get_antenna(struct ieee80211_hw *hw, u32 *tx_ant, u32 *rx_ant) {

  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);

  *tx_ant = pRadio()->TxAntenna;
  *rx_ant = pRadio()->RxAntenna;

  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);
  return result()=OBUSTATUS_SUCCESS;
  
};

int WCH11P_get_survey(struct ieee80211_hw *hw, int idx, struct survey_info *survey) {

/**
 * enum survey_info_flags - survey information flags
 *
 * @SURVEY_INFO_NOISE_DBM: noise (in dBm) was filled in
 * @SURVEY_INFO_IN_USE: channel is currently being used
 * @SURVEY_INFO_TIME: active time (in ms) was filled in
 * @SURVEY_INFO_TIME_BUSY: busy time was filled in
 * @SURVEY_INFO_TIME_EXT_BUSY: extension channel busy time was filled in
 * @SURVEY_INFO_TIME_RX: receive time was filled in
 * @SURVEY_INFO_TIME_TX: transmit time was filled in
 * @SURVEY_INFO_TIME_SCAN: scan time was filled in
 *
 * Used by the driver to indicate which info in &struct survey_info
 * it has filled in during the get_survey().
 *
 * struct survey_info - channel survey response
 *
 * @channel: the channel this survey record reports, may be %NULL for a single
 *	record to report global statistics
 * @filled: bitflag of flags from &enum survey_info_flags
 * @noise: channel noise in dBm. This and all following fields are
 *	optional
 * @time: amount of time in ms the radio was turn on (on the channel)
 * @time_busy: amount of time the primary channel was sensed busy
 * @time_ext_busy: amount of time the extension channel was sensed busy
 * @time_rx: amount of time the radio spent receiving data
 * @time_tx: amount of time the radio spent transmitting data
 * @time_scan: amount of time the radio spent for scanning
 *
 * Used by dump_survey() to report back per-channel survey information.
 *
 * This structure can later be expanded with things like
 * channel duty cycle etc.
 */

  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);

  *survey = (struct survey_info) {
   .channel = hw->conf.chandef.chan,
   .time_busy = pobu()->State.Stats[pRadio()->ID].RadioStatsData.Chan[OBU_CHANNEL_0].ChannelBusyRatio,
   .noise = pobu()->State.Stats[pRadio()->ID].RadioStatsData.Chan[OBU_CHANNEL_0].AverageIdlePower,
   .filled = SURVEY_INFO_NOISE_DBM | SURVEY_INFO_TIME_BUSY
 };
 
  return result()=OBUSTATUS_SUCCESS;

};

void WCH11P_bss_info_changed(struct ieee80211_hw *hw,
			 struct ieee80211_vif *vif,
			 struct ieee80211_bss_conf *info,
			 u32 changed) {

  tMKxRadioConfig Cfg;
  tMKxRadioConfigData *pCfg = &(Cfg.RadioConfigData);

  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);
  d_print(" Changed %08X\n", changed);

  if ((changed & BSS_CHANGED_TXPOWER) && ( (pRadio()->txPower) != DBM_TO_HALFDBM(info->txpower))) {
    pRadio()->txPower = DBM_TO_HALFDBM(info->txpower);
    d_print(" Changing TxPower to %d hdBm (%d dBm)\n", pRadio()->txPower, info->txpower);
    memcpy(pCfg, &pobu()->Config.Radio[pRadio()->ID], sizeof(*pCfg));
    pCfg->ChanConfig[OBU_CHANNEL_0].PHY.DefaultTxPower = pRadio()->txPower;
    result() = pobu()->API.Functions.Config(pobu(), pRadio()->ID, &Cfg);
  }

  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);

};


int WCH11P_config(struct ieee80211_hw *hw, u32 changed){

  tMKxRadioConfig Cfg;
  tMKxRadioConfigData *pCfg = &(Cfg.RadioConfigData);
  struct ieee80211_conf *conf = &hw->conf;
  bool dochange = false;
  
  d_create(hw);
  d_stringify(struct_ieee80211_hw, hw);

  //spin_lock_irqsave(SLOCK, SLOCK_FLAGS);
  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print(" changed:(u32){%08X}\n", changed);
  //spin_unlock_irqrestore(SLOCK, SLOCK_FLAGS);

  if (changed & IEEE80211_CONF_CHANGE_CHANNEL) {
    pRadio()->ChanFreq = conf->chandef.chan->center_freq;
    d_print(" Changing Channel to %d MHz\n", pRadio()->ChanFreq);
    dochange = true;
  }
    
  if (changed & IEEE80211_CONF_CHANGE_POWER) {
    pRadio()->txPower = DBM_TO_HALFDBM(conf->power_level);
    d_print(" Changing TxPower to %d hdBm (%d dBm)\n", pRadio()->txPower, conf->power_level);
    dochange = true;
  }
  
  if (dochange) {
    memcpy(pCfg, &pobu()->Config.Radio[pRadio()->ID], sizeof(*pCfg));
    pCfg->ChanConfig[OBU_CHANNEL_0].PHY.ChannelFreq = pRadio()->ChanFreq;
    pCfg->ChanConfig[OBU_CHANNEL_0].PHY.DefaultTxPower = pRadio()->txPower;
    result() = pobu()->API.Functions.Config(pobu(), pRadio()->ID, &Cfg);
  }
  
  d_destroy(hw);

  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);

  return result();
  
};

void WCH11P_configure_filter(struct ieee80211_hw *hw,
			     unsigned int changed_flags,
			     unsigned int *total_flags,
			     u64 multicast){
  
  unsigned int new_flags = 0;
  d_create(hw);
  d_stringify(struct_ieee80211_hw, hw);

  //spin_lock_irqsave(SLOCK, SLOCK_FLAGS);
  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print(" changed_flags:(u32){%08X}\n", changed_flags);
  d_print(" total_flags@%p:(u32){%08X}\n", total_flags, (total_flags!=NULL)? *total_flags : 0xDEADBEEF);
  d_print(" multicast:(u64){%08X %08X}\n", (((unsigned int*)&multicast)[0]), (((unsigned int*)&multicast)[1]));*/

  if (total_flags != NULL) {
    /*if (*total_flags & FIF_PROMISC_IN_BSS) {
      new_flags |= FIF_PROMISC_IN_BSS;
    } else */
    if (*total_flags & FIF_ALLMULTI || multicast == ~(0ULL)) {
      new_flags |= FIF_ALLMULTI;
    } else {
    }
    *total_flags =  new_flags;
  } else {
    //*total_flags = 0x0;
    new_flags = 0x0;
  }
  d_print("total flags %08X\n", *total_flags);
  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);
  //spin_unlock_irqrestore(SLOCK, SLOCK_FLAGS);

  d_destroy(hw);
  
};

int WCH11P_add_chanctx(struct ieee80211_hw *hw, struct ieee80211_chanctx_conf *ctx) {

  d_create(hw);
  d_create(ctx);

  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);
  d_stringify(struct_ieee80211_hw, hw);
  d_stringify(struct_ieee80211_chanctx_conf, ctx);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print(" *ctx@%p:", ctx); d_dump(ctx); d_print("\n");

  d_destroy(hw);
  d_destroy(ctx);

  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);

  return 0;
};

void WCH11P_remove_chanctx(struct ieee80211_hw *hw, struct ieee80211_chanctx_conf *ctx) {

  d_create(hw);
  d_create(ctx);

  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);

  d_stringify(struct_ieee80211_hw, hw);
  d_stringify(struct_ieee80211_chanctx_conf, ctx);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print(" *ctx@%p:", ctx); d_dump(ctx); d_print("\n");

  d_destroy(hw);
  d_destroy(ctx);

  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);

};
void WCH11P_change_chanctx(struct ieee80211_hw *hw, struct ieee80211_chanctx_conf *ctx, u32 changed) {

  d_create(hw);
  d_create(ctx);

  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);

  d_stringify(struct_ieee80211_hw, hw);
  d_stringify(struct_ieee80211_chanctx_conf, ctx);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print(" *ctx@%p:", ctx); d_dump(ctx); d_print("\n");

  d_destroy(hw);
  d_destroy(ctx);

  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);

};
int WCH11P_assign_vif_chanctx(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_chanctx_conf *ctx) {

  d_create(hw);
  d_create(vif);
  d_create(ctx);

  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);

  d_stringify(struct_ieee80211_hw, hw);
  d_stringify(struct_ieee80211_vif, vif);
  d_stringify(struct_ieee80211_chanctx_conf, ctx);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print(" *vif@%p:", vif); d_dump(vif); d_print("\n");
  d_print(" *ctx@%p:", ctx); d_dump(ctx); d_print("\n");

  d_destroy(hw);
  d_destroy(vif);
  d_destroy(ctx);

  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);

  return 0;
};
void WCH11P_unassign_vif_chanctx(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_chanctx_conf *ctx) {

  d_create(hw);
  d_create(vif);
  d_create(ctx);

  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);

  d_stringify(struct_ieee80211_hw, hw);
  d_stringify(struct_ieee80211_vif, vif);
  d_stringify(struct_ieee80211_chanctx_conf, ctx);
  d_print("Function Parameters:\n");
  d_print(" *hw@%p:", hw); d_dump(hw); d_print("\n");
  d_print(" *vif@%p:", vif); d_dump(vif); d_print("\n");
  d_print(" *ctx@%p:", ctx); d_dump(ctx); d_print("\n");

  d_destroy(hw);
  d_destroy(vif);
  d_destroy(ctx);

  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);

};

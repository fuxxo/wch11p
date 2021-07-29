#include <linux/module.h>
#include <linux/init.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/delay.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "wireless-stack.h"
#include "intech/llc/llc.h"
#include "mac80211-ops.h"
#include "wch11p-ops.h"
#include "debug.h"
#include "debug_ieee80211.h"
#include "radio.h"
#include "util.h"

MODULE_AUTHOR("Antonio Giraldez Camano");
MODULE_DESCRIPTION("NXP WCH11P to wlan");
MODULE_SUPPORTED_DEVICE("WCH11P");
MODULE_LICENSE("GPL");

static unsigned int chanFreq = 5890;
module_param(chanFreq, uint, 0);
MODULE_PARM_DESC(chanFreq, "Frequency to tune the radio to");

static unsigned int antenna = 3;
module_param(antenna, uint, 0);
MODULE_PARM_DESC(antenna, "Antenna for Tx/Rx [0:dummy, 1:antenna1, 2:antenna2, 3:both antennas(diversity)]");

// Untill LinuxKernel implements OBS mode for 802.11p
static bool adaptBSSID = false;
module_param(adaptBSSID, bool, false);
MODULE_PARM_DESC(adaptBSSID, "Translate first byte of BSSID between 0xF2 and 0xFF and back");

//------------------------------------------------------------------------------
// Variables and private data
//------------------------------------------------------------------------------
struct WCH11P_hw HW = {

    .status = 0,
    .pMKx = NULL,
    .AvailableAntennas = OBU_ANT_1AND2,
    .radio =
        (struct radio[]){
// Per radio access/management pointer, as mandatory by "radio.h" to use its
// MACROs
#define pRadio(rd) ((struct radio *)&(HW.radio[rd]))

            [OBU_RADIO_A] = {.status = 0,
                             .ID = OBU_RADIO_A,
                             .ppMKx = &HW.pMKx,
                             .pMac80211_ops = &HW.mac80211_ops,
                             .pBands = HW.bands,
                             .ChanFreq = 5860,
                             .TxAntenna = OBU_ANT_NONE,
                             .RxAntenna = OBU_ANT_NONE,
                             .AvailableAntennas = &HW.AvailableAntennas,
                             .txPower = OBU_POWER_TX_DEFAULT,
                             .pMacDev = NULL,
                             .def_addr = {0x02, 0xE5, 0x48, 0xC7, 0xA6, 0x0A}},
            [OBU_RADIO_B] = {.status = 0,
                             .ID = OBU_RADIO_B,
                             .ppMKx = &HW.pMKx,
                             .pMac80211_ops = &HW.mac80211_ops,
                             .ChanFreq = 5870,
                             .TxAntenna = OBU_ANT_NONE,
                             .RxAntenna = OBU_ANT_NONE,
                             .AvailableAntennas = &HW.AvailableAntennas,
                             .txPower = OBU_POWER_TX_DEFAULT,
                             .pBands = HW.bands,
                             .pMacDev = NULL,
                             .def_addr = {0x02, 0xE5, 0x48, 0xC7, 0xA6, 0x0B}}},
    .mac80211_ops = {.tx = WCH11P_tx,
                     .start = WCH11P_start,
                     .stop = WCH11P_stop,
                     .add_interface = WCH11P_add_interface,
                     .remove_interface = WCH11P_remove_interface,
                     .get_antenna = WCH11P_get_antenna,
                     .set_antenna = WCH11P_set_antenna,
                     .bss_info_changed = WCH11P_bss_info_changed,
                     .config = WCH11P_config,
                     .configure_filter = WCH11P_configure_filter},
    .bands = {

      [ IEEE80211_BAND_2GHZ ] = {
      .channels = (struct ieee80211_channel []){
	CHAN2G(2412), /* Channel 1 */
	CHAN2G(2417), /* Channel 2 */
	CHAN2G(2422), /* Channel 3 */
	CHAN2G(2427), /* Channel 4 */
	CHAN2G(2432), /* Channel 5 */
	CHAN2G(2437), /* Channel 6 */
	CHAN2G(2442), /* Channel 7 */
	CHAN2G(2447), /* Channel 8 */
	CHAN2G(2452), /* Channel 9 */
	CHAN2G(2457), /* Channel 10 */
	CHAN2G(2462), /* Channel 11 */
	CHAN2G(2467), /* Channel 12 */
	CHAN2G(2472), /* Channel 13 */
	CHAN2G(2484)  /* Channel 14 */ 
      },
      .n_channels = 14,
      .bitrates = (struct ieee80211_rate []){
	{ .bitrate = 10 },
	{ .bitrate = 20, .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 55, .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 60 },
	{ .bitrate = 90 },
	{ .bitrate = 110, .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 120 },
	{ .bitrate = 180 },
	{ .bitrate = 240 },
	{ .bitrate = 360 },
	{ .bitrate = 480 },
	{ .bitrate = 540 }
      },
      .n_bitrates = 12
    },

    [ IEEE80211_BAND_5GHZ ] = {
      .channels = (struct ieee80211_channel []){
	CHAN5G(5860), /* Channel 172 */
	CHAN5G(5870), /* Channel 174 */
	CHAN5G(5875), /* Channel 175 */
	CHAN5G(5880), /* Channel 176 */
	CHAN5G(5890), /* Channel 178 */
	CHAN5G(5900), /* Channel 180 */
	CHAN5G(5905), /* Channel 181 */
	CHAN5G(5910), /* Channel 182 */
	CHAN5G(5920), /* Channel 184 */
      },
      .n_channels = 9,
      .bitrates = (struct ieee80211_rate []){
	{ .bitrate = 30, .hw_value = OBUMCS_R12BPSK, .flags = IEEE80211_RATE_SUPPORTS_10MHZ },
	{ .bitrate = 45, .hw_value = OBUMCS_R34BPSK },
	{ .bitrate = 60, .hw_value = OBUMCS_R12QPSK },
	{ .bitrate = 90, .hw_value = OBUMCS_R34QPSK },
	{ .bitrate = 120, .hw_value = OBUMCS_R12QAM16 },
	{ .bitrate = 180, .hw_value = OBUMCS_R34QAM16 },
	{ .bitrate = 240, .hw_value = OBUMCS_R23QAM64 },
	{ .bitrate = 270, .hw_value = OBUMCS_R34QAM64 }
      },
      .n_bitrates = 8
    }
  }

};

struct WCH11P_hw * pHW = &HW;

// Global HW data access/management MACROs
#define SLOCK (&(pHW->lock))
#define SLOCK_FLAGS (pHW->lock_flags)
#define STATUS (pHW->status)
#define RET (pHW->ret)

static int __init WCH11P_init(void)
{

  tMKxRadioConfig Cfg;
  tMKxRadioConfigData *pCfg = &(Cfg.RadioConfigData);

  d_print(">>>%s:%d\n",__FUNCTION__,__LINE__);
  d_print("Loading... Compilation date %s : %s\n", __DATE__, __TIME__);

  //spin_lock_init(SLOCK);
  
  // -------------------------- Init MKx
  GOTO_ASSERT( ((RET = MKx_Init (&(pHW->pMKx))) ==  OBUSTATUS_SUCCESS) && (pHW->pMKx != NULL),
	       STATUS = RET,
	       BadMKx);
  pHW->pMKx->pPriv = pHW; // Used in callbacks

  // --------------------------- Set mac80211 devices
  
  GOTO_ASSERT( NULL != (pRadio(OBU_RADIO_A)->pMacDev = ieee80211_alloc_hw( sizeof(struct radio_priv), (pRadio(OBU_RADIO_A)->pMac80211_ops))),
	       STATUS = -ENOMEM,
	       BadHwAllocA);
  pPriv(OBU_RADIO_A)->pThisRadio = pRadio(OBU_RADIO_A);
    
  GOTO_ASSERT( NULL != (pRadio(OBU_RADIO_B)->pMacDev = ieee80211_alloc_hw( sizeof(struct radio_priv), (pRadio(OBU_RADIO_B)->pMac80211_ops))),
	       STATUS = -ENOMEM,
	       BadHwAllocB);
  pPriv(OBU_RADIO_B)->pThisRadio = pRadio(OBU_RADIO_B);

  pRadio(OBU_RADIO_A)->pMacDev->wiphy->interface_modes =
    BIT(NL80211_IFTYPE_STATION) |
    BIT(NL80211_IFTYPE_ADHOC) |
    BIT(NL80211_IFTYPE_OCB);
  pRadio(OBU_RADIO_A)->pMacDev->extra_tx_headroom = sizeof(tMKxTxPacket);
  pRadio(OBU_RADIO_A)->pMacDev->flags = IEEE80211_HW_SIGNAL_DBM;
  _ieee80211_hw_set(pRadio(OBU_RADIO_A)->pMacDev,IEEE80211_HW_SIGNAL_DBM);
  SET_IEEE80211_PERM_ADDR(pRadio(OBU_RADIO_A)->pMacDev, pRadio(OBU_RADIO_A)->def_addr);
  pRadio(OBU_RADIO_A)->pMacDev->wiphy->bands[IEEE80211_BAND_2GHZ] = &pRadio(OBU_RADIO_A)->pBands[IEEE80211_BAND_2GHZ];
  pRadio(OBU_RADIO_A)->pMacDev->wiphy->bands[IEEE80211_BAND_5GHZ] = &pRadio(OBU_RADIO_A)->pBands[IEEE80211_BAND_5GHZ];
  pRadio(OBU_RADIO_A)->pMacDev->wiphy->available_antennas_tx = *(pRadio(OBU_RADIO_A)->AvailableAntennas);
  pRadio(OBU_RADIO_A)->pMacDev->wiphy->available_antennas_rx = *(pRadio(OBU_RADIO_A)->AvailableAntennas);
  pRadio(OBU_RADIO_A)->pMacDev->wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
  pRadio(OBU_RADIO_A)->pMacDev->wiphy->features = NL80211_FEATURE_VIF_TXPOWER;
  
  pRadio(OBU_RADIO_B)->pMacDev->wiphy->interface_modes =
    BIT(NL80211_IFTYPE_STATION) |
    BIT(NL80211_IFTYPE_ADHOC) |
    BIT(NL80211_IFTYPE_OCB);
  pRadio(OBU_RADIO_B)->pMacDev->extra_tx_headroom = sizeof(tMKxTxPacket);
  pRadio(OBU_RADIO_B)->pMacDev->flags = IEEE80211_HW_SIGNAL_DBM;
  _ieee80211_hw_set(pRadio(OBU_RADIO_B)->pMacDev,IEEE80211_HW_SIGNAL_DBM);
  SET_IEEE80211_PERM_ADDR(pRadio(OBU_RADIO_B)->pMacDev, pRadio(OBU_RADIO_B)->def_addr);
  pRadio(OBU_RADIO_B)->pMacDev->wiphy->bands[IEEE80211_BAND_5GHZ] = &pRadio(OBU_RADIO_B)->pBands[IEEE80211_BAND_5GHZ];
  pRadio(OBU_RADIO_B)->pMacDev->wiphy->available_antennas_tx = *(pRadio(OBU_RADIO_B)->AvailableAntennas);
  pRadio(OBU_RADIO_B)->pMacDev->wiphy->available_antennas_rx = *(pRadio(OBU_RADIO_B)->AvailableAntennas);
  pRadio(OBU_RADIO_B)->pMacDev->wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
  pRadio(OBU_RADIO_B)->pMacDev->wiphy->features = NL80211_FEATURE_VIF_TXPOWER;
  
  GOTO_ASSERT( ( 0 == (result(OBU_RADIO_A) = ieee80211_register_hw(pRadio(OBU_RADIO_A)->pMacDev))),
	       STATUS = result(OBU_RADIO_A),
	       BadHwRegA);

  GOTO_ASSERT( ( 0 == (result(OBU_RADIO_B) = ieee80211_register_hw(pRadio(OBU_RADIO_B)->pMacDev))),
	       STATUS = result(OBU_RADIO_B),
	       BadHwRegB);

  // ----------------------------  Retrieve default radio configuration from MKx and set callbacks
  pHW->pMKx->API.Callbacks = (tMKxCallbacks){
    .RxAlloc = WCH11P_RxAlloc,
    .RxInd = WCH11P_RxInd,
    .NotifInd = WCH11P_NotifInd,
    .TxCnf = WCH11P_TxCnf,
    .DebugInd = NULL,
    .C2XSecRsp = NULL
  };

  // Disable radio A
  memcpy(pCfg, &(pHW->pMKx->Config.Radio[OBU_RADIO_A]), sizeof(*pCfg));
  pCfg->Mode = OBU_MODE_OFF;
  RET = pHW->pMKx->API.Functions.Config(pHW->pMKx, OBU_RADIO_A, &Cfg);
  TEST_ASSERT(RET == OBUSTATUS_SUCCESS,);
  
  // Disable radio B
  memcpy(pCfg, &(pHW->pMKx->Config.Radio[OBU_RADIO_B]), sizeof(*pCfg));
  pCfg->Mode = OBU_MODE_OFF;
  RET = pHW->pMKx->API.Functions.Config(pHW->pMKx, OBU_RADIO_B, &Cfg);
  TEST_ASSERT(RET == OBUSTATUS_SUCCESS,);

  goto Exit;

 BadMKxComm:
  ieee80211_unregister_hw(pRadio(OBU_RADIO_B)->pMacDev);
 BadHwRegB:
  ieee80211_unregister_hw(pRadio(OBU_RADIO_A)->pMacDev);
 BadHwRegA:
  ieee80211_free_hw(pRadio(OBU_RADIO_B)->pMacDev);
 BadHwAllocB:
  ieee80211_free_hw(pRadio(OBU_RADIO_A)->pMacDev);
 BadHwAllocA:
  MKx_Exit( pHW->pMKx );
 BadMKx:
 Exit:
   
  d_print("<<<%s:%d\n",__FUNCTION__,__LINE__);
  
  return STATUS;

}

static void __exit WCH11P_exit(void)
{

  if (pRadio(OBU_RADIO_A)->pMacDev != NULL) {
    ieee80211_unregister_hw(pRadio(OBU_RADIO_A)->pMacDev);
    ieee80211_free_hw(pRadio(OBU_RADIO_A)->pMacDev);
  }
  if (pRadio(OBU_RADIO_B)->pMacDev != NULL) {
    ieee80211_unregister_hw(pRadio(OBU_RADIO_B)->pMacDev);
    ieee80211_free_hw(pRadio(OBU_RADIO_B)->pMacDev);
  }
  if (pHW->pMKx != NULL) {
    pHW->pMKx->pPriv = NULL; // Used in callbacks
    pHW->pMKx->API.Callbacks = (tMKxCallbacks){
      .RxAlloc = NULL, //WCH11P_RxAlloc;
      .RxInd = NULL, //WCH11P_RxInd;
      .NotifInd = NULL, //WCH11P_NotifInd;
      .TxCnf = NULL, //WCH11P_TxCnf;
      .DebugInd = NULL,
      .C2XSecRsp = NULL
    };
    (void)MKx_Exit( pHW->pMKx );
  };
  
  d_print("WCH11P driver unloaded succesfully\n");
  return;
  
}

module_init(WCH11P_init);
module_exit(WCH11P_exit);

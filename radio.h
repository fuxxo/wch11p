#ifndef __RADIO_H__
#define __RADIO_H__ 
#include <linux/spinlock.h>
#include "wireless-stack.h"
#include "intech/llc/llc.h"

#define CHAN2G(_freq)  { \
	.band = IEEE80211_BAND_2GHZ, \
	.center_freq = (_freq), \
	.hw_value = (_freq), \
	.max_power = 20, \
}

#define CHAN5G(_freq) { \
	.band = IEEE80211_BAND_5GHZ, \
	.center_freq = (_freq), \
	.hw_value = (_freq), \
	.max_power = 20, \
}

struct radio_priv {
  
  int result;
  spinlock_t lock;
  unsigned long lock_flags;
  
  struct ieee80211_low_level_stats stats;

  int mode;
  int channel;
  u8 bssid[ETH_ALEN];
  struct radio *pThisRadio;
  
};

struct radio {

  int status;
  int ret;
  spinlock_t lock;
  unsigned long lock_flags;

  tMKxRadio ID;
  tMKxChannelFreq ChanFreq;
  tMKxAntenna TxAntenna;
  tMKxAntenna RxAntenna;
  tMKxAntenna *AvailableAntennas;
  tMKxPower txPower;
  tMKxRadioConfig Cfg;
  struct MKx **ppMKx;
  struct ieee80211_ops *pMac80211_ops;
  struct ieee80211_hw *pMacDev;
  struct ieee80211_supported_band *pBands;
  u8 def_addr[ETH_ALEN];

};

struct WCH11P_hw {

  int status;
  int ret;
  spinlock_t lock;
  unsigned long lock_flags;
  
  struct MKx *pMKx;
  tMKxAntenna AvailableAntennas;
  tMKxNotif Notif;
  struct ieee80211_ops mac80211_ops;
  struct ieee80211_supported_band bands[IEEE80211_NUM_BANDS];
  
  struct radio radio[OBU_RADIO_COUNT];

};

//
#define DBM_TO_MDBM( val ) ((val)*100)
#define MDBM_TO_DBM( val ) ((val)/100)
#define HALFDBM_TO_DBM( val ) ( (val)/2 )
#define DBM_TO_HALFDBM( val ) ( (val)*2 )
#define HALFDBM_TO_MDBM( val ) ( (val)*50 )
#define MBDM_TO_HALFDBM( val ) ( (val)/50 )

//
// Radio management MACROs. pRadio(radio) must be defined to point to a
// 'struct radio' when using this macros
//    e.g.
//         #define pRadio(radio) ((struct radio*)(&radios[radio]))
//
// parameter <radio> can be empty if not required by pRadio
//
#define pPriv(rd) ((struct radio_priv *)(pRadio(rd)->pMacDev->priv))
#define pobu(rd) (*((struct MKx **)(pRadio(rd)->ppMKx)))
#define result(rd) (((struct radio_priv *)(pRadio(rd)->pMacDev->priv))->result)
#define pullCfg(rd)							\
  memcpy( &(pRadio(rd)->Cfg.RadioConfigData),				\
	  &((*(pRadio(rd)->ppMKx))->Config.Radio[pRadio(rd)->ID]),	\
	  sizeof(pRadio(rd)->Cfg.RadioConfigData))			
#define pushCfg(rd)							\
  (*(pRadio(rd)->ppMKx))->API.Functions.Config((*(pRadio(rd)->ppMKx)),	\
					       pRadio(rd)->ID,		\
					       &(pRadio(rd)->Cfg))			
#define setCfg(rd, param, value)                        		\
  pRadio(rd)->Cfg.RadioConfigData.param = value		
#define updateCfg(rd, param, value)					\
  ( setCfg(rd, param, value),						\
    pushCfg(rd) )									

#endif

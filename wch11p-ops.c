#include "wch11p-ops.h"
#include "debug.h"
#include "wireless-stack.h"
#include "radio.h"

// As mandatory to use "radio.h" MACROs
#define pRadio(rd) ((struct radio*)(&(((struct WCH11P_hw*)(pMKx->pPriv))->radio[ rd ])))
#define TST_FRAME_RESERVE 24

/////////////////////////////////////////////////////////////////////////
// MK5 LLC interface. Callbacks
/////////////////////////////////////////////////////////////////////////

tMKxStatus WCH11P_RxAlloc (struct MKx *pMKx,
                         int BufLen,
                         uint8_t **ppBuf,
                         void **ppPriv)
{
  tMKxStatus Res = -ENOSYS;
  struct sk_buff *pSkb = NULL;

  pSkb = alloc_skb(TST_FRAME_RESERVE + BufLen, GFP_DMA|GFP_ATOMIC);
  if (pSkb == NULL)
  {
    Res = -ENOMEM;
    goto Error;
  }

  skb_reserve(pSkb, TST_FRAME_RESERVE);
  skb_put(pSkb, BufLen);

  *ppBuf = (uint8_t *)(pSkb->data);
  *ppPriv = (void *)pSkb;

  Res = OBUSTATUS_SUCCESS;

Error:
  return Res;

}

tMKxStatus WCH11P_RxInd (struct MKx *pMKx,
                      tMKxRxPacket *pRxPkt,
                      void *pPriv)
{
  struct ieee80211_rx_status rx_status = {0};
  struct sk_buff *pSkb = (struct sk_buff *)pPriv;
  tMKxRadio ID = pRxPkt->RxPacketData.RadioID;
  
  rx_status.mactime = pRxPkt->RxPacketData.RxTSF;
  rx_status.device_timestamp = pRxPkt->RxPacketData.RxTSF;
  rx_status.flag = RX_FLAG_MACTIME_START | RX_FLAG_10MHZ;
  rx_status.freq = (*(pRadio(ID)->ppMKx))->Config.Radio[ID].ChanConfig[pRxPkt->RxPacketData.ChannelID].PHY.ChannelFreq;
  rx_status.rate_idx = pRxPkt->RxPacketData.MCS;
  rx_status.rate_idx = pRxPkt->RxPacketData.MCS - 10;
  rx_status.band = IEEE80211_BAND_5GHZ;
  rx_status.antenna = (*(pRadio(ID)->ppMKx))->Config.Radio[pRadio(ID)->ID].ChanConfig[pRxPkt->RxPacketData.ChannelID].PHY.RxAntenna;
  rx_status.signal = ( pRxPkt->RxPacketData.RxPowerA > pRxPkt->RxPacketData.RxPowerB )? pRxPkt->RxPacketData.RxPowerA : pRxPkt->RxPacketData.RxPowerB;

  memcpy(IEEE80211_SKB_RXCB(pSkb), &rx_status, sizeof(rx_status));
  
  skb_orphan(pSkb);
  skb_trim(pSkb, pRxPkt->Hdr.Len-4); // drop FCS
  skb_pull(pSkb, sizeof(tMKxRxPacket));
  // memset(&pSkb->data[16], 0xF2 , 1); // OBS mode still not implemented in Kernel

  ieee80211_rx_irqsafe( pRadio(ID)->pMacDev, pSkb);

  return OBUSTATUS_SUCCESS;
}

tMKxStatus WCH11P_NotifInd (struct MKx *pMKx,
                          tMKxNotif Notif)
{
  return OBUSTATUS_SUCCESS;
}

tMKxStatus WCH11P_TxCnf (struct MKx *pMKx,
                                  tMKxTxPacket *pTxPkt,
                                  const tMKxTxEvent *pTxEvent,
                                  void *pPriv)
{
  struct sk_buff *pSkb = pPriv;

  dev_kfree_skb(pSkb);
  
  return OBUSTATUS_SUCCESS;
}

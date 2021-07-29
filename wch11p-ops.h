#ifndef __WCH11P__OPS_H__
#define __WCH11P__OPS_H__
#include "intech/llc/llc.h"

/////////////////////////////////////////////////////////////////////////
// MK5 LLC interface. Callbacks
/////////////////////////////////////////////////////////////////////////

tMKxStatus WCH11P_RxAlloc (struct MKx *pMKx,
                         int BufLen,
                         uint8_t **ppBuf,
			    void **ppPriv);

tMKxStatus WCH11P_RxInd (struct MKx *pMKx,
                      tMKxRxPacket *pRxPkt,
			  void *pPriv);

tMKxStatus WCH11P_NotifInd (struct MKx *pMKx,
			     tMKxNotif Notif);

tMKxStatus WCH11P_TxCnf (struct MKx *pMKx,
                                  tMKxTxPacket *pTxPkt,
                                  const tMKxTxEvent *pTxEvent,
			  void *pPriv);

#endif

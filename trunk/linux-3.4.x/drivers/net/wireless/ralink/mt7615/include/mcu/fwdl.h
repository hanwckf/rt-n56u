
#ifndef _FWDL_H 
#define _FWDL_H


#ifdef MT_MAC
#define FW_NO_INIT 0
#define FW_DOWNLOAD 1
#define FW_RUN_TIME 2
#define ROM_PATCH_DOWNLOAD 3
#define FW_DOWNLOAD_SCATTER 4
#define ROM_PATCH_DOWNLOAD_SCATTER 5

INT AndesMtFwdlHookInit(struct _RTMP_ADAPTER *pAd);
#endif

#define PATCH_INFO_SIZE 30

/*
 * Command response RX Ring selection
 */
enum RX_RING_ID {
	RX_RING0,
	RX_RING1,
};

/*
 *  Load code method
 */
enum LOAD_CODE_METHOD {
	HEADER_METHOD,
	BIN_FILE_METHOD,
};


INT NICLoadFirmware(struct _RTMP_ADAPTER *pAd);

VOID NICEraseFirmware(struct _RTMP_ADAPTER *pAd);

INT FwdlHookInit(struct _RTMP_ADAPTER *pAd);
VOID NICRestartFirmware(struct _RTMP_ADAPTER *pAd);
INT NICLoadRomPatch(struct _RTMP_ADAPTER *ad);
VOID NICEraseRomPatch(struct _RTMP_ADAPTER *pAd);
#ifdef Mt7615
#if defined(RTMP_PCI_SUPPORT)
VOID NICResetFirmware(struct _RTMP_ADAPTER *pAd);
#endif
#endif
#endif

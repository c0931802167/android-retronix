/**
 *  Copyright (c) 2009-2010, Freescale Semiconductor Inc.,
 *  All Rights Reserved.
 *
 *  The following programs are the sole property of Freescale Semiconductor Inc.,
 *  and contain its proprietary and confidential information.
 *
 */

/**
 *  Copyright 2019 NXP
 *
 *  The following programs are the sole property of NXP,
 *  and contain its proprietary and confidential information.
 */

/**
 *  @file PlatformResourceMgr.h
 *  @brief Interface definition of PlatformResourceMgr
 *  @ingroup State
 */

#ifndef PlatformResourceMgr_h
#define PlatformResourceMgr_h

#include "OMX_Core.h"
#include "fsl_osal.h"
#include "List.h"

typedef struct {
    OMX_PTR pVirtualAddr;
    OMX_PTR pPhyiscAddr;
    OMX_PTR pCpuAddr;
    OMX_U32 nSize;
    OMX_S32 nFd;
    OMX_PTR pFdAddr;
}PLATFORM_DATA;

class PlatformResourceMgr {
    public:
        OMX_ERRORTYPE Init();
        OMX_ERRORTYPE DeInit();
        OMX_ERRORTYPE AddHwBuffer(OMX_PTR pPhyiscAddr, OMX_PTR pVirtualAddr);
        OMX_ERRORTYPE RemoveHwBuffer(OMX_PTR pVirtualAddr);
        OMX_PTR GetHwBuffer(OMX_PTR pVirtualAddr);
        OMX_ERRORTYPE GetFdAndAddr(OMX_PTR pVirtualAddr, OMX_S32 * fd, OMX_PTR *pFdAddr);
        OMX_ERRORTYPE ModifyFdAndAddr(OMX_PTR pVirtualAddr,OMX_S32 fd, OMX_PTR pFdAddr);
        OMX_ERRORTYPE ModifyCpuAddr(OMX_PTR pVirtualAddr, OMX_PTR pCpuAddr, OMX_U32 nSize);
        OMX_ERRORTYPE GetCpuAddr(OMX_PTR pVirtualAddr, OMX_PTR * pCpuAddr, OMX_U32 * nSize);
    private:
	List<PLATFORM_DATA> *PlatformDataList;
        fsl_osal_mutex lock;
        OMX_PTR SearchData(OMX_PTR pVirtualAddr);
};

#endif

//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
//
//--------------------------------------------------------------------------


/****************************************************************************
*
*   Module Title :     BlockMapping.h
*
*   Description  :     Common block mapping functions header
*
*
*****************************************************************************
*/

#ifndef BLOCKMAPPING_H
#define BLOCKMAPPING_H

#include "codec_common.h"
#include "pbdll.h"
/****************************************************************************
*  Constants
*****************************************************************************
*/

/****************************************************************************
*  Types
*****************************************************************************
*/        

/****************************************************************************
*   Data structures
*****************************************************************************
*/

/****************************************************************************
*  Functions
*****************************************************************************
*/

// Converts quad map coordinates to a linear index.
extern __inline INT32 QuadMapToIndex1( INT32	(*BlockMap)[4][4], UINT32 SB, UINT32 MB, UINT32 B );
extern __inline INT32 QuadMapToIndex2( INT32	(*BlockMap)[4][4], UINT32 SB, UINT32 MB, UINT32 B );
extern __inline INT32 QuadMapToMBTopLeft( INT32	(*BlockMap)[4][4], UINT32 SB, UINT32 MB );

extern void CreateBlockMapping ( INT32	(*BlockMap)[4][4], UINT32 YSuperBlocks, UINT32 UVSuperBlocks, UINT32 HFrags, UINT32 VFrags );
extern void CreateMapping ( INT32 (*BlockMap)[4][4], UINT32 FirstSB, UINT32 FirstFrag, UINT32 HFrags, UINT32 VFrags );


#endif

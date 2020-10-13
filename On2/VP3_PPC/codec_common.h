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
*   Module Title :     Codec_common.h
*
*   Description  :     Video CODEC DEMO playback dll header
*
*
*****************************************************************************
*/
#ifndef __INC_COMCODEC_H
#define __INC_COMCODEC_H

#include <string.h>

#include "type_aliases.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

#ifdef MACPPC
#define CURRENT_ENCODE_VERSION	1
#else
#define CURRENT_ENCODE_VERSION	1
#endif

#define CURRENT_DECODE_VERSION  1

// Baseline dct height and width.
#define BLOCK_HEIGHT_WIDTH	    8

// Baseline dct block size
#define BLOCK_SIZE              (BLOCK_HEIGHT_WIDTH * BLOCK_HEIGHT_WIDTH)

// Border is for unrestricted mv's
#define UMV_BORDER              16
#define STRIDE_EXTRA            (UMV_BORDER * 2)
#define Q_TABLE_SIZE            64

#define BASE_FRAME              0
#define NORMAL_FRAME            1

#define MODE_BITS               3
#define MODE_METHODS            8
#define MODE_METHOD_BITS        3

// Different key frame types/methods
#define DCT_KEY_FRAME           0

/****************************************************************************
*  Types
*****************************************************************************
*/

/* Type defining YUV data elements. */
typedef INT32 YUV_ENTRY;
typedef UINT8 YUV_BUFFER_ENTRY;
typedef UINT8 * YUV_BUFFER_ENTRY_PTR;

/* Possible methods by which a block may be coded. */
#define MAX_MODES               8 
typedef enum
{
	CODE_INTER_NO_MV	    = 0x0,		// INTER prediction, (0,0) motion vector implied.		
	CODE_INTRA			    = 0x1,		// INTRA i.e. no prediction.
	CODE_INTER_PLUS_MV	    = 0x2,		// INTER prediction, non zero motion vector.
    CODE_INTER_LAST_MV      = 0x3,      // Use Last Motion vector
	CODE_INTER_PRIOR_LAST	= 0x4,		// Prior last motion vector
	CODE_USING_GOLDEN	    = 0x5,		// 'Golden frame' prediction (no MV).
	CODE_GOLDEN_MV          = 0x6,		// 'Golden frame' prediction plus MV.
	CODE_INTER_FOURMV       = 0x7		// Inter prediction 4MV per macro block.
} CODING_MODE;


typedef struct CONFIG_TYPE
{
    // The size of the surface we want to draw to
    UINT32 VideoFrameWidth;
    UINT32 VideoFrameHeight; 
   
    UINT32 YStride;
    UINT32 UVStride;

    // The number of horizontal and vertical blocks encoded
    UINT32 HFragPixels;
    UINT32 VFragPixels;

    
} CONFIG_TYPE;

// Motion compensation
typedef struct
{
	INT32	x;
	INT32	y;
} MOTION_VECTOR;

typedef MOTION_VECTOR COORDINATE;
typedef	INT16			Q_LIST_ENTRY;
typedef Q_LIST_ENTRY	Q_LIST[64];


/****************************************************************************
*  MACROS
*****************************************************************************
*/

#define LIMIT(x)	( (x)<0 ? 0: (x)>255 ? 255: (x) )

/****************************************************************************
*  Global Variables
*****************************************************************************
*/


/****************************************************************************
*  Functions.
*****************************************************************************
*/  



extern void SetupRgbYuvAccelerators();

#endif

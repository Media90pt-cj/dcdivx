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
*   Module Title :     Reconstruct.h
*
*   Description  :     Block Reconstruction module header
*
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#ifndef RECONSTRUCT_H
#define RECONSTRUCT_H

#include "type_aliases.h"

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

// Scalar (no mmx) reconstruction functions
extern void ScalarReconIntra( PB_INSTANCE *pbi, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep );
extern void ScalarReconInter( PB_INSTANCE *pbi, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep );
extern void ScalarReconInterHalfPixel2( PB_INSTANCE *pbi, UINT8 * ReconPtr, 
	   		        		            UINT8 * RefPtr1, UINT8 * RefPtr2, 
					        	        INT16 * ChangePtr, UINT32 LineStep );

// MMx versions
extern void MMXReconIntra( PB_INSTANCE *pbi, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep );
extern void MmxReconInter( PB_INSTANCE *pbi, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep );
extern void MmxReconInterHalfPixel2( PB_INSTANCE *pbi, UINT8 * ReconPtr, 
		    	                     UINT8 * RefPtr1, UINT8 * RefPtr2, 
						             INT16 * ChangePtr, UINT32 LineStep );

extern void WmtReconIntra( PB_INSTANCE *pbi, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep );
extern void WmtReconInter( PB_INSTANCE *pbi, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep );
extern void WmtReconInterHalfPixel2( PB_INSTANCE *pbi, UINT8 * ReconPtr, 
		    	                     UINT8 * RefPtr1, UINT8 * RefPtr2, 
						             INT16 * ChangePtr, UINT32 LineStep );


#endif

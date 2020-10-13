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
*   Module Title :     MiscCommon.h
*
*   Description  :     Miscellaneous common routines header file
*
*****************************************************************************
*/


#ifndef MISCCOMP_H
#define MISCCOMP_H

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
*  Function Prototypes
*****************************************************************************
*/
extern double GetEstimatedBpb( CP_INSTANCE *cpi, UINT32 TargetQ );
extern void UpRegulateMB( CP_INSTANCE *cpi, UINT32 RegulationQ, UINT32 SB, UINT32 MB, BOOL NoCheck );
extern void UpRegulateDataStream( CP_INSTANCE *cpi, UINT32 RegulationQ, INT32 RecoveryBlocks );
extern void RegulateQ( CP_INSTANCE *cpi, INT32 UpdateScore );
extern void ConfigureQuality( CP_INSTANCE *cpi, UINT32 QualityValue );
extern void CopyBackExtraFrags(CP_INSTANCE *cpi);






#endif
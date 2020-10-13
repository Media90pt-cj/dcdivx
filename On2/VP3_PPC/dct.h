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
*   Module Title :     DCT.H
*
*   Description  :     DCT header file
*
*****************************************************************************
*/						

#ifndef DCT_HEADER
#define DCT_HEADER

#include "quantize.h"
#include "type_aliases.h"
/****************************************************************************
*  Constants
*****************************************************************************
*/

// DCT lookup tables
#define COEFF_MAX               32768      

/****************************************************************************
*  Types
*****************************************************************************
*/        

/****************************************************************************
*   Data structures
*****************************************************************************
*/

/*	Cos and Sin constant multipliers used during DCT and IDCT */
extern const double C1S7;
extern const double C2S6;
extern const double C3S5;
extern const double C4S4;
extern const double C5S3;
extern const double C6S2;
extern const double C7S1;

// DCT lookup tables and pointers
extern INT32 * C4S4_TablePtr;
extern INT32 C4S4_Table[(COEFF_MAX * 4) + 1];

extern INT32 * C6S2_TablePtr;
extern INT32 C6S2_Table[(COEFF_MAX * 2) + 1];

extern INT32 * C2S6_TablePtr;
extern INT32 C2S6_Table[(COEFF_MAX * 2) + 1];

extern INT32 * C1S7_TablePtr;
extern INT32 C1S7_Table[(COEFF_MAX * 2) + 1];

extern INT32 * C7S1_TablePtr;
extern INT32 C7S1_Table[(COEFF_MAX * 2) + 1];

extern INT32 * C3S5_TablePtr;
extern INT32 C3S5_Table[(COEFF_MAX * 2) + 1];

extern INT32 * C5S3_TablePtr;
extern INT32 C5S3_Table[(COEFF_MAX * 2) + 1];

/****************************************************************************
*  Functions
*****************************************************************************
*/

#ifdef COMPDLL
// Forward Transform
extern void fdct_slow ( INT32 * InputData, double * OutputData );
#endif

// DCT Table initialisation
extern void InitDctTables();

// Reverse Transform
//extern void IDctSlow( INT32 * InputData, INT16 * OutputData );
extern void IDctSlow(  INT16 * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void IDct10(  INT16 * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void IDct1(  INT16 * InputData, INT16 *QuantMatrix, INT16 * OutputData );
#endif

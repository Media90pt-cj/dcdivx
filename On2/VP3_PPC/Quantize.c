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
*   Module Title :     Quantise
*
*   Description  :     Quantisation and dequanitsation of an 8x8 dct block. .
*
*
*****************************************************************************
*/						

/****************************************************************************
*  Header Frames
*****************************************************************************
*/
#define STRICT              /* Strict type checking. */
#include <string.h>  
#include "pbdll.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/ 
       

// MIN_LEGAL_QUANT_ENTRY = (X * the dct normalisation factor(4))
// Designed to keep quantised values within the required number of bits

#define MIN_LEGAL_QUANT_ENTRY	8  
#define MIN_DEQUANT_VAL			2

// Scale factors used to improve precision of DCT/IDCT
#define IDCT_SCALE_FACTOR       2       // Shift left bits to improve IDCT precision
//#define DCT_SCALE_FACTOR        4.0   // Not used at the moment

#define OLD_SCHEME	1

/****************************************************************************
*  Imported Functions
*****************************************************************************
*/

/****************************************************************************
*  Imported Global Variables
*****************************************************************************
*/

/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/
void init_dequantizer ( PB_INSTANCE *pbi, UINT32 scale_factor, UINT8 QIndex );


/****************************************************************************
*  Foreward References
*****************************************************************************
*/    
          

/****************************************************************************
*  Module Statics
*****************************************************************************
*/      

// Table that relates quality index values to quantizer values
UINT32 QThreshTableV1[Q_TABLE_SIZE] = 
{ 500,  450,  400,  370,  340,  310, 285, 265,
  245,  225,  210,  195,  185,  180, 170, 160, 
  150,  145,  135,  130,  125,  115, 110, 107,
  100,   96,   93,   89,   85,   82,  75,  74,
   70,   68,   64,   60,   57,   56,  52,  50,  
   49,   45,   44,   43,   40,   38,  37,  35,  
   33,   32,   30,   29,   28,   25,  24,  22,
   21,   19,   18,   17,   15,   13,  12,  10 };


Q_LIST_ENTRY DcScaleFactorTableV1[ Q_TABLE_SIZE ] = 
{ 220, 200, 190, 180, 170, 170, 160, 160,
  150, 150, 140, 140, 130, 130, 120, 120,
  110, 110, 100, 100, 90,  90,  90,  80,
  80,  80,  70,  70,  70,  60,  60,  60,
  60,  50,  50,  50,  50,  40,  40,  40,
  40,  40,  30,  30,  30,  30,  30,  30,
  30,  20,  20,  20,  20,  20,  20,  20,
  20,  10,  10,  10,  10,  10,  10,  10 
};


Q_LIST_ENTRY Y_coeffsV1[64] =
{	16,  11,  10,  16,  24,  40,  51,  61,
	12,  12,  14,  19,  26,  58,  60,  55, 
    14,  13,  16,  24,  40,  57,  69,  56, 
	14,  17,  22,  29,  51,  87,  80,  62, 
	18,  22,  37,  58,  68, 109, 103,  77, 
	24,  35,  55,  64,  81, 104, 113,  92, 
	49,  64,  78,  87, 103, 121, 120, 101, 
 	72,  92,  95,  98, 112, 100, 103,  99
};

Q_LIST_ENTRY UV_coeffsV1[64] =
{	17,	18,	24,	47,	99,	99,	99,	99,
	18,	21,	26,	66,	99,	99,	99,	99,
	24,	26,	56,	99,	99,	99,	99,	99,
	47,	66,	99,	99,	99,	99,	99,	99,
	99,	99,	99,	99,	99,	99,	99,	99,
	99,	99,	99,	99,	99,	99,	99,	99,
	99,	99,	99,	99,	99,	99,	99,	99,
	99,	99,	99,	99,	99,	99,	99,	99
};

// Different matrices for different encoder versions
Q_LIST_ENTRY Inter_coeffsV1[64] =
{   16,  16,  16,  20,  24,  28,  32,  40,
	16,  16,  20,  24,  28,  32,  40,  48, 
    16,  20,  24,  28,  32,  40,  48,  64, 
	20,  24,  28,  32,  40,  48,  64,  64, 
	24,  28,  32,  40,  48,  64,  64,  64, 
	28,  32,  40,  48,  64,  64,  64,  96, 
 	32,  40,  48,  64,  64,  64,  96,  128,
 	40,  48,  64,  64,  64,  96,  128, 128
};


/*	Inverse fast DCT index											*/
/*	This contains the offsets needed to convert zigzag order into	*/
/*	x, y order for decoding. It is generated from the input zigzag	*/
/*	indexat run time.												*/

/*	For maximum speed during both quantisation and dequantisation	*/
/*	we maintain separate quantisation and zigzag tables for each	*/
/*	operation.														*/

/*	pbi->quant_index:	the zigzag index used during quantisation		*/
/*	dequant_index:	zigzag index used during dequantisation			*/
/*					the pbi->quant_index is the inverse of dequant_index	*/
/*					and is calculated during initialisation			*/
/*	pbi->quant_Y_coeffs:	quantising coefficients used, corrected for		*/
/*					compression ratio and DCT algorithm-specific	*/
/*					scaling, for the Y plane						*/
/*	pbi->quant_UV_coeffs	similar for the UV planes						*/
/*	pbi->dequant_Y_coeffs similarly adjusted Y coefficients for			*/
/*					dequantisation, also reordered for zigzag		*/
/*					indexing										*/
/*	pbi->dequant_UV_coeffs ditto for UV planes							*/

UINT32 dequant_index[64] = 
{	0,  1,  8,  16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

/****************************************************************************
 * 
 *  ROUTINE       :     InitQTables
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises Q tables based upon version number
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void InitQTables( PB_INSTANCE *pbi )
{  
	// Make version specific assignments.
	memcpy ( pbi->QThreshTable, QThreshTableV1, sizeof( pbi->QThreshTable ) );
}

/****************************************************************************
 * 
 *  ROUTINE       :     BuildQuantIndex_Generic
 *
 *  INPUTS        :     
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Builds the quant_index table.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void BuildQuantIndex_Generic(PB_INSTANCE *pbi)
{
    INT32 i,j;

    // invert the dequant index into the quant index
	for ( i = 0; i < BLOCK_SIZE; i++ )
	{	
        j = dequant_index[i];
		pbi->quant_index[j] = i;
	}
}


/****************************************************************************
 * 
 *  ROUTINE       :     UpdateQ
 *
 *  INPUTS        :     UINT32  NewQ
 *                              (A New Q value (50 - 1000))
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Updates the quantisation tables for a new Q
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UpdateQ( PB_INSTANCE *pbi, UINT32 NewQ )
{  
    UINT32 qscale;

	// Do bounds checking and convert to a float. 
	qscale = NewQ; 
	if ( qscale < pbi->QThreshTable[Q_TABLE_SIZE-1] )
		qscale = pbi->QThreshTable[Q_TABLE_SIZE-1];
	else if ( qscale > pbi->QThreshTable[0] )
	   qscale = pbi->QThreshTable[0];       

	// Set the inter/intra descision control variables.
	pbi->FrameQIndex = Q_TABLE_SIZE - 1;
	while ( (INT32) pbi->FrameQIndex >= 0 )
	{
		if ( (pbi->FrameQIndex == 0) || ( pbi->QThreshTable[pbi->FrameQIndex] >= NewQ) )
			break;
		pbi->FrameQIndex --;
	}

	// Re-initialise the q tables for forward and reverse transforms.    
	init_dequantizer ( pbi, qscale, (UINT8) pbi->FrameQIndex );
}


/***************************************************************************************
*  Dequantiser code for decode loop
/***************************************************************************************/

/****************************************************************************
* 
*   Routine:	init_pbi->dequantizer
*
*   Purpose:    Used to initialize the encoding/decoding data structures
*				and to select DCT algorithm	
*
*   Parameters :
*       Input :
*           UINT32          scale_factor
*                           Defines the factor by which to scale QUANT_ARRAY to
*                           produce quantization_array
*
*           UINT8           QIndex          :: 
*                           Index into Q table for current quantiser value.
*   Return value :
*       None.
*
****************************************************************************
*/

void init_dequantizer ( PB_INSTANCE *pbi, UINT32 scale_factor, UINT8 QIndex )
{
    int i, j;						 // Loop counter 
	
	// Used for decoder version specific tables
	Q_LIST_ENTRY * Inter_coeffs;	 
	Q_LIST_ENTRY * Y_coeffs;	 
	Q_LIST_ENTRY * UV_coeffs;	 
	Q_LIST_ENTRY * DcScaleFactorTable;
	Q_LIST_ENTRY * UVDcScaleFactorTable;

	// Decoder specific selections
	Inter_coeffs = Inter_coeffsV1;
	Y_coeffs = Y_coeffsV1;
	UV_coeffs = UV_coeffsV1;
	DcScaleFactorTable = DcScaleFactorTableV1;
	UVDcScaleFactorTable = DcScaleFactorTableV1;

	// invert the dequant index into the quant index
    // the dxer has a different order than the cxer.
    pbi->BuildQuantIndex(pbi);

	// Reorder dequantisation coefficients into dct zigzag order.
	for ( i = 0; i < BLOCK_SIZE; i++ )
	{	
        j = pbi->quant_index[i];
		pbi->dequant_Y_coeffs[j] = Y_coeffs[i];
	}
	for ( i = 0; i < BLOCK_SIZE; i++ )
	{	
		j = pbi->quant_index[i];
		pbi->dequant_Inter_coeffs[j] = Inter_coeffs[i];
	}
	for ( i = 0; i < BLOCK_SIZE; i++ )
	{	
        j = pbi->quant_index[i];
		pbi->dequant_UV_coeffs[j] = UV_coeffs[i];
	}
	for ( i = 0; i < BLOCK_SIZE; i++ )
	{	
		j = pbi->quant_index[i];
		pbi->dequant_InterUV_coeffs[j] = Inter_coeffs[i];
	}

    // Intra Y
    pbi->dequant_Y_coeffs[0] = (Q_LIST_ENTRY)((DcScaleFactorTable[QIndex] * pbi->dequant_Y_coeffs[0])/100);
    if ( pbi->dequant_Y_coeffs[0] < MIN_DEQUANT_VAL * 2 )
        pbi->dequant_Y_coeffs[0] = MIN_DEQUANT_VAL * 2;
    pbi->dequant_Y_coeffs[0] = pbi->dequant_Y_coeffs[0] << IDCT_SCALE_FACTOR;

    // Intra UV
    pbi->dequant_UV_coeffs[0] = (Q_LIST_ENTRY)((UVDcScaleFactorTable[QIndex] * pbi->dequant_UV_coeffs[0])/100);
    if ( pbi->dequant_UV_coeffs[0] < MIN_DEQUANT_VAL * 2 )
        pbi->dequant_UV_coeffs[0] = MIN_DEQUANT_VAL * 2;
    pbi->dequant_UV_coeffs[0] = pbi->dequant_UV_coeffs[0] << IDCT_SCALE_FACTOR;

    // Inter Y
    pbi->dequant_Inter_coeffs[0] = (Q_LIST_ENTRY)((DcScaleFactorTable[QIndex] * pbi->dequant_Inter_coeffs[0])/100);
    if ( pbi->dequant_Inter_coeffs[0] < MIN_DEQUANT_VAL * 4 )
        pbi->dequant_Inter_coeffs[0] = MIN_DEQUANT_VAL * 4;
    pbi->dequant_Inter_coeffs[0] = pbi->dequant_Inter_coeffs[0] << IDCT_SCALE_FACTOR;

    // Inter UV
    pbi->dequant_InterUV_coeffs[0] = (Q_LIST_ENTRY)((UVDcScaleFactorTable[QIndex] * pbi->dequant_InterUV_coeffs[0])/100);
    if ( pbi->dequant_InterUV_coeffs[0] < MIN_DEQUANT_VAL * 4 )
        pbi->dequant_InterUV_coeffs[0] = MIN_DEQUANT_VAL * 4;
    pbi->dequant_InterUV_coeffs[0] = pbi->dequant_InterUV_coeffs[0] << IDCT_SCALE_FACTOR;

	for ( i = 1; i < 64; i++ )
	{	
		// now scale coefficients by required compression factor
		pbi->dequant_Y_coeffs[i] = (Q_LIST_ENTRY)(( scale_factor * pbi->dequant_Y_coeffs[i] ) / 100);
		if ( pbi->dequant_Y_coeffs[i] < MIN_DEQUANT_VAL )
			pbi->dequant_Y_coeffs[i] = MIN_DEQUANT_VAL;
		pbi->dequant_Y_coeffs[i] = pbi->dequant_Y_coeffs[i] << IDCT_SCALE_FACTOR;

		pbi->dequant_UV_coeffs[i] = (Q_LIST_ENTRY)(( scale_factor * pbi->dequant_UV_coeffs[i] ) / 100);
		if ( pbi->dequant_UV_coeffs[i] < MIN_DEQUANT_VAL )
			pbi->dequant_UV_coeffs[i] = MIN_DEQUANT_VAL;
		pbi->dequant_UV_coeffs[i] = pbi->dequant_UV_coeffs[i] << IDCT_SCALE_FACTOR;

		pbi->dequant_Inter_coeffs[i] = (Q_LIST_ENTRY)(( scale_factor * pbi->dequant_Inter_coeffs[i] ) / 100);
		if ( pbi->dequant_Inter_coeffs[i] < (MIN_DEQUANT_VAL * 2) )
			pbi->dequant_Inter_coeffs[i] = MIN_DEQUANT_VAL * 2;
		pbi->dequant_Inter_coeffs[i] = pbi->dequant_Inter_coeffs[i] << IDCT_SCALE_FACTOR;

		pbi->dequant_InterUV_coeffs[i] = (Q_LIST_ENTRY)(( scale_factor * pbi->dequant_InterUV_coeffs[i] ) / 100);
		if ( pbi->dequant_InterUV_coeffs[i] < (MIN_DEQUANT_VAL * 2) )
			pbi->dequant_InterUV_coeffs[i] = MIN_DEQUANT_VAL * 2;
		pbi->dequant_InterUV_coeffs[i] = pbi->dequant_InterUV_coeffs[i] << IDCT_SCALE_FACTOR;

    }

	pbi->dequant_coeffs = pbi->dequant_Y_coeffs;

}


/****************************************************************************/
/*																			*/
/*		Select Quantisation Parameters										*/
/*																			*/
/*		void select_Y_dequantiser ( void )									*/
/*			sets dequantiser to use for intra Y         					*/
/*																			*/
/*		void select_Inter_dequantiser ( void )									*/
/*			sets dequantiser to use for inter Y         					*/
/*																			*/
/*		void select_UV_dequantiser ( void )									*/
/*			sets dequantiser to use UV compression constants				*/
/*																			*/
/****************************************************************************/
void select_Y_dequantiser ( PB_INSTANCE *pbi)
{	
    pbi->dequant_coeffs = pbi->dequant_Y_coeffs;
}																			  

void select_Inter_dequantiser ( PB_INSTANCE *pbi )
{	
    pbi->dequant_coeffs = pbi->dequant_Inter_coeffs;
}

void select_UV_dequantiser ( PB_INSTANCE *pbi )
{	
    pbi->dequant_coeffs = pbi->dequant_UV_coeffs;
}


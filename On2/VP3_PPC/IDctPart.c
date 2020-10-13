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
*   Module Title :     IDCTPart.c
*
*   Description  :     IDCT with multiple versions based on # of non 0 coeffs
*
*
*****************************************************************************
*/

/* Modules Included */
/*
void IDctSlow( int32 * InputData, int16 * OutputData );   //Original IDCT

void IDct10( int32 * InputData, int16 * OutputData );   // All zeros except the first 10 

void IDct1 ( int32 * InputData, int16 * OutputData );	// All zeros except the first 1 

*/

#include "dct.h"
#include "Quantize.h"
//#include <memory.h>
#include <string.h>

#define int32 int
#define int16 short
#define IdctAdjustBeforeShift 8

/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

#define xC1S7 64277
#define xC2S6 60547
#define xC3S5 54491
#define xC4S4 46341
#define xC5S3 36410
#define xC6S2 25080
#define xC7S1 12785


/****************************************************************************
* 
*   Routine:    dequant
*
*   Purpose:    The reverse of routine quantize, this routine takes a Q_LIST
*               list of quantized values and multipies by the relevant value in
*               quantization array. 
*
*   Parameters :  
*       Input :
*           quantized_list :: Q_LIST
*                      -- The quantized values in zig-zag order
*       Output :
*           DCT_block      :: INT16 *              
*                      -- The expanded values in a 2-D block
*
*   Return value :
*       None.
*
* 
****************************************************************************
*/
extern unsigned dequant_index[64];
static void dequant_slow( INT16 * dequant_coeffs, INT16 * quantized_list, INT32 * DCT_block)
{
    // Loop fully expanded for maximum speed
    DCT_block[dequant_index[0]] = quantized_list[0] * dequant_coeffs[0];
    DCT_block[dequant_index[1]] = quantized_list[1] * dequant_coeffs[1];
    DCT_block[dequant_index[2]] = quantized_list[2] * dequant_coeffs[2];
    DCT_block[dequant_index[3]] = quantized_list[3] * dequant_coeffs[3];
    DCT_block[dequant_index[4]] = quantized_list[4] * dequant_coeffs[4];
    DCT_block[dequant_index[5]] = quantized_list[5] * dequant_coeffs[5];
    DCT_block[dequant_index[6]] = quantized_list[6] * dequant_coeffs[6];
    DCT_block[dequant_index[7]] = quantized_list[7] * dequant_coeffs[7];
    DCT_block[dequant_index[8]] = quantized_list[8] * dequant_coeffs[8];
    DCT_block[dequant_index[9]] = quantized_list[9] * dequant_coeffs[9];
    DCT_block[dequant_index[10]] = quantized_list[10] * dequant_coeffs[10];
    DCT_block[dequant_index[11]] = quantized_list[11] * dequant_coeffs[11];
    DCT_block[dequant_index[12]] = quantized_list[12] * dequant_coeffs[12];
    DCT_block[dequant_index[13]] = quantized_list[13] * dequant_coeffs[13];
    DCT_block[dequant_index[14]] = quantized_list[14] * dequant_coeffs[14];
    DCT_block[dequant_index[15]] = quantized_list[15] * dequant_coeffs[15];
    DCT_block[dequant_index[16]] = quantized_list[16] * dequant_coeffs[16];
    DCT_block[dequant_index[17]] = quantized_list[17] * dequant_coeffs[17];
    DCT_block[dequant_index[18]] = quantized_list[18] * dequant_coeffs[18];
    DCT_block[dequant_index[19]] = quantized_list[19] * dequant_coeffs[19];
    DCT_block[dequant_index[20]] = quantized_list[20] * dequant_coeffs[20];
    DCT_block[dequant_index[21]] = quantized_list[21] * dequant_coeffs[21];
    DCT_block[dequant_index[22]] = quantized_list[22] * dequant_coeffs[22];
    DCT_block[dequant_index[23]] = quantized_list[23] * dequant_coeffs[23];
    DCT_block[dequant_index[24]] = quantized_list[24] * dequant_coeffs[24];
    DCT_block[dequant_index[25]] = quantized_list[25] * dequant_coeffs[25];
    DCT_block[dequant_index[26]] = quantized_list[26] * dequant_coeffs[26];
    DCT_block[dequant_index[27]] = quantized_list[27] * dequant_coeffs[27];
    DCT_block[dequant_index[28]] = quantized_list[28] * dequant_coeffs[28];
    DCT_block[dequant_index[29]] = quantized_list[29] * dequant_coeffs[29];
    DCT_block[dequant_index[30]] = quantized_list[30] * dequant_coeffs[30];
    DCT_block[dequant_index[31]] = quantized_list[31] * dequant_coeffs[31];
    DCT_block[dequant_index[32]] = quantized_list[32] * dequant_coeffs[32];
    DCT_block[dequant_index[33]] = quantized_list[33] * dequant_coeffs[33];
    DCT_block[dequant_index[34]] = quantized_list[34] * dequant_coeffs[34];
    DCT_block[dequant_index[35]] = quantized_list[35] * dequant_coeffs[35];
    DCT_block[dequant_index[36]] = quantized_list[36] * dequant_coeffs[36];
    DCT_block[dequant_index[37]] = quantized_list[37] * dequant_coeffs[37];
    DCT_block[dequant_index[38]] = quantized_list[38] * dequant_coeffs[38];
    DCT_block[dequant_index[39]] = quantized_list[39] * dequant_coeffs[39];
    DCT_block[dequant_index[40]] = quantized_list[40] * dequant_coeffs[40];
    DCT_block[dequant_index[41]] = quantized_list[41] * dequant_coeffs[41];
    DCT_block[dequant_index[42]] = quantized_list[42] * dequant_coeffs[42];
    DCT_block[dequant_index[43]] = quantized_list[43] * dequant_coeffs[43];
    DCT_block[dequant_index[44]] = quantized_list[44] * dequant_coeffs[44];
    DCT_block[dequant_index[45]] = quantized_list[45] * dequant_coeffs[45];
    DCT_block[dequant_index[46]] = quantized_list[46] * dequant_coeffs[46];
    DCT_block[dequant_index[47]] = quantized_list[47] * dequant_coeffs[47];
    DCT_block[dequant_index[48]] = quantized_list[48] * dequant_coeffs[48];
    DCT_block[dequant_index[49]] = quantized_list[49] * dequant_coeffs[49];
    DCT_block[dequant_index[50]] = quantized_list[50] * dequant_coeffs[50];
    DCT_block[dequant_index[51]] = quantized_list[51] * dequant_coeffs[51];
    DCT_block[dequant_index[52]] = quantized_list[52] * dequant_coeffs[52];
    DCT_block[dequant_index[53]] = quantized_list[53] * dequant_coeffs[53];
    DCT_block[dequant_index[54]] = quantized_list[54] * dequant_coeffs[54];
    DCT_block[dequant_index[55]] = quantized_list[55] * dequant_coeffs[55];
    DCT_block[dequant_index[56]] = quantized_list[56] * dequant_coeffs[56];
    DCT_block[dequant_index[57]] = quantized_list[57] * dequant_coeffs[57];
    DCT_block[dequant_index[58]] = quantized_list[58] * dequant_coeffs[58];
    DCT_block[dequant_index[59]] = quantized_list[59] * dequant_coeffs[59];
    DCT_block[dequant_index[60]] = quantized_list[60] * dequant_coeffs[60];
    DCT_block[dequant_index[61]] = quantized_list[61] * dequant_coeffs[61];
    DCT_block[dequant_index[62]] = quantized_list[62] * dequant_coeffs[62];
    DCT_block[dequant_index[63]] = quantized_list[63] * dequant_coeffs[63];
}

/************************************************/
/* Module Name:		IDctSlow					*/
/*												*/
/* Description:		IDCT for Blocks only have	*/
/*					less than 10 coefficents    */
/************************************************/


void IDctSlow(  Q_LIST_ENTRY * InputData, int16 *QuantMatrix, int16 * OutputData )
{
	int32 IntermediateData[64];
	int32 * ip = IntermediateData;
	int16 * op = OutputData;
	
	int32 _varA, _varB, _varC, _varD, _varAd, _varBd, _varCd, _varDd, _varE, _varF, _varG, _varH;
	int32 _varEd, _varGd, _varAdd, _varBdd, _varFd, _varHd;
	int32 t1, t2;

	int loop;
	
	// dequantize the input 
	dequant_slow( QuantMatrix, InputData, IntermediateData);

	// Inverse DCT on the rows now
	for ( loop = 0; loop < 8; loop++)
	{
		// Check for non-zero values
		if ( ip[0] | ip[1] | ip[2] | ip[3] | ip[4] | ip[5] | ip[6] | ip[7] )
		{
			t1 = (int32)(xC1S7 * ip[1]);
            t2 = (int32)(xC7S1 * ip[7]);
            t1 >>= 16;
            t2 >>= 16;
//            t1 += ip[1];
			_varA = t1 + t2;

			t1 = (int32)(xC7S1 * ip[1]);
			t2 = (int32)(xC1S7 * ip[7]);
            t1 >>= 16;
            t2 >>= 16;
//            t2 += ip[7];
			_varB = t1 - t2;

			t1 = (int32)(xC3S5 * ip[3]);
			t2 = (int32)(xC5S3 * ip[5]);
            t1 >>= 16;
            t2 >>= 16;
//            t1 += ip[3];
//            t2 += ip[5];
			_varC = t1 + t2;

			t1 = (int32)(xC3S5 * ip[5]);
			t2 = (int32)(xC5S3 * ip[3]);
            t1 >>= 16;
            t2 >>= 16;
//            t1 += ip[5];
//            t2 += ip[3];
			_varD = t1 - t2;


			t1 = (int32)(xC4S4 * (_varA - _varC));
            t1 >>= 16;
//            t1 += (_varA - _varC);
			_varAd = t1;

			t1 = (int32)(xC4S4 * (_varB - _varD));
            t1 >>= 16;
//            t1 += (_varB - _varD);
			_varBd = t1;
			

			_varCd = _varA + _varC;
			_varDd = _varB + _varD;

			t1 = (int32)(xC4S4 * (ip[0] + ip[4]));
            t1 >>= 16;
//            t1 += (ip[0] + ip[4]);
			_varE = t1;

			t1 = (int32)(xC4S4 * (ip[0] - ip[4]));
            t1 >>= 16;
//            t1 += (ip[0] - ip[4]);
			_varF = t1;
			
			t1 = (int32)(xC2S6 * ip[2]);
			t2 = (int32)(xC6S2 * ip[6]);
            t1 >>= 16;
            t2 >>= 16;
//            t1 += ip[2];
			_varG = t1 + t2;

			t1 = (int32)(xC6S2 * ip[2]);
			t2 = (int32)(xC2S6 * ip[6]);
            t1 >>= 16;
            t2 >>= 16;
//            t2 += ip[6];
			_varH = t1 - t2;
			

			_varEd = _varE - _varG;
			_varGd = _varE + _varG;

			_varAdd = _varF + _varAd;
			_varBdd = _varBd - _varH;
			
			_varFd = _varF - _varAd;
			_varHd = _varBd + _varH;
	
			// Final sequence of operations over-write original inputs.
			ip[0] = (int16)((_varGd + _varCd )   >> 0);
			ip[7] = (int16)((_varGd - _varCd )   >> 0);

			ip[1] = (int16)((_varAdd + _varHd )  >> 0);
			ip[2] = (int16)((_varAdd - _varHd )  >> 0);

			ip[3] = (int16)((_varEd + _varDd )   >> 0);
			ip[4] = (int16)((_varEd - _varDd )   >> 0);

			ip[5] = (int16)((_varFd + _varBdd )  >> 0);
			ip[6] = (int16)((_varFd - _varBdd )  >> 0);

		}

		ip += 8;			/* next row */
	}

	ip = IntermediateData;

	for ( loop = 0; loop < 8; loop++)
	{	
		// Check for non-zero values (bitwise or faster than ||)
		if ( ip[0 * 8] | ip[1 * 8] | ip[2 * 8] | ip[3 * 8] |
			 ip[4 * 8] | ip[5 * 8] | ip[6 * 8] | ip[7 * 8] )
		{

			t1 = (int32)(xC1S7 * ip[1*8]);
            t2 = (int32)(xC7S1 * ip[7*8]);
            t1 >>= 16;
            t2 >>= 16;
//            t1 += ip[1*8];
			_varA = t1 + t2;

			t1 = (int32)(xC7S1 * ip[1*8]);
			t2 = (int32)(xC1S7 * ip[7*8]);
            t1 >>= 16;
            t2 >>= 16;
//            t2 += ip[7*8];
			_varB = t1 - t2;

			t1 = (int32)(xC3S5 * ip[3*8]);
			t2 = (int32)(xC5S3 * ip[5*8]);
            t1 >>= 16;
            t2 >>= 16;
//            t1 += ip[3*8];
//            t2 += ip[5*8];
			_varC = t1 + t2;

			t1 = (int32)(xC3S5 * ip[5*8]);
			t2 = (int32)(xC5S3 * ip[3*8]);
            t1 >>= 16;
            t2 >>= 16;
//            t1 += ip[5*8];
//            t2 += ip[3*8];
			_varD = t1 - t2;


			t1 = (int32)(xC4S4 * (_varA - _varC));
            t1 >>= 16;
//            t1 += (_varA - _varC);
			_varAd = t1;

			t1 = (int32)(xC4S4 * (_varB - _varD));
            t1 >>= 16;
//            t1 += (_varB - _varD);
			_varBd = t1;
			

			_varCd = _varA + _varC;
			_varDd = _varB + _varD;

			t1 = (int32)(xC4S4 * (ip[0*8] + ip[4*8]));
            t1 >>= 16;
//            t1 += (ip[0*8] + ip[4*8]);
			_varE = t1;

			t1 = (int32)(xC4S4 * (ip[0*8] - ip[4*8]));
            t1 >>= 16;
//            t1 += (ip[0*8] - ip[4*8]);
			_varF = t1;
			
			t1 = (int32)(xC2S6 * ip[2*8]);
			t2 = (int32)(xC6S2 * ip[6*8]);
            t1 >>= 16;
            t2 >>= 16;
//            t1 += ip[2*8];
			_varG = t1 + t2;

			t1 = (int32)(xC6S2 * ip[2*8]);
			t2 = (int32)(xC2S6 * ip[6*8]);
            t1 >>= 16;
            t2 >>= 16;
//            t2 += ip[6*8];
			_varH = t1 - t2;
			

			_varEd = _varE - _varG;
			_varGd = _varE + _varG;

			_varAdd = _varF + _varAd;
			_varBdd = _varBd - _varH;
			
			_varFd = _varF - _varAd;
			_varHd = _varBd + _varH;
	
			_varGd += IdctAdjustBeforeShift;
			_varAdd += IdctAdjustBeforeShift;
			_varEd += IdctAdjustBeforeShift;
			_varFd += IdctAdjustBeforeShift;

			// Final sequence of operations over-write original inputs.
			op[0*8] = (int16)((_varGd + _varCd )   >> 4);
			op[7*8] = (int16)((_varGd - _varCd )   >> 4);

			op[1*8] = (int16)((_varAdd + _varHd )  >> 4);
			op[2*8] = (int16)((_varAdd - _varHd )  >> 4);

			op[3*8] = (int16)((_varEd + _varDd )   >> 4);
			op[4*8] = (int16)((_varEd - _varDd )   >> 4);

			op[5*8] = (int16)((_varFd + _varBdd )  >> 4);
			op[6*8] = (int16)((_varFd - _varBdd )  >> 4);
		}
		else
		{
			op[0*8] = 0;
			op[7*8] = 0;
			op[1*8] = 0;
			op[2*8] = 0;
			op[3*8] = 0;
			op[4*8] = 0;
			op[5*8] = 0;
			op[6*8] = 0;
		}

		ip++;			// next column
        op++;
	}

}




/****************************************************************************
* 
*   Routine:    dequant10
*
*   Purpose:    The reverse of routine quantize, this routine takes a Q_varLIST
*               list of quantized values and multipies by the relevant value in
*               quantization array. 
*
*   Parameters :  
*       Input :
*           quantized_varlist :: Q_varLIST
*                      -- The quantized values in zig-zag order
*       Output :
*           DCT_varblock      :: INT16 *              
*                      -- The expanded values in a 2-D block
*
*   Return value :
*       None.
*
* 
****************************************************************************
*/
extern unsigned dequant_index[64];
static void dequant_slow10( INT16 * dequant_coeffs, INT16 * quantized_list, INT32 * DCT_block)
{
    
	INT32* t=(INT32*)DCT_block;
	int i;
	for(i=0;i<32;i++)
	{
		t[0]=0;
		t++;
	}
//	memset(DCT_block,0, 128);
	// Loop fully expanded for maximum speed
    DCT_block[dequant_index[0]] = quantized_list[0] * dequant_coeffs[0];
    DCT_block[dequant_index[1]] = quantized_list[1] * dequant_coeffs[1];
    DCT_block[dequant_index[2]] = quantized_list[2] * dequant_coeffs[2];
    DCT_block[dequant_index[3]] = quantized_list[3] * dequant_coeffs[3];
    DCT_block[dequant_index[4]] = quantized_list[4] * dequant_coeffs[4];
    DCT_block[dequant_index[5]] = quantized_list[5] * dequant_coeffs[5];
    DCT_block[dequant_index[6]] = quantized_list[6] * dequant_coeffs[6];
    DCT_block[dequant_index[7]] = quantized_list[7] * dequant_coeffs[7];
    DCT_block[dequant_index[8]] = quantized_list[8] * dequant_coeffs[8];
    DCT_block[dequant_index[9]] = quantized_list[9] * dequant_coeffs[9];
    DCT_block[dequant_index[10]] = quantized_list[10] * dequant_coeffs[10];

}

/************************************************/
/* Module Name:		IDct10						*/
/*												*/
/* Description:		IDCT for Blocks only have	*/
/*					less than 10 coefficents    */
/************************************************/
//
//////////////////////////
// x  x  x  x  0  0  0  0 
// x  x  x  0  0  0  0  0
// x  x  0  0  0  0  0  0
// x  0  0  0  0  0  0  0 
// 0  0  0  0  0  0  0  0
// 0  0  0  0  0  0  0  0
// 0  0  0  0  0  0  0  0
// 0  0  0  0  0  0  0  0
//////////////////////////

void IDct10( Q_LIST_ENTRY * InputData, int16 *QuantMatrix, int16 * OutputData )
{
	int32 IntermediateData[64];
	int32 * ip = IntermediateData;
	int16 * op = OutputData;

	int32 _varA, _varB, _varC, _varD, _varAd, _varBd, _varCd, _varDd, _varE, _varF, _varG, _varH;
	int32 _varEd, _varGd, _varAdd, _varBdd, _varFd, _varHd;
	int32 t1, t2;

	int loop;
	
	// dequantize the input 
	dequant_slow10( QuantMatrix, InputData, IntermediateData);


	// Inverse DCT on the rows now
	for ( loop = 0; loop < 4; loop++)
	{
		// Check for non-zero values
		if ( ip[0] | ip[1] | ip[2] | ip[3] )
		{
			t1 = (int32)(xC1S7 * ip[1]);
            t1 >>= 16;
			_varA = t1; 

			t1 = (int32)(xC7S1 * ip[1]);
            t1 >>= 16;
			_varB = t1 ;

			t1 = (int32)(xC3S5 * ip[3]);
            t1 >>= 16;
			_varC = t1; 

			t2 = (int32)(xC5S3 * ip[3]);
            t2 >>= 16;
			_varD = -t2; 


			t1 = (int32)(xC4S4 * (_varA - _varC));
            t1 >>= 16;
			_varAd = t1;

			t1 = (int32)(xC4S4 * (_varB - _varD));
            t1 >>= 16;
			_varBd = t1;
			

			_varCd = _varA + _varC;
			_varDd = _varB + _varD;

			t1 = (int32)(xC4S4 * ip[0] );
            t1 >>= 16;
			_varE = t1;

			_varF = t1;
			
			t1 = (int32)(xC2S6 * ip[2]);
            t1 >>= 16;
			_varG = t1; 

			t1 = (int32)(xC6S2 * ip[2]);
            t1 >>= 16;
			_varH = t1 ;
			

			_varEd = _varE - _varG;
			_varGd = _varE + _varG;

			_varAdd = _varF + _varAd;
			_varBdd = _varBd - _varH;
			
			_varFd = _varF - _varAd;
			_varHd = _varBd + _varH;
	
			// Final sequence of operations over-write original inputs.
			ip[0] = (int16)((_varGd + _varCd )   >> 0);
			ip[7] = (int16)((_varGd - _varCd )   >> 0);

			ip[1] = (int16)((_varAdd + _varHd )  >> 0);
			ip[2] = (int16)((_varAdd - _varHd )  >> 0);

			ip[3] = (int16)((_varEd + _varDd )   >> 0);
			ip[4] = (int16)((_varEd - _varDd )   >> 0);

			ip[5] = (int16)((_varFd + _varBdd )  >> 0);
			ip[6] = (int16)((_varFd - _varBdd )  >> 0);

		}

		ip += 8;			/* next row */
	}

	ip = IntermediateData;

	for ( loop = 0; loop < 8; loop++)
	{	
		// Check for non-zero values (bitwise or faster than ||)
		if ( ip[0 * 8] | ip[1 * 8] | ip[2 * 8] | ip[3 * 8] )
		{

			t1 = (int32)(xC1S7 * ip[1*8]);
            t1 >>= 16;
			_varA = t1 ;

			t1 = (int32)(xC7S1 * ip[1*8]);
            t1 >>= 16;
			_varB = t1 ;

			t1 = (int32)(xC3S5 * ip[3*8]);
            t1 >>= 16;
			_varC = t1 ;

			t2 = (int32)(xC5S3 * ip[3*8]);
            t2 >>= 16;
			_varD = - t2;


			t1 = (int32)(xC4S4 * (_varA - _varC));
            t1 >>= 16;
//            t1 += (_varA - _varC);
			_varAd = t1;

			t1 = (int32)(xC4S4 * (_varB - _varD));
            t1 >>= 16;
//            t1 += (_varB - _varD);
			_varBd = t1;
			

			_varCd = _varA + _varC;
			_varDd = _varB + _varD;

			t1 = (int32)(xC4S4 * ip[0*8]);
            t1 >>= 16;
			_varE = t1;
			_varF = t1;
			
			t1 = (int32)(xC2S6 * ip[2*8]);
            t1 >>= 16;
			_varG = t1;

			t1 = (int32)(xC6S2 * ip[2*8]);
            t1 >>= 16;
			_varH = t1;
			

			_varEd = _varE - _varG;
			_varGd = _varE + _varG;

			_varAdd = _varF + _varAd;
			_varBdd = _varBd - _varH;
			
			_varFd = _varF - _varAd;
			_varHd = _varBd + _varH;
	
			_varGd += IdctAdjustBeforeShift;
			_varAdd += IdctAdjustBeforeShift;
			_varEd += IdctAdjustBeforeShift;
			_varFd += IdctAdjustBeforeShift;

			// Final sequence of operations over-write original inputs.
			op[0*8] = (int16)((_varGd + _varCd )   >> 4);
			op[7*8] = (int16)((_varGd - _varCd )   >> 4);

			op[1*8] = (int16)((_varAdd + _varHd )  >> 4);
			op[2*8] = (int16)((_varAdd - _varHd )  >> 4);

			op[3*8] = (int16)((_varEd + _varDd )   >> 4);
			op[4*8] = (int16)((_varEd - _varDd )   >> 4);

			op[5*8] = (int16)((_varFd + _varBdd )  >> 4);
			op[6*8] = (int16)((_varFd - _varBdd )  >> 4);
		}
		else
		{
			op[0*8] = 0;
			op[7*8] = 0;
			op[1*8] = 0;
			op[2*8] = 0;
			op[3*8] = 0;
			op[4*8] = 0;
			op[5*8] = 0;
			op[6*8] = 0;
		}

		ip++;			// next column
op++;
	}

}



/************************************************/
/* Module Name:		IDct1						*/
/*												*/
/* Description:		IDCT for Blocks only have	*/
/*					less than 1 coefficents		*/
/************************************************/
////////////////////////////
// x   0   0  0  0  0  0  0	
// 0   0   0  0  0  0  0  0	
// 0   0   0  0  0  0  0  0	
// 0   0   0  0  0  0  0  0	
// 0   0   0  0  0  0  0  0	
// 0   0   0  0  0  0  0  0	
// 0   0   0  0  0  0  0  0	
// 0   0   0  0  0  0  0  0	
///////////////////////////

void IDct1( Q_LIST_ENTRY * InputData, int16 *QuantMatrix, INT16 * OutputData )
{
    INT32 loop;
	
	INT16  OutD;
	INT32* tmp=(INT32*)OutputData;
	OutD=(INT16) ((INT32)(InputData[0]*QuantMatrix[0]+15)>>5);

	for(loop=0;loop<32;loop++)
		tmp[loop]=(((INT32)OutD)<<16)+OutD;


}




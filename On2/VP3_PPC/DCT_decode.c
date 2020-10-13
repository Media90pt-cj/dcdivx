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
*   Module Title :     DCT_Decode.c
*
*   Description  :     DCT block expansion, decode and reconstruction functions
*
*
*****************************************************************************
*/

/****************************************************************************
*  Header Frames
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "pbdll.h"
#include "Dct.h"
#include "Huffman.h"
#include "Quantize.h"
#include "Reconstruct.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
extern int render;
/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
#define GOLDEN_FRAME_THRESH_Q   50

/****************************************************************************
*  Explicit Imports
*****************************************************************************
*/              
UINT8 LimitVal_VP31[VAL_RANGE * 3];


extern UnpackBlock ( UINT8 *, INT16 *, UINT32 );
extern ReconBlock ( INT16 *, INT16 *, UINT8 *, UINT32);

extern void ReconMotionBlock(PB_INSTANCE *pbi, UINT32 FragmentNumber, INT32 MvShift, INT32 MvModMask, 
                      UINT32 ReconPixelIndex, UINT32 ReconPixelsPerLine);
/* Last Inter frame DC values */
/*
extern Q_LIST_ENTRY InvLastInterDC;
extern Q_LIST_ENTRY InvLastIntraDC;
*/

/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

/****************************************************************************
*  Exported Functions
*****************************************************************************
*/              

/****************************************************************************
*  Module Statics
*****************************************************************************
*/              
//****************************************************************************
// Copied From DCT_decode.c 

UINT32 LoopFilterLimitValuesV1[Q_TABLE_SIZE] = {  30, 25, 20, 20, 15, 15, 14, 14,
                                                  13, 13, 12, 12, 11, 11, 10, 10, 
                                                  9,  9,  8,  8,  7,  7,  7,  7,
                                                  6,  6,  6,  6,  5,  5,  5,  5,
                                                  4,  4,  4,  4,  3,  3,  3,  3,  
                                                  2,  2,  2,  2,  2,  2,  2,  2,  
                                                  0,  0,  0,  0,  0,  0,  0,  0,  
                                                  0,  0,  0,  0,  0,  0,  0,  0 };

UINT32 LoopFilterLimitValuesV2[Q_TABLE_SIZE] = {  30, 25, 20, 20, 15, 15, 14, 14,
                                                  13, 13, 12, 12, 11, 11, 10, 10, 
                                                  9,  9,  8,  8,  7,  7,  7,  7,
                                                  6,  6,  6,  6,  5,  5,  5,  5,
                                                  4,  4,  4,  4,  3,  3,  3,  3,  
                                                  2,  2,  2,  2,  2,  2,  2,  2,  
                                                  2,  2,  2,  2,  2,  2,  2,  2,  
                                                  1,  1,  1,  1,  1,  1,  1,  1 };


/****************************************************************************
*  Foreward References
*****************************************************************************
*/              
void ExpandBlock2 ( PB_INSTANCE *pbi, INT32 FragmentNumber );
void CopyRecon( PB_INSTANCE *pbi, UINT8 * DestReconPtr, UINT8 * SrcReconPtr );
void CopyNotRecon( PB_INSTANCE *pbi, UINT8 * DestReconPtr, UINT8 * SrcReconPtr );
void UpdateUMVBorder( PB_INSTANCE *pbi, UINT8 * DestReconPtr );
void UpdateUMV_HBorders( PB_INSTANCE *pbi, UINT8 * DestReconPtr, UINT32  PlaneFragOffset );
void UpdateUMV_VBorders( PB_INSTANCE *pbi, UINT8 * DestReconPtr, UINT32  PlaneFragOffset );
void ApplyReconLoopFilter(PB_INSTANCE *pbi);
void LoopFilter(PB_INSTANCE *pbi);


/****************************************************************************
 * 
 *  ROUTINE       :     SetupLoopFilter
 *
 *  INPUTS        :     None
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     prepares to Apply a loop filter 
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void SetupLoopFilter(PB_INSTANCE *pbi)
{
    INT32 FLimit; 

	FLimit = LoopFilterLimitValuesV2[pbi->FrameQIndex];
	
	pbi->BoundingValuePtr = pbi->SetupBoundingValueArray(pbi, FLimit);
}

/****************************************************************************
 * 
 *  ROUTINE       :     CopyBlock
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Copies a block from source to destination
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
#ifndef ARM
#ifdef MIPS


void CopyBlock(unsigned char * Src, unsigned char * Dst, int Stride) 
{ 

__asm(	"addi	$2,$0,+8;" 
"bucle	:uld	$10,0($4);" 
		"add	$2,$2,-1;" 
		"add	$4,$4,$6;"
		"sdr	$10,0($5);" 
		"add	$5,$5,$6;" 
"bgtz	$2,bucle;"); 

} 

#else
void CopyBlock(unsigned char *src, unsigned char *dest, unsigned int srcstride)
{
	unsigned char *s = src;
	unsigned char *d = dest;
	unsigned int stride = srcstride;

	int j;
    for ( j = 0; j < 8; j++ )
	{
		((UINT32*)d)[0] = ((UINT32*)s)[0];
		((UINT32*)d)[1] = ((UINT32*)s)[1];
		s+=stride;
		d+=stride;
	}
}
#endif
#endif
/****************************************************************************
 * 
 *  ROUTINE       :     ReconRefFrames
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Reconstructs the various reference frames
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void ReconRefFrames (PB_INSTANCE *pbi)
{
    INT32 i;
	INT32 FragIndex;			    // Fragment number
	UINT8 *SwapReconBuffersTemp;

#define PUL 8
#define PU 4
#define PUR 2
#define PL 1
	unsigned long uvHFragments = pbi->HFragments >> 1;


	short pc[16][6]=
	{
		{0,0,0,0,0,0},	
		{0,0,0,1,0,0},		// PL
		{0,0,1,0,0,0},		// PUR
		{0,0,53,75,7,127},	// PUR|PL
		{0,1,0,0,0,0},		// PU
		{0,1,0,1,1,1},		// PU|PL
		{0,1,0,0,0,0},		// PU|PUR
		{0,0,53,75,7,127},	// PU|PUR|PL
		{1,0,0,0,0,0},		// PUL|
		{0,0,0,1,0,0},		// PUL|PL
		{1,0,1,0,1,1},		// PUL|PUR
		{0,0,53,75,7,127},	// PUL|PUR|PL
		{0,1,0,0,0,0},		// PUL|PU
		{-26,29,0,29,5,31}, // PUL|PU|PL
		{3,10,3,0,4,15},	// PUL|PU|PUR
		{-26,29,0,29,5,31}	// PUL|PU|PUR|PL
	};
	// fragment left fragment up-left, fragment up, fragment up-right
	int fl,ful,fu,fur;

	// value left value up-left, value up, value up-right
	int vl,vul,vu,vur;

	// fragment number left, up-left, up, up-right
	int l,ul,u,ur;

	//which predictor constants to use
	short wpc;

	short Mode2Frame[] =
	{
		1,	// CODE_INTER_NO_MV		0 => Encoded diff from same MB last frame 
		0,	// CODE_INTRA			1 => DCT Encoded Block
		1,	// CODE_INTER_PLUS_MV	2 => Encoded diff from included MV MB last frame
		1,	// CODE_INTER_LAST_MV	3 => Encoded diff from MRU MV MB last frame
		1,	// CODE_INTER_PRIOR_MV	4 => Encoded diff from included 4 separate MV blocks
		2,	// CODE_USING_GOLDEN	5 => Encoded diff from same MB golden frame
		2,	// CODE_GOLDEN_MV		6 => Encoded diff from included MV MB golden frame
		1	// CODE_INTER_FOUR_MV	7 => Encoded diff from included 4 separate MV blocks
	};
	short Last[3];	// last value used for given frame
	short PredictedDC;
	void (*ExpandBlockA) ( PB_INSTANCE *pbi, INT32 FragmentNumber );
	

#define HIGHBITDUPPED(X) (((signed short) X)  >> 15)

	int FragsAcross=pbi->HFragments;	
	int FromFragment,ToFragment;
	int FragsDown = pbi->VFragments;

	int WhichFrame;
	int WhichCase;

	int j,k,m,n;

	/* Search Points are ordered by distance from 0,0 only DCSearchpointCount positions checked!
		-4 -3 -2 -1  0  1  2  3  4 
	-4	         21 19 22         
	-3     24 15 11  9 12 16 25   
	-2     14  6  3  1  4  7 17   
	-1  20 10  2  z  z  z  5 13 23
	 0	18  8  0  z  z	z  z  z  z
	*/
	struct SearchPoints
	{
		int RowOffset;
		int ColOffset;
	} DCSearchPoints[]=
	{
		{0,-2},{-2,0},{-1,-2},{-2,-1},{-2,1},{-1,2},{-2,-2},{-2,2},{0,-3},
		{-3,0},{-1,-3},{-3,-1},{-3,1},{-1,3},{-2,-3},{-3,-2},{-3,2},{-2,3},
		{0,-4},{-4,0},{-1,-4},{-4,-1},{-4,1},{-1,4},{-3,-3},{-3,3}
	};
	//int DCSearchPointCount = sizeof(DCSearchPoints) / ( 2 * sizeof(int));
	int DCSearchPointCount = 0;

    if ( GetFrameType(pbi) == BASE_FRAME )
		ExpandBlockA=ExpandKFBlock;
	else
    {
   		ExpandBlockA=ExpandBlock;
    }


    // testing ?
    SetupLoopFilter(pbi);

	// for y,u,v
	for ( j = 0; j < 3 ; j++)
	{
		// pick which fragments based on Y, U, V
		switch(j)
		{
		case 0: // y
			FromFragment = 0;
			ToFragment = pbi->YPlaneFragments;
			FragsAcross = pbi->HFragments;
			FragsDown = pbi->VFragments;
			break;
		case 1: // u
			FromFragment = pbi->YPlaneFragments;
			ToFragment = pbi->YPlaneFragments + pbi->UVPlaneFragments ;
			FragsAcross = pbi->HFragments >> 1;
			FragsDown = pbi->VFragments >> 1;
			break;
		case 2:	// v
			FromFragment = pbi->YPlaneFragments + pbi->UVPlaneFragments;
			ToFragment = pbi->YPlaneFragments + (2 * pbi->UVPlaneFragments) ;
			FragsAcross = pbi->HFragments >> 1;
			FragsDown = pbi->VFragments >> 1;
			break;
		}

		// initialize our array of last used DC Components
		for(k=0;k<3;k++)
			Last[k]=0;

		i=FromFragment;

		// do prediction on all of Y, U or V
		for ( m = 0 ; m < FragsDown ; m++)
		{
			for ( n = 0 ; n < FragsAcross ; n++, i++)
			{
				
				// only do 2 prediction if fragment coded and on non intra or if all fragments are intra 
				if( pbi->display_fragments[i] || (GetFrameType(pbi) == BASE_FRAME) )
				{
					// Type of Fragment
					WhichFrame = Mode2Frame[pbi->FragCodingMethod[i]];

					// Check Borderline Cases
					WhichCase = (n==0) + ((m==0) << 1) + ((n+1 == FragsAcross) << 2);

					switch(WhichCase)
					{
					case 0: // normal case no border condition

						// calculate values left, up, up-right and up-left
						l = i-1;
						u = i - FragsAcross;
						ur = i - FragsAcross + 1;
						ul = i - FragsAcross - 1;

						// calculate values
						vl = pbi->QFragData[l][0];
						vu = pbi->QFragData[u][0];
						vur = pbi->QFragData[ur][0];
						vul = pbi->QFragData[ul][0];
						
						// fragment valid for prediction use if coded and it comes from same frame as the one we are predicting
						fl = pbi->display_fragments[l] && (Mode2Frame[pbi->FragCodingMethod[l]] == WhichFrame);
						fu = pbi->display_fragments[u] && (Mode2Frame[pbi->FragCodingMethod[u]] == WhichFrame);
						fur = pbi->display_fragments[ur] && (Mode2Frame[pbi->FragCodingMethod[ur]] == WhichFrame);
						ful = pbi->display_fragments[ul] && (Mode2Frame[pbi->FragCodingMethod[ul]] == WhichFrame);

						// calculate which predictor to use 
						wpc = (fl*PL) | (fu*PU) | (ful*PUL) | (fur*PUR);

						break;

					case 1: // n == 0 Left Column

						// calculate values left, up, up-right and up-left
						u = i - FragsAcross;
						ur = i - FragsAcross + 1;

						// calculate values
						vu = pbi->QFragData[u][0];
						vur = pbi->QFragData[ur][0];

						// fragment valid for prediction if coded and it comes from same frame as the one we are predicting
						fu = pbi->display_fragments[u] && (Mode2Frame[pbi->FragCodingMethod[u]] == WhichFrame);
						fur = pbi->display_fragments[ur] && (Mode2Frame[pbi->FragCodingMethod[ur]] == WhichFrame);

						// calculate which predictor to use 
						wpc = (fu*PU) | (fur*PUR);

						break;

					case 2: // m == 0 Top Row 
					case 6: // m == 0 and n+1 == FragsAcross or Top Row Right Column

						// calculate values left, up, up-right and up-left
						l = i-1;

						// calculate values
						vl = pbi->QFragData[l][0];

						// fragment valid for prediction if coded and it comes from same frame as the one we are predicting
						fl = pbi->display_fragments[l] && (Mode2Frame[pbi->FragCodingMethod[l]] == WhichFrame);

						// calculate which predictor to use 
						wpc = (fl*PL) ;

						break;

					case 3: // n == 0 & m == 0 Top Row Left Column

						wpc = 0;

						break;

					case 4: // n+1 == FragsAcross : Right Column

						// calculate values left, up, up-right and up-left
						l = i-1;
						u = i - FragsAcross;
						ul = i - FragsAcross - 1;

						// calculate values
						vl = pbi->QFragData[l][0];
						vu = pbi->QFragData[u][0];
						vul = pbi->QFragData[ul][0];
						
						// fragment valid for prediction if coded and it comes from same frame as the one we are predicting
						fl = pbi->display_fragments[l] && (Mode2Frame[pbi->FragCodingMethod[l]] == WhichFrame);
						fu = pbi->display_fragments[u] && (Mode2Frame[pbi->FragCodingMethod[u]] == WhichFrame);
						ful = pbi->display_fragments[ul] && (Mode2Frame[pbi->FragCodingMethod[ul]] == WhichFrame);

						// calculate which predictor to use 
						wpc = (fl*PL) | (fu*PU) | (ful*PUL) ;

						break;

					}
					
					
					if(wpc==0)
					{
						FragIndex = 1;
						
						// find the nearest one that is coded 
						for( k = 0; k < DCSearchPointCount ; k++)
						{
							FragIndex = i + DCSearchPoints[k].RowOffset * FragsAcross + DCSearchPoints[k].ColOffset;
							
							if( FragIndex - FromFragment > 0 ) 
							{
								if(pbi->display_fragments[FragIndex] && (Mode2Frame[pbi->FragCodingMethod[FragIndex]] == WhichFrame))
								{
									pbi->QFragData[i][0] += pbi->QFragData[FragIndex][0];
									FragIndex = 0;
									break;
								}
							}
						}
						
						
						// if none matched fall back to the last one ever
						if(FragIndex)
						{
							pbi->QFragData[i][0] += Last[WhichFrame];
						}
						
					}
					else
					{
						
						// don't do divide if divisor is 1 or 0
						PredictedDC = (pc[wpc][0]*vul + pc[wpc][1] * vu + pc[wpc][2] * vur + pc[wpc][3] * vl );

						// if we need to do a shift
						if(pc[wpc][4] != 0 )
						{
							
							// If negative add in the negative correction factor
							PredictedDC += (HIGHBITDUPPED(PredictedDC) & pc[wpc][5]);
							
							// Shift in lieu of a divide
							PredictedDC >>= pc[wpc][4];
						}
						
                        // check for outranging on the two predictors that can outrange 
                        switch(wpc)
                        {
                        case 13: // pul pu pl
                        case 15: // pul pu pur pl
                            if( abs(PredictedDC - vu) > 128)
                                PredictedDC = vu;
                            else if( abs(PredictedDC - vl) > 128)
                                PredictedDC = vl;
                            else if( abs(PredictedDC - vul) > 128)
                                PredictedDC = vul;

                            break;
                        }
						
   						pbi->QFragData[i][0] += PredictedDC;
						
					}
					
					// Save the last fragment coded for whatever frame we are predicting from
					Last[WhichFrame] = pbi->QFragData[i][0];

					// Inverse DCT and reconstitute buffer in thisframe
					ExpandBlockA( pbi, i );

				} // if display fragments

			} // for n = 0 to columns across

		} // for m = 0 to rows down

	} // for j = 0 to 2 (y,u,v)


    // Copy the current reconstruction back to the last frame recon buffer.

//printf("%d\n",pbi->CodedBlockIndex);
	if(pbi->CodedBlockIndex > (INT32) (pbi->UnitFragments >> 1))
	{
		SwapReconBuffersTemp = pbi->ThisFrameRecon;
		pbi->ThisFrameRecon = pbi->LastFrameRecon;
		pbi->LastFrameRecon = SwapReconBuffersTemp;
		CopyNotRecon( pbi, pbi->LastFrameRecon, pbi->ThisFrameRecon );
	}
	else
    {
		CopyRecon( pbi, pbi->LastFrameRecon, pbi->ThisFrameRecon );
   }
        
    // We may need to update the UMV border
    //UpdateUMVBorder(pbi, pbi->LastFrameRecon );
    
    // Apply a loop filter to edge pixels of updated blocks	
//	if( pbi->Vp3VersionNo == 0 )
//		ApplyReconLoopFilter(pbi);
//    else
	     LoopFilter(pbi);

		if ( GetFrameType(pbi) == BASE_FRAME )
		{
			CopyRecon( pbi, pbi->GoldenFrame, pbi->LastFrameRecon );
		}

	

    // Reconstruct the golden frame if necessary. 
    // For VFW codec only on key frames

    // If appropriate clear the MMX state.
//    pbi->ClearSysState();

}

/****************************************************************************
 * 
 *  ROUTINE       :     ExpandKFBlock
 *
 *  INPUTS        :     INT32	FragIndex 
 *                      
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Reverses quantisation and dc and reconstructs pixel 
 *                      data for an "Intra" frame block.
 *
 *  SPECIAL NOTES :     
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ExpandKFBlock ( PB_INSTANCE *pbi, INT32 FragmentNumber )
{
    UINT32		ReconPixelsPerLine;	// Pixels per line
    INT32       ReconPixelIndex;    // Offset for block into a reconstruction buffer
	//INT32       i;

    // Select the appropriate inverse Q matrix and line stride
	if ( FragmentNumber<(INT32)pbi->YPlaneFragments )
	{
		ReconPixelsPerLine = pbi->Configuration.YStride;
		select_Y_dequantiser(pbi);
	}
	else
	{
		ReconPixelsPerLine = pbi->Configuration.UVStride;
		select_UV_dequantiser(pbi);
    }
    
    // Set up pointer into the quantisation buffer.
    pbi->quantized_list = (Q_LIST_ENTRY *)(&pbi->QFragData[FragmentNumber][0]);

    // Invert quantisation and DCT to get pixel data.                       
    pbi->idct[pbi->FragCoefEOB[FragmentNumber]]( pbi->quantized_list, pbi->dequant_coeffs, pbi->ReconDataBuffer );

    // Convert fragment number to a pixel offset in a reconstruction buffer.
    ReconPixelIndex = ReconGetFragIndex ( pbi->recon_pixel_index_table, FragmentNumber );

    // Get the pixel index for the first pixel in the fragment.
    pbi->ReconIntra( pbi, (UINT8 *)(&pbi->ThisFrameRecon[ReconPixelIndex]), (UINT16 *)pbi->ReconDataBuffer, ReconPixelsPerLine );

}

/****************************************************************************
 * 
 *  ROUTINE       :     ExpandBlock
 *
 *  INPUTS        :     INT32	FragIndex 
 *                      
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Reverses quantisation and dc and reconstructs pixel data.
 *
 *  SPECIAL NOTES :     
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ExpandBlock ( PB_INSTANCE *pbi, INT32 FragmentNumber )
{
    UINT8       *LastFrameRecPtr;   // Pointer into previous frame reconstruction.
    UINT8       *LastFrameRecPtr2;  // Pointer into previous frame reconstruction for 1/2 pixel MC.
	
    UINT32		ReconPixelsPerLine;	// Pixels per line
    INT32       ReconPixelIndex;    // Offset for block into a reconstruction buffer
    INT32       ReconPtr2Offset;    // Offset for second reconstruction in half pixel MC

    INT32       MVOffset;           // Baseline motion vector offset
    INT32       MvShift  ;          // Shift to correct to 1/2 or 1/4 pixel
    INT32       MvModMask;          // Mask to determine whether 1/2 pixel is used

    // TEST
    //INT32       mmask;
    //INT32       ReconPtr2OffsetB;    // Offset for second reconstruction in half pixel MC
    //INT32       MVOffsetB;

	// Get coding mode for this block
    if ( GetFrameType(pbi) == BASE_FRAME )
	{
		pbi->CodingMode = CODE_INTRA;
	}
	else
	{
	    // Get Motion vector and mode for this block.
        pbi->CodingMode = pbi->FragCodingMethod[FragmentNumber];
	}

    // Select the appropriate inverse Q matrix and line stride
	if ( FragmentNumber<(INT32)pbi->YPlaneFragments )
	{
		ReconPixelsPerLine = pbi->Configuration.YStride;
        MvShift = 1;
        MvModMask = 0x00000001;

        //mmask = 1;

		// Select appropriate dequantiser matrix.
        if ( pbi->CodingMode == CODE_INTRA )
		    select_Y_dequantiser(pbi);
        else
            select_Inter_dequantiser(pbi);
	}
	else
	{
		ReconPixelsPerLine = pbi->Configuration.UVStride;
        MvShift = 2;
        MvModMask = 0x00000003;

        //mmask = 3;

		// Select appropriate dequantiser matrix.
        if ( pbi->CodingMode == CODE_INTRA )
		    select_UV_dequantiser(pbi);
        else
            select_Inter_dequantiser(pbi);
    }
    
    // Set up pointer into the quantisation buffer.
    pbi->quantized_list = (Q_LIST_ENTRY *)(&pbi->QFragData[FragmentNumber][0]);

    // Invert quantisation and DCT to get pixel data.                       
    pbi->idct[pbi->FragCoefEOB[FragmentNumber]]( pbi->quantized_list, pbi->dequant_coeffs, pbi->ReconDataBuffer );

    // Convert fragment number to a pixel offset in a reconstruction buffer.
    ReconPixelIndex = ReconGetFragIndex ( pbi->recon_pixel_index_table, FragmentNumber );

    // Action depends on decode mode.
	if ( pbi->CodingMode == CODE_INTER_NO_MV )       // Inter with no motion vector
    {
		// Reconstruct the pixel data using the last frame reconstruction and change data
        // when the motion vector is (0,0), the recon is based on the lastframe without
        // loop filtering---- for testing
        pbi->ReconInter( pbi, (UINT8 *)&pbi->ThisFrameRecon[ReconPixelIndex], 
            (UINT8 *)&pbi->LastFrameRecon[ReconPixelIndex], 
            pbi->ReconDataBuffer, ReconPixelsPerLine );
            
    }
	else if ( ModeUsesMC[pbi->CodingMode] )          // The mode uses a motion vector.
	{
        {
            // Get vector from list
            pbi->MVector.x = pbi->FragMVect[FragmentNumber].x;
            pbi->MVector.y = pbi->FragMVect[FragmentNumber].y;
            
            // Work out the base motion vector offset and the 1/2 pixel offset if any.
            // For the U and V planes the MV specifies 1/4 pixel accuracy. This is adjusted to
            // 1/2 pixel as follows ( 0->0, 1/4->1/2, 1/2->1/2, 3/4->1/2 ).
            MVOffset = 0;
            ReconPtr2Offset = 0;
            if ( pbi->MVector.x > 0 )
            {
                MVOffset = pbi->MVector.x >> MvShift;
                if ( pbi->MVector.x & MvModMask )
                    ReconPtr2Offset += 1;
            }
            else if ( pbi->MVector.x < 0 )
            {
                MVOffset -= (-pbi->MVector.x) >> MvShift;
                if ( (-pbi->MVector.x) & MvModMask )
                    ReconPtr2Offset -= 1;
            }
            if ( pbi->MVector.y > 0 )
            {
                MVOffset += (pbi->MVector.y >>  MvShift) * ReconPixelsPerLine;
                if ( pbi->MVector.y & MvModMask )
                    ReconPtr2Offset += ReconPixelsPerLine;
            }
            else if ( pbi->MVector.y < 0 )
            {
                MVOffset -= ((-pbi->MVector.y) >> MvShift) * ReconPixelsPerLine;
                if ( (-pbi->MVector.y) & MvModMask )
                    ReconPtr2Offset -= ReconPixelsPerLine;
            }
            
            // Set up the first of the two reconstruction buffer pointers.
            if ( pbi->CodingMode==CODE_GOLDEN_MV ) 
            {
                LastFrameRecPtr = (UINT8 * )( &pbi->GoldenFrame[ReconPixelIndex] + MVOffset);
            }
            else
            {
                LastFrameRecPtr = (UINT8 * )( &pbi->LastFrameRecon[ReconPixelIndex] + MVOffset);
            }
            
            // Set up the second of the two reconstruction pointers.
            LastFrameRecPtr2 = LastFrameRecPtr + ReconPtr2Offset;
            
            // Select the appropriate reconstruction function
            if ( (int)(LastFrameRecPtr - LastFrameRecPtr2) == 0 ) 
            {
                // Reconstruct the pixel dats from the reference frame and change data
                // (no half pixel in this case as the two references were the same.
                pbi->ReconInter( pbi, (UINT8 *)&pbi->ThisFrameRecon[ReconPixelIndex], 
                    LastFrameRecPtr, pbi->ReconDataBuffer, ReconPixelsPerLine );
            }
            // Fractional pixel reconstruction.
            // Note that we only use two pixels per reconstruction even for the diagonal.
            else 
            {
                pbi->ReconInterHalfPixel2( pbi, (UINT8 *)&pbi->ThisFrameRecon[ReconPixelIndex], 
                    LastFrameRecPtr, LastFrameRecPtr2, pbi->ReconDataBuffer, ReconPixelsPerLine );
            }
        }
    }
	else if ( pbi->CodingMode == CODE_USING_GOLDEN )     // Golden frame with motion vector
    {
		// Reconstruct the pixel data using the golden frame reconstruction and change data
        pbi->ReconInter( pbi, (UINT8 *)&pbi->ThisFrameRecon[ReconPixelIndex], 
                    (UINT8 *)&pbi->GoldenFrame[ ReconPixelIndex ], 
                    pbi->ReconDataBuffer, ReconPixelsPerLine );
    }
	else                                            // Simple Intra coding
	{
        // Get the pixel index for the first pixel in the fragment.
        pbi->ReconIntra( pbi, (UINT8 *)&pbi->ThisFrameRecon[ReconPixelIndex], (UINT16 *)pbi->ReconDataBuffer, ReconPixelsPerLine );
	}
}


/****************************************************************************
 * 
 *  ROUTINE       :     ExpandToken
 *
 *  INPUTS        :     Q_LIST_ENTRY * ExpandedBlock
 *                                     Pointer to block structure into which the token
 *                                     should be expanded.
 *
 *                      INT32 * CoeffIndex
 *                             Where we are in the current block.
 *
 *                      INT32 Token, ExtraBits
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Expands a DCT token into a data structure.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ExpandToken( PB_INSTANCE *pbi, Q_LIST_ENTRY * ExpandedBlock, UINT8 * CoeffIndex, UINT32 Token, INT32 ExtraBits )
{
    // Is the token is a combination run and value token. 
    if ( Token >= DCT_RUN_CATEGORY1 )
    {
        // Expand the token and additional bits to a zero run length and data value. 
        if ( Token < DCT_RUN_CATEGORY2 )
        {   
            // Decoding method depends on token
            if ( Token < DCT_RUN_CATEGORY1B )
            {
                // Step on by the zero run length
                *CoeffIndex += (UINT8)((Token - DCT_RUN_CATEGORY1) + 1);

                // The extra bit determines the sign.
                if ( ExtraBits & 0x01 )
                    ExpandedBlock[*CoeffIndex] = -1;
                else
                    ExpandedBlock[*CoeffIndex] = 1;
            }
            else if ( Token == DCT_RUN_CATEGORY1B )
            {
                // Bits 0-1 determines the zero run length
                *CoeffIndex += (6 + (ExtraBits & 0x03));

                // Bit 2 determines the sign
                if ( ExtraBits & 0x04 )
                    ExpandedBlock[*CoeffIndex] = -1;
                else
                    ExpandedBlock[*CoeffIndex] = 1;
            }
            else
            {
                // Bits 0-2 determines the zero run length
                *CoeffIndex += (10 + (ExtraBits & 0x07));

                // Bit 3 determines the sign
                if ( ExtraBits & 0x08 )
                    ExpandedBlock[*CoeffIndex] = -1;
                else
                    ExpandedBlock[*CoeffIndex] = 1;
            }
        }   
        else 
        {   
            // If token == DCT_RUN_CATEGORY2 we have a single 0 followed by a value
            if ( Token == DCT_RUN_CATEGORY2 )
            {
                // Step on by the zero run length
                *CoeffIndex += 1;

                // Bit 1 determines sign, bit 0 the value
                if ( ExtraBits & 0x02 )
                    ExpandedBlock[*CoeffIndex] = -(2 + (ExtraBits & 0x01));
                else
                    ExpandedBlock[*CoeffIndex] = 2 + (ExtraBits & 0x01);
            }
            // else we have 2->3 zeros followed by a value
            else
            {
                // Bit 0 determines the zero run length
                *CoeffIndex += 2 + (ExtraBits & 0x01);

                // Bit 2 determines the sign, bit 1 the value
                if ( ExtraBits & 0x04 )
                    ExpandedBlock[*CoeffIndex] = -(2 + ((ExtraBits & 0x02) >> 1));
                else
                    ExpandedBlock[*CoeffIndex] = 2 + ((ExtraBits & 0x02) >> 1);
            }
        }

        // Step on over value
        *CoeffIndex += 1;

    }
    // Token is a ZRL token so step on by the appropriate number of zeros
    else if ( Token == DCT_SHORT_ZRL_TOKEN )
    {
        *CoeffIndex += ExtraBits + 1;
    }
    // Token is a ZRL token so step on by the appropriate number of zeros
    else if ( Token == DCT_ZRL_TOKEN )
    {
        *CoeffIndex += ExtraBits + 1;
    }
    // Token is a small single value token.
    else if ( Token < LOW_VAL_TOKENS )
    {
        switch ( Token )
        {
        case ONE_TOKEN:
            ExpandedBlock[*CoeffIndex] = 1;
            break;
        case MINUS_ONE_TOKEN:
            ExpandedBlock[*CoeffIndex] = -1;
            break;
        case TWO_TOKEN:
            ExpandedBlock[*CoeffIndex] = 2;
            break;
        case MINUS_TWO_TOKEN:
            ExpandedBlock[*CoeffIndex] = -2;
            break;
        }

        // Step on the coefficient index.
        *CoeffIndex += 1;
    }
    // Token is a larger single value token
    else 
    {
        // Expand the token and additional bits to a data value. 
        if ( Token < DCT_VAL_CATEGORY3 )
        {   
            // Offset from LOW_VAL_TOKENS determines value
            Token = Token - LOW_VAL_TOKENS;

            // Extra bit determines sign
            if ( ExtraBits )
                ExpandedBlock[*CoeffIndex] = -((Q_LIST_ENTRY)(Token + DCT_VAL_CAT2_MIN));
            else
                ExpandedBlock[*CoeffIndex] = (Q_LIST_ENTRY)(Token + DCT_VAL_CAT2_MIN);
        }
        else if ( Token == DCT_VAL_CATEGORY3 )
        {   
            // Bit 1 determines sign, Bit 0 the value
            if ( ExtraBits & 0x02 )
                ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT3_MIN + (ExtraBits & 0x01));
            else
                ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT3_MIN + (ExtraBits & 0x01);
        }
        else if ( Token == DCT_VAL_CATEGORY4 )
        {   
            // Bit 2 determines sign, Bit 0-1 the value
            if ( ExtraBits & 0x04 )
                ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT4_MIN + (ExtraBits & 0x03));
            else
                ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT4_MIN + (ExtraBits & 0x03);
        }
        else if ( Token == DCT_VAL_CATEGORY5 )
        {
            // Bit 3 determines sign, Bit 0-2 the value
            if ( ExtraBits & 0x08 )
                ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT5_MIN + (ExtraBits & 0x07));
            else
                ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT5_MIN + (ExtraBits & 0x07);
        }
        else if ( Token == DCT_VAL_CATEGORY6 )
        {
            // Bit 4 determines sign, Bit 0-3 the value
            if ( ExtraBits & 0x10 )
                ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT6_MIN + (ExtraBits & 0x0F));
            else
                ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT6_MIN + (ExtraBits & 0x0F);
        }
        else if ( Token == DCT_VAL_CATEGORY7 )
        {
            // Bit 5 determines sign, Bit 0-4 the value
            if ( ExtraBits & 0x20 )
                ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT7_MIN + (ExtraBits & 0x1F));
            else
                ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT7_MIN + (ExtraBits & 0x1F);
        }
        else if ( Token == DCT_VAL_CATEGORY8 )
        {
            // Bit 9 determines sign, Bit 0-8 the value
            if ( ExtraBits & 0x200 )
                ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT8_MIN + (ExtraBits & 0x1FF));
            else
                ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT8_MIN + (ExtraBits & 0x1FF);
        }

        // Step on the coefficient index.
        *CoeffIndex += 1;
    }


}

/****************************************************************************
 * 
 *  ROUTINE       :     CopyRecon
 *
 *  INPUTS        :     UINT8 * DestReconPtr
 *                      UINT8 * SrcReconPtr
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Copies the current reconstruction to the last frame
 *                      reconstruction buffer.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void CopyRecon( PB_INSTANCE *pbi, UINT8 * DestReconPtr, UINT8 * SrcReconPtr )
{
    UINT32  i;
    UINT32	PlaneLineStep;		// Pixels per line
    UINT32  PixelIndex;

	UINT8  *SrcPtr;	            // Pointer to line of source image data
	UINT8  *DestPtr;            // Pointer to line of destination image data

    //static Temp = 0;

    // Copy over only updated blocks.

    // First Y plane
    PlaneLineStep = pbi->Configuration.YStride;
    for ( i = 0; i < pbi->YPlaneFragments; i++ )
    {
        if ( pbi->display_fragments[i] )
        {
            PixelIndex = ReconGetFragIndex ( pbi->recon_pixel_index_table, i );
            SrcPtr = &SrcReconPtr[ PixelIndex ];
            DestPtr = &DestReconPtr[ PixelIndex ];

			pbi->CopyBlock(SrcPtr, DestPtr, PlaneLineStep);
        }
    }

    // Then U and V
    PlaneLineStep = pbi->Configuration.UVStride;
    for ( i = pbi->YPlaneFragments; i < pbi->UnitFragments; i++ )
    {
        if ( pbi->display_fragments[i] )
        {
            PixelIndex = ReconGetFragIndex ( pbi->recon_pixel_index_table, i );
            SrcPtr = &SrcReconPtr[ PixelIndex ];
            DestPtr = &DestReconPtr[ PixelIndex ];

			pbi->CopyBlock(SrcPtr, DestPtr, PlaneLineStep);
			
        }
    }

    // We may need to update the UMV border
    UpdateUMVBorder(pbi, DestReconPtr);

}
/****************************************************************************
 * 
 *  ROUTINE       :     CopyRecon
 *
 *  INPUTS        :     UINT8 * DestReconPtr
 *                      UINT8 * SrcReconPtr
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Copies the current reconstruction to the last frame
 *                      reconstruction buffer.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void CopyNotRecon( PB_INSTANCE *pbi, UINT8 * DestReconPtr, UINT8 * SrcReconPtr )
{
    UINT32  i;
    UINT32	PlaneLineStep;		// Pixels per line
    UINT32  PixelIndex;

	UINT8  *SrcPtr;	            // Pointer to line of source image data
	UINT8  *DestPtr;            // Pointer to line of destination image data

    //static Temp = 0;

    // Copy over only updated blocks.

    // First Y plane
    PlaneLineStep = pbi->Configuration.YStride;
    for ( i = 0; i < pbi->YPlaneFragments; i++ )
    {
        if ( !pbi->display_fragments[i] )
        {
            PixelIndex = ReconGetFragIndex ( pbi->recon_pixel_index_table, i );
            SrcPtr = &SrcReconPtr[ PixelIndex ];
            DestPtr = &DestReconPtr[ PixelIndex ];

			pbi->CopyBlock(SrcPtr, DestPtr, PlaneLineStep);
        }
    }

    // Then U and V
    PlaneLineStep = pbi->Configuration.UVStride;
    for ( i = pbi->YPlaneFragments; i < pbi->UnitFragments; i++ )
    {
        if ( !pbi->display_fragments[i] )
        {
            PixelIndex = ReconGetFragIndex ( pbi->recon_pixel_index_table, i );
            SrcPtr = &SrcReconPtr[ PixelIndex ];
            DestPtr = &DestReconPtr[ PixelIndex ];

			pbi->CopyBlock(SrcPtr, DestPtr, PlaneLineStep);
			
        }
    }

    // We may need to update the UMV border
    UpdateUMVBorder(pbi, DestReconPtr);

}

/****************************************************************************
 * 
 *  ROUTINE       :     UpdateUMVBorder
 *
 *  INPUTS        :     UINT8 * DestReconPtr
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Updates the UMV border for the given reconstruction 
 *                      buffer.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UpdateUMVBorder( PB_INSTANCE *pbi, UINT8 * DestReconPtr )
{
    UINT32  PlaneFragOffset;

    // Y plane
    PlaneFragOffset = 0;
    UpdateUMV_VBorders( pbi, DestReconPtr, PlaneFragOffset );
    UpdateUMV_HBorders( pbi, DestReconPtr, PlaneFragOffset );

    // Then the U and V Planes
    PlaneFragOffset = pbi->YPlaneFragments;
    UpdateUMV_VBorders( pbi, DestReconPtr, PlaneFragOffset );
    UpdateUMV_HBorders( pbi, DestReconPtr, PlaneFragOffset );

    PlaneFragOffset = pbi->YPlaneFragments + pbi->UVPlaneFragments;   
    UpdateUMV_VBorders( pbi, DestReconPtr, PlaneFragOffset );
    UpdateUMV_HBorders( pbi, DestReconPtr, PlaneFragOffset );
}


/****************************************************************************
 * 
 *  ROUTINE       :     UpdateUMV_HBorders
 *
 *  INPUTS        :     UINT8 * DestReconPtr
 *                      UINT32  PlaneFragOffset
 *                      
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Fills in the Horizontal UMV borders for the given 
 *                      reconstruction buffer.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UpdateUMV_HBorders( PB_INSTANCE *pbi, UINT8 * DestReconPtr, UINT32  PlaneFragOffset )
{
    UINT32  i;
    UINT32  PixelIndex;

    UINT32  PlaneStride;
    UINT32  BlockVStep;
    UINT32  PlaneFragments;
    UINT32  LineFragments;
    UINT32  PlaneBorderWidth;

	UINT8   *SrcPtr1;            // Pointer into destination buffer
	UINT8   *SrcPtr2;            // Pointer into destination buffer
	UINT8   *DestPtr1;           // Pointer into destination buffer
	UINT8   *DestPtr2;           // Pointer into destination buffer

    // Work out various plane specific values
    if ( PlaneFragOffset == 0 )
    {
        // Y Plane
        BlockVStep = (pbi->Configuration.YStride * (pbi->Configuration.VFragPixels - 1));
        PlaneStride = pbi->Configuration.YStride;
        PlaneBorderWidth = UMV_BORDER;
        PlaneFragments = pbi->YPlaneFragments;
        LineFragments = pbi->HFragments;
    }
    else
    {
        // U or V plane.
        BlockVStep = (pbi->Configuration.UVStride * (pbi->Configuration.VFragPixels - 1));
        PlaneStride = pbi->Configuration.UVStride;
        PlaneBorderWidth = UMV_BORDER / 2;
        PlaneFragments = pbi->UVPlaneFragments;
        LineFragments = pbi->HFragments / 2;
    }

    // Setup the source and destination pointers for the top and bottom borders
    PixelIndex = ReconGetFragIndex ( pbi->recon_pixel_index_table, PlaneFragOffset );
    SrcPtr1 = &DestReconPtr[ PixelIndex - PlaneBorderWidth ];
    DestPtr1 = SrcPtr1 - (PlaneBorderWidth * PlaneStride);

    PixelIndex = ReconGetFragIndex ( pbi->recon_pixel_index_table, PlaneFragOffset + PlaneFragments - LineFragments ) + BlockVStep;
    SrcPtr2 = &DestReconPtr[ PixelIndex - PlaneBorderWidth];
    DestPtr2 = SrcPtr2 + PlaneStride;

    // Now copy the top and bottom source lines into each line of the respective borders
    for ( i = 0; i < PlaneBorderWidth; i++ )
    {
        memcpy( DestPtr1, SrcPtr1, PlaneStride );
        memcpy( DestPtr2, SrcPtr2, PlaneStride );
        DestPtr1 += PlaneStride;
        DestPtr2 += PlaneStride;
    }
}


/****************************************************************************
 * 
 *  ROUTINE       :     UpdateUMV_VBorders
 *
 *  INPUTS        :     UINT8 * DestReconPtr
 *                      UINT32  PlaneFragOffset
 *                      
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Fills in the vertical UMV borders for the given 
 *                      reconstruction buffer.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UpdateUMV_VBorders( PB_INSTANCE *pbi, UINT8 * DestReconPtr, UINT32  PlaneFragOffset )
{
    UINT32  i;
    UINT32  PixelIndex;

    UINT32  PlaneStride;
    UINT32  LineFragments;
    UINT32  PlaneBorderWidth;
    UINT32  PlaneHeight;

	UINT8   *SrcPtr1;            // Pointer into destination buffer
	UINT8   *SrcPtr2;            // Pointer into destination buffer
	UINT8   *DestPtr1;           // Pointer into destination buffer
	UINT8   *DestPtr2;           // Pointer into destination buffer

    // Work out various plane specific values
    if ( PlaneFragOffset == 0 )
    {
        // Y Plane
        PlaneStride = pbi->Configuration.YStride;
        PlaneBorderWidth = UMV_BORDER;
        LineFragments = pbi->HFragments;
        PlaneHeight = pbi->Configuration.VideoFrameHeight;
    }
    else
    {
        // U or V plane.
        PlaneStride = pbi->Configuration.UVStride;
        PlaneBorderWidth = UMV_BORDER / 2;
        LineFragments = pbi->HFragments / 2;
        PlaneHeight = pbi->Configuration.VideoFrameHeight / 2;
    }

    // Setup the source data values and destination pointers for the left and right edge borders
    PixelIndex = ReconGetFragIndex ( pbi->recon_pixel_index_table, PlaneFragOffset );
    SrcPtr1 = &DestReconPtr[ PixelIndex ];
    DestPtr1 = &DestReconPtr[ PixelIndex - PlaneBorderWidth ];

    PixelIndex = ReconGetFragIndex ( pbi->recon_pixel_index_table, PlaneFragOffset + LineFragments - 1 ) + (pbi->Configuration.HFragPixels - 1);
    SrcPtr2 = &DestReconPtr[ PixelIndex ];
    DestPtr2 = &DestReconPtr[ PixelIndex + 1 ];

    // Now copy the top and bottom source lines into each line of the respective borders
    for ( i = 0; i < PlaneHeight; i++ )
    {
        memset( DestPtr1, SrcPtr1[0], PlaneBorderWidth );
        memset( DestPtr2, SrcPtr2[0], PlaneBorderWidth );
        SrcPtr1 += PlaneStride;
        SrcPtr2 += PlaneStride;
        DestPtr1 += PlaneStride;
        DestPtr2 += PlaneStride;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     ClearDownQFragData
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Clears down the data structure that is used
 *                      to store quantised dct coefficients for each block.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ClearDownQFragData(PB_INSTANCE *pbi)
{
    INT32       i,j;
    UINT32 *    QFragPtr;
	
	for ( i = 0; i < pbi->CodedBlockIndex; i++ )
	{
		// Get the linear index for the current fragment.
		QFragPtr = (UINT32*)(&pbi->QFragData[pbi->CodedBlockList[i]]);
		for ( j = 0; j < 8; j++ )
		{
			QFragPtr[0]  = 0;
			QFragPtr[1]  = 0;
			QFragPtr[2]  = 0;
			QFragPtr[3]  = 0;
//			QFragPtr[4]  = 0;
//			QFragPtr[5]  = 0;
//			QFragPtr[6]  = 0;
//			QFragPtr[7]  = 0;
			QFragPtr += 4;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     FilterHoriz_Generic
 *
 *  INPUTS        :     None
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Applies a loop filter to the vertical edge horizontally
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void FilterHoriz_Generic(PB_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr)
{
	INT32 j;
	INT32 FiltVal;
    UINT8 * LimitTable = &LimitVal_VP31[VAL_RANGE];
	for ( j = 8; j ; j-- )
	{            
		FiltVal =  ( PixelPtr[0] ) - 
			( PixelPtr[1] * 3 ) +
			( PixelPtr[2] * 3 ) - 
			( PixelPtr[3] );
		
		FiltVal = BoundingValuePtr[(FiltVal + 4) >> 3];
		
		PixelPtr[1] = LimitTable[(INT32)PixelPtr[1] + FiltVal];
		PixelPtr[2] = LimitTable[(INT32)PixelPtr[2] - FiltVal];
		
		PixelPtr += LineLength;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     FilterVert_Generic
 *
 *  INPUTS        :     None
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Applies a loop filter to a horizontal edge vertically
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void FilterVert_Generic(PB_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr)
{
	INT32 j;
	INT32 FiltVal;
    UINT8 * LimitTable = &LimitVal_VP31[VAL_RANGE];
	for ( j = 8; j ; j-- )
	{            
		FiltVal = ( (INT32)PixelPtr[- (2 * LineLength)] ) - 
			( (INT32)PixelPtr[- LineLength] * 3 ) + 
			( (INT32)PixelPtr[0] * 3 ) - 
			( (INT32)PixelPtr[LineLength] );
		
		FiltVal = BoundingValuePtr[(FiltVal + 4) >> 3];
		PixelPtr[- LineLength] = LimitTable[(INT32)PixelPtr[- LineLength] + FiltVal];
		PixelPtr[0] = LimitTable[(INT32)PixelPtr[0] - FiltVal];
		PixelPtr ++;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     SetupBoundingValueArray
 *
 *  INPUTS        :     None
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Set up the bounding value array.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
INT32 *SetupBoundingValueArray_Generic(PB_INSTANCE *pbi, INT32 FLimit)
{
    INT32 * BoundingValuePtr;
    INT32 i;
	INT32* t=(INT32*)pbi->FiltBoundingValue;
	int count=(512*sizeof(*pbi->FiltBoundingValue))/4;
    BoundingValuePtr = &pbi->FiltBoundingValue[256];
    // Set up the bounding value array.
	for(i=0;i<count;i++)
	{
		t[0]=0;
		t++;
	}
//    memset ( pbi->FiltBoundingValue, 0, (512*sizeof(*pbi->FiltBoundingValue)) );
    for ( i = 0; i < FLimit; i++ )
    {
        BoundingValuePtr[-i-FLimit] = (-FLimit+i);
        BoundingValuePtr[-i] = -i;
        BoundingValuePtr[i] = i;
        BoundingValuePtr[i+FLimit] = FLimit-i;
    }

    return BoundingValuePtr;
}

/****************************************************************************
 * 
 *  ROUTINE       :     LoopFilter
 *
 *  INPUTS        :     None
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Applies a loop filter to the edge pixels of coded blocks.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void LoopFilter(PB_INSTANCE *pbi)
{
    INT32 i;

	unsigned long uvHFragments = pbi->HFragments >> 1;
    INT32 * BoundingValuePtr;
    UINT8 * LimitTable = &LimitVal_VP31[VAL_RANGE];
	int FragsAcross=pbi->HFragments;	
	int FromFragment,ToFragment;
	int FragsDown = pbi->VFragments;
    INT32 LineFragments;
    INT32 LineLength;
    UINT8 BlockHeight = (UINT8)pbi->Configuration.VFragPixels;
    UINT8 BlockWidth = (UINT8)pbi->Configuration.HFragPixels;
    INT32 FLimit; 
    INT32 QIndex;
	int j,m,n;

	// Set the limit value for the loop filter based upon the current quantizer.
    QIndex = Q_TABLE_SIZE - 1;
    while ( QIndex >= 0 )
    {
        if ( (QIndex == 0) || ( pbi->QThreshTable[QIndex] >= pbi->ThisFrameQualityValue) )
            break;
        QIndex --;
    }

	// Encoder version specific clause
	FLimit = LoopFilterLimitValuesV1[QIndex];

    if ( FLimit == 0 )
        return;
        
    BoundingValuePtr = pbi->SetupBoundingValueArray(pbi, FLimit);
 

	for ( j = 0; j < 3 ; j++)
	{
		switch(j)
		{
		case 0: // y
			FromFragment = 0;
			ToFragment = pbi->YPlaneFragments;
			FragsAcross = pbi->HFragments;
			FragsDown = pbi->VFragments;
			LineLength = pbi->Configuration.YStride;
			LineFragments = pbi->HFragments;
			break;
		case 1: // u
			FromFragment = pbi->YPlaneFragments;
			ToFragment = pbi->YPlaneFragments + pbi->UVPlaneFragments ;
			FragsAcross = pbi->HFragments >> 1;
			FragsDown = pbi->VFragments >> 1;
			LineLength = pbi->Configuration.UVStride;
			LineFragments = pbi->HFragments / 2;
			break;
		case 2:	// v
			FromFragment = pbi->YPlaneFragments + pbi->UVPlaneFragments;
			ToFragment = pbi->YPlaneFragments + (2 * pbi->UVPlaneFragments) ;
			FragsAcross = pbi->HFragments >> 1;
			FragsDown = pbi->VFragments >> 1;
			LineLength = pbi->Configuration.UVStride;
			LineFragments = pbi->HFragments / 2;
			break;
		}

		i=FromFragment;

		//****************************************************************************************************************
		// First Row 
		//****************************************************************************************************************

		//******************************************************************
		// first column conditions
		
		// only do 2 prediction if fragment coded and on non intra or if all fragments are intra 
		if( pbi->display_fragments[i])
		{
			
			// Filter right hand border only if the block to the right is not coded
			if ( !pbi->display_fragments[ i + 1 ] )
			{
				pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) + 6 ], LineLength, BoundingValuePtr);
			}
			
			// Bottom done if next row set
			if( !pbi->display_fragments[ i + LineFragments] )
			{
				pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i + LineFragments) ], LineLength, BoundingValuePtr);
			}
			
		} // if( pbi->display_fragments[i])
		
		i++; // next i
		
		//******************************************************************
		// middle columns 
		for ( n = 1 ; n < FragsAcross - 1 ; n++, i++)
		{
			if( pbi->display_fragments[i])
			{
				// Filter Left edge always
				pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) - 2 ], LineLength, BoundingValuePtr);
				
				// Filter right hand border only if the block to the right is not coded
				if ( !pbi->display_fragments[ i + 1 ] )
				{
					pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) + 6 ], LineLength, BoundingValuePtr);
				}
				
				// Bottom done if next row set
				if( !pbi->display_fragments[ i + LineFragments] )
				{
					pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i + LineFragments) ], LineLength, BoundingValuePtr);
				}
				
				
			} // if( pbi->display_fragments[i])
			
		} // for ( n = 1 ; n < FragsAcross - 1 ; n++, i++)
		
		//******************************************************************
		// Last Column
		if( pbi->display_fragments[i])
		{
			// Filter Left edge always
			pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) - 2 ], LineLength, BoundingValuePtr);
			
			// Bottom done if next row set
			if( !pbi->display_fragments[ i + LineFragments] )
			{
				pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i + LineFragments) ], LineLength, BoundingValuePtr);
			}
			
		} // if( pbi->display_fragments[i])
		
		i++; // next i

		//****************************************************************************************************************
		// Middle Rows
		//****************************************************************************************************************
		for ( m = 1 ; m < FragsDown-1 ; m++)
		{

			//******************************************************************
			// first column conditions
			
			// only do 2 prediction if fragment coded and on non intra or if all fragments are intra 
			if( pbi->display_fragments[i])
			{

				// TopRow is always done
				pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) ], LineLength, BoundingValuePtr);

				// Filter right hand border only if the block to the right is not coded
				if ( !pbi->display_fragments[ i + 1 ] )
				{
					pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) + 6 ], LineLength, BoundingValuePtr);
				}

				// Bottom done if next row set
				if( !pbi->display_fragments[ i + LineFragments] )
				{
					pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i + LineFragments) ], LineLength, BoundingValuePtr);
				}
				
			} // if( pbi->display_fragments[i])

			i++; // next i

			//******************************************************************
			// middle columns 
			for ( n = 1 ; n < FragsAcross - 1 ; n++, i++)
			{
				if( pbi->display_fragments[i])
				{
					// Filter Left edge always
					pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) - 2 ], LineLength, BoundingValuePtr);

					// TopRow is always done
					pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) ], LineLength, BoundingValuePtr);

					// Filter right hand border only if the block to the right is not coded
					if ( !pbi->display_fragments[ i + 1 ] )
					{
						pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) + 6 ], LineLength, BoundingValuePtr);
					}

					// Bottom done if next row set
					if( !pbi->display_fragments[ i + LineFragments] )
					{
						pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i + LineFragments) ], LineLength, BoundingValuePtr);
					}
					
					
				} // if( pbi->display_fragments[i])

			} // for ( n = 1 ; n < FragsAcross - 1 ; n++, i++)

			//******************************************************************
			// Last Column
			if( pbi->display_fragments[i])
			{
				// Filter Left edge always
				pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) - 2 ], LineLength, BoundingValuePtr);
				
				// TopRow is always done
				pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) ], LineLength, BoundingValuePtr);
				
				// Bottom done if next row set
				if( !pbi->display_fragments[ i + LineFragments] )
				{
					pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i + LineFragments) ], LineLength, BoundingValuePtr);
				}

			} // if( pbi->display_fragments[i])

			i++; // next i


		} // for ( m = 0 ; m < FragsDown ; m++)

		
		//****************************************************************************************************************
		// Last Row 
		//****************************************************************************************************************
		
		//******************************************************************
		// first column conditions
		
		// only do 2 prediction if fragment coded and on non intra or if all fragments are intra 
		if( pbi->display_fragments[i])
		{
			
			// TopRow is always done
			pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) ], LineLength, BoundingValuePtr);
			
			// Filter right hand border only if the block to the right is not coded
			if ( !pbi->display_fragments[ i + 1 ] )
			{
				pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) + 6 ], LineLength, BoundingValuePtr);
			}
			
		} // if( pbi->display_fragments[i])
		
		i++; // next i
		
		//******************************************************************
		// middle columns 
		for ( n = 1 ; n < FragsAcross - 1 ; n++, i++)
		{
			if( pbi->display_fragments[i])
			{
				// Filter Left edge always
				pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) - 2 ], LineLength, BoundingValuePtr);
				
				// TopRow is always done
				pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) ], LineLength, BoundingValuePtr);
				
				// Filter right hand border only if the block to the right is not coded
				if ( !pbi->display_fragments[ i + 1 ] )
				{
					pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) + 6 ], LineLength, BoundingValuePtr);
				}
				
			} // if( pbi->display_fragments[i])
			
		} // for ( n = 1 ; n < FragsAcross - 1 ; n++, i++)
		
		//******************************************************************
		// Last Column
		if( pbi->display_fragments[i])
		{
			// Filter Left edge always
			pbi->FilterHoriz(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) - 2 ], LineLength, BoundingValuePtr);
			
			// TopRow is always done
			pbi->FilterVert(pbi, &pbi->LastFrameRecon[ ReconGetFragIndex(pbi->recon_pixel_index_table, i) ], LineLength, BoundingValuePtr);
			
		} // if( pbi->display_fragments[i])
		
		i++; // next i

	} // for ( j = 0; j < 3 ; j++)

}





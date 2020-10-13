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
*   Module Title :     unpack.C
*
*   Description  :     Video CODEC: Top level decode module.
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

//#include "BlockMapping.h"
#include "pbdll.h"
#include "codec_common_interface.h"
#include <string.h>


/****************************************************************************
*  Module constants.
*****************************************************************************
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


/****************************************************************************
*  Imports
*****************************************************************************
*/  


/****************************************************************************
 * 
 *  ROUTINE       :     UnPackVideo
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Upacks and decodes the video stream.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UnPackVideo (PB_INSTANCE *pbi)
{
    INT32       EncodedCoeffs = 1;
    INT32       FragIndex;
    INT32 *     CodedBlockListPtr;
    INT32 *     CodedBlockListEnd;

    UINT8       AcHuffIndex1;
    UINT8       AcHuffIndex2;
    UINT8       AcHuffChoice1;
    UINT8       AcHuffChoice2;
    
    UINT8       DcHuffChoice1;
    UINT8       DcHuffChoice2;


    // Bail out immediately if a decode error has already been reported.
//    if ( pbi->DecoderErrorCode != NO_DECODER_ERROR )
//        return;

    // Clear down the array that indicates the current coefficient index for each block.
    memset(pbi->FragCoeffs, 0, pbi->UnitFragments);
    memset(pbi->FragCoefEOB, 0, pbi->UnitFragments);

    // Clear down the pbi->QFragData structure for all coded blocks.
    pbi->ClearDownQFrag(pbi);

    // Note the number of blocks to decode
    pbi->BlocksToDecode = pbi->CodedBlockIndex;

    // Get the DC huffman table choice for Y and then UV
    DcHuffChoice1 = (UINT8)bitread( &pbi->br,  DC_HUFF_CHOICE_BITS ) + DC_HUFF_OFFSET;
    DcHuffChoice2 = (UINT8)bitread( &pbi->br,  DC_HUFF_CHOICE_BITS ) + DC_HUFF_OFFSET;

    // UnPack DC coefficients / tokens
    CodedBlockListPtr = pbi->CodedBlockList;
    CodedBlockListEnd = &pbi->CodedBlockList[pbi->CodedBlockIndex];
    while ( CodedBlockListPtr < CodedBlockListEnd  )
    {
        // Get the block data index
        FragIndex = *CodedBlockListPtr;
		pbi->FragCoefEOB[FragIndex] = pbi->FragCoeffs[FragIndex];

        // Select the appropriate huffman table offset according to whether the token is fro am Y or UV block
        if ( FragIndex < (INT32)pbi->YPlaneFragments )
            pbi->DcHuffChoice = DcHuffChoice1;
        else
            pbi->DcHuffChoice = DcHuffChoice2;

        // If we are in the middle of an EOB run
        if ( pbi->EOB_Run )
        {
            // Mark the current block as fully expanded and decrement EOB_RUN count
            pbi->FragCoeffs[FragIndex] = BLOCK_SIZE;
            pbi->EOB_Run --;
            pbi->BlocksToDecode --;
        }
        // Else unpack a DC token
        else
        {
            UnpackAndExpandDcToken( pbi, pbi->QFragData[FragIndex], &pbi->FragCoeffs[FragIndex] );
        }
        CodedBlockListPtr++;
    }

    // Get the AC huffman table choice for Y and then for UV.
    AcHuffIndex1 = (UINT8) bitread( &pbi->br,  AC_HUFF_CHOICE_BITS ) + AC_HUFF_OFFSET;
    AcHuffIndex2 = (unsigned char) bitread( &pbi->br,  AC_HUFF_CHOICE_BITS ) + AC_HUFF_OFFSET;

    // Unpack Lower AC coefficients.
    while ( EncodedCoeffs < 64 ) 
    {
        // Repeatedly scan through the list of blocks.
        CodedBlockListPtr = pbi->CodedBlockList;
        CodedBlockListEnd = &pbi->CodedBlockList[pbi->CodedBlockIndex];

        // Huffman table selection based upon which AC coefficient we are on
        if ( EncodedCoeffs <= AC_TABLE_2_THRESH )
        {
            AcHuffChoice1 = AcHuffIndex1;
            AcHuffChoice2 = AcHuffIndex2;
        }
        else if ( EncodedCoeffs <= AC_TABLE_3_THRESH )
        {
            AcHuffChoice1 = AcHuffIndex1 + AC_HUFF_CHOICES;
            AcHuffChoice2 = AcHuffIndex2 + AC_HUFF_CHOICES;
        }
        else if ( EncodedCoeffs <= AC_TABLE_4_THRESH )
        {
            AcHuffChoice1 = AcHuffIndex1 + (AC_HUFF_CHOICES * 2);
            AcHuffChoice2 = AcHuffIndex2 + (AC_HUFF_CHOICES * 2);
        }
        else
        {
            AcHuffChoice1 = AcHuffIndex1 + (AC_HUFF_CHOICES * 3);
            AcHuffChoice2 = AcHuffIndex2 + (AC_HUFF_CHOICES * 3);
        }

        while( CodedBlockListPtr < CodedBlockListEnd )
        {
            // Get the linear index for the current fragment.
            FragIndex = *CodedBlockListPtr;

            // Should we decode a token for this block on this pass.
            if ( pbi->FragCoeffs[FragIndex] <= EncodedCoeffs )
            {
				pbi->FragCoefEOB[FragIndex] = pbi->FragCoeffs[FragIndex];
                // If we are in the middle of an EOB run
                if ( pbi->EOB_Run )
                {
                    // Mark the current block as fully expanded and decrement EOB_RUN count
                    pbi->FragCoeffs[FragIndex] = BLOCK_SIZE;
                    pbi->EOB_Run --;
                    pbi->BlocksToDecode --;
                }
                // Else unpack an AC token
                else
                {
                    // Work out which huffman table to use, then decode a token
                    if ( FragIndex < (INT32)pbi->YPlaneFragments )
                        pbi->ACHuffChoice = AcHuffChoice1;
                    else
                        pbi->ACHuffChoice = AcHuffChoice2;

                    UnpackAndExpandAcToken( pbi, pbi->QFragData[FragIndex], &pbi->FragCoeffs[FragIndex] );
                }
            }
            CodedBlockListPtr++;
        }

        // Test for condition where there are no blocks left with any tokesn to decode
        if ( !pbi->BlocksToDecode )
            break;

        EncodedCoeffs ++;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     ExtractToken
 *
 *  INPUTS        :     INT8 * ExpandedBlock
 *                             Pointer to block structure into which the token
 *                             should be expanded.
 *
 *                      UINT32 * CoeffIndex
 *                             Where we are in the current block.
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     The number of bits decoded
 *
 *  FUNCTION      :     Unpacks and expands an DC DCT token.
 *
 *  SPECIAL NOTES :     PROBLEM !!!!!!!!!!!   right now handles only left 
 *                      justified bits in bitreader.  the c version keeps every
 *                      thing in place so I can't use it!!
 *                      
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT32 ExtractToken(BITREADER *br, HUFF_ENTRY * CurrentRoot)
{
	UINT32 Token;
    // Loop searches down through tree based upon bits read from the bitstream 
    // until it hits a leaf at which point we have decoded a token
	while ( CurrentRoot->Value < 0 )
    {
		
		if ( bitread1(br) )
            CurrentRoot = (HUFF_ENTRY *)(CurrentRoot->OneChild);
        else
            CurrentRoot = (HUFF_ENTRY *)(CurrentRoot->ZeroChild);

    }
    Token = CurrentRoot->Value; 
	return Token;
}

/****************************************************************************
 * 
 *  ROUTINE       :     UnpackAndExpandDcToken
 *
 *  INPUTS        :     INT8 * ExpandedBlock
 *                             Pointer to block structure into which the token
 *                             should be expanded.
 *
 *                      UINT32 * CoeffIndex
 *                             Where we are in the current block.
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     The number of bits decoded
 *
 *  FUNCTION      :     Unpacks and expands an DC DCT token.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UnpackAndExpandDcToken( PB_INSTANCE *pbi, Q_LIST_ENTRY * ExpandedBlock, UINT8 * CoeffIndex )
{
    INT32           ExtraBits;
    UINT32          Token;

    // Extract a token.   
    Token = ExtractToken(&pbi->br, pbi->HuffRoot_VP3x[pbi->DcHuffChoice]);


    /* Now.. if we are using the DCT optimised coding system, extract any
    *  assosciated additional bits token. 
    */
    if ( ExtraBitLengths_VP31[Token] > 0 )
    {
        /* Extract the appropriate number of extra bits. */
        ExtraBits = bitread(&pbi->br, ExtraBitLengths_VP31[Token]);

    }

    // Take token dependant action
    if ( Token >= DCT_SHORT_ZRL_TOKEN )
    {
        // "Value", "zero run" and "zero run value" tokens
        ExpandToken(  pbi, ExpandedBlock, CoeffIndex, Token, ExtraBits );
        if ( *CoeffIndex >= BLOCK_SIZE )
            pbi->BlocksToDecode --;
    }
    else if ( Token == DCT_EOB_TOKEN )
    {
        *CoeffIndex = BLOCK_SIZE;
        pbi->BlocksToDecode --;
    }
    else
    {
        // Special action and EOB tokens
        switch ( Token )
        {
        case DCT_EOB_PAIR_TOKEN:
            pbi->EOB_Run = 1;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        case DCT_EOB_TRIPLE_TOKEN:
            pbi->EOB_Run = 2;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        case DCT_REPEAT_RUN_TOKEN:
            pbi->EOB_Run = ExtraBits + 3;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        case DCT_REPEAT_RUN2_TOKEN:
            pbi->EOB_Run = ExtraBits + 7;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        case DCT_REPEAT_RUN3_TOKEN:
            pbi->EOB_Run = ExtraBits + 15;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        case DCT_REPEAT_RUN4_TOKEN:
            pbi->EOB_Run = ExtraBits - 1;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        }
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     UnpackAndExpandAcToken
 *
 *  INPUTS        :     INT8 * ExpandedBlock
 *                             Pointer to block structure into which the token
 *                             should be expanded.
 *
 *                      UINT32 * CoeffIndex
 *                             Where we are in the current block.
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     The number of bits decoded
 *
 *  FUNCTION      :     Unpacks and expands a AC DCT token.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UnpackAndExpandAcToken( PB_INSTANCE *pbi, Q_LIST_ENTRY * ExpandedBlock, UINT8 * CoeffIndex  )
{
    INT32           ExtraBits;
    UINT32          Token;

    // Extract a token.   
    Token = ExtractToken(&pbi->br, pbi->HuffRoot_VP3x[pbi->ACHuffChoice]);


    /* Now.. if we are using the DCT optimised coding system, extract any
    *  assosciated additional bits token. 
    */
    if ( ExtraBitLengths_VP31[Token] > 0 )
    {
        /* Extract the appropriate number of extra bits. */
        ExtraBits = bitread(&pbi->br,ExtraBitLengths_VP31[Token]); //0;
    }

    // Take token dependant action
    if ( Token >= DCT_SHORT_ZRL_TOKEN )
    {
        // "Value", "zero run" and "zero run value" tokens
        ExpandToken(  pbi, ExpandedBlock, CoeffIndex, Token, ExtraBits );
        if ( *CoeffIndex >= BLOCK_SIZE )
            pbi->BlocksToDecode --;
    }
    else if ( Token == DCT_EOB_TOKEN )
    {
        *CoeffIndex = BLOCK_SIZE;
        pbi->BlocksToDecode --;
    }
    else
    {
        // Special action and EOB tokens
        switch ( Token )
        {
        case DCT_EOB_PAIR_TOKEN:
            pbi->EOB_Run = 1;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        case DCT_EOB_TRIPLE_TOKEN:
            pbi->EOB_Run = 2;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        case DCT_REPEAT_RUN_TOKEN:
            pbi->EOB_Run = ExtraBits + 3;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        case DCT_REPEAT_RUN2_TOKEN:
            pbi->EOB_Run = ExtraBits + 7;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        case DCT_REPEAT_RUN3_TOKEN:
            pbi->EOB_Run = ExtraBits + 15;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        case DCT_REPEAT_RUN4_TOKEN:
            pbi->EOB_Run = ExtraBits - 1;
            *CoeffIndex = BLOCK_SIZE;
            pbi->BlocksToDecode --;
            break;
        }
    }
}


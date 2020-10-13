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
*   Module Title :     DDecode.C
*
*   Description  :     Video CODEC: Top level decode module.
*
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "BlockMapping.h"
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

void DecodeModes( PB_INSTANCE *pbi, UINT32 SBRows, UINT32 SBCols, UINT32 HExtra, UINT32 VExtra );
void DecodeMVectors ( PB_INSTANCE *pbi, UINT32 SBRows, UINT32 SBCols, UINT32 HExtra, UINT32 VExtra );

INT32 ExtractMVectorComponentA(PB_INSTANCE *pbi);
INT32 ExtractMVectorComponentB(PB_INSTANCE *pbi);


// Mode coding schemes
CODING_MODE  ModeAlphabet[MODE_METHODS-1][MAX_MODES] = 
{   
    {    (CODING_MODE)0,(CODING_MODE)0,(CODING_MODE)0,(CODING_MODE)0,(CODING_MODE)0,(CODING_MODE)0,(CODING_MODE)0,(CODING_MODE)0 },     // Reserved for custom alphabet.
    
    // Last motion vector dominates
    {    CODE_INTER_LAST_MV,    CODE_INTER_PRIOR_LAST,  CODE_INTER_PLUS_MV,     CODE_INTER_NO_MV,     
         CODE_INTRA,            CODE_USING_GOLDEN,      CODE_GOLDEN_MV,         CODE_INTER_FOURMV },

    {    CODE_INTER_LAST_MV,    CODE_INTER_PRIOR_LAST,  CODE_INTER_NO_MV,       CODE_INTER_PLUS_MV,     
         CODE_INTRA,            CODE_USING_GOLDEN,      CODE_GOLDEN_MV,         CODE_INTER_FOURMV },
         
    {    CODE_INTER_LAST_MV,    CODE_INTER_PLUS_MV,     CODE_INTER_PRIOR_LAST,  CODE_INTER_NO_MV,     
         CODE_INTRA,            CODE_USING_GOLDEN,      CODE_GOLDEN_MV,         CODE_INTER_FOURMV },

    {    CODE_INTER_LAST_MV,    CODE_INTER_PLUS_MV,     CODE_INTER_NO_MV,       CODE_INTER_PRIOR_LAST,     
         CODE_INTRA,            CODE_USING_GOLDEN,      CODE_GOLDEN_MV,         CODE_INTER_FOURMV },

    //   No motion vector dominates
    {    CODE_INTER_NO_MV,      CODE_INTER_LAST_MV,     CODE_INTER_PRIOR_LAST,  CODE_INTER_PLUS_MV,     
         CODE_INTRA,            CODE_USING_GOLDEN,      CODE_GOLDEN_MV,         CODE_INTER_FOURMV },
    
    {    CODE_INTER_NO_MV,      CODE_USING_GOLDEN,      CODE_INTER_LAST_MV,     CODE_INTER_PRIOR_LAST,       
         CODE_INTER_PLUS_MV,    CODE_INTRA,             CODE_GOLDEN_MV,         CODE_INTER_FOURMV },

};

/****************************************************************************
*  Imports
*****************************************************************************
*/  

/****************************************************************************
 * 
 *  ROUTINE       :     DecodeData
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Decodes and displays the video stream.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DecodeData(PB_INSTANCE *pbi)
{   
    UINT32 i;
    UINT32 YHExtra = pbi->HFragments%4;
    UINT32 YVExtra = pbi->VFragments%4;
    UINT32 UvHExtra = (pbi->HFragments/2)%4;
    UINT32 UvVExtra = (pbi->VFragments/2)%4;

    // Bail out immediately if a decode error has already been reported.
//    if ( pbi->DecoderErrorCode != NO_DECODER_ERROR )
//        return;

    /* Clear down the macro block level mode and MV arrays. */
    for ( i = 0; i < pbi->UnitFragments; i++ )
    {
        pbi->FragCodingMethod[i] = CODE_INTER_NO_MV;     // Default coding mode
        pbi->FragMVect[i].x = 0;
        pbi->FragMVect[i].y = 0;
    }

    // Zero Decoder EOB run count 
    pbi->EOB_Run = 0;

    // Make a not of the number of coded blocks this frame
    pbi->CodedBlocksThisFrame = pbi->CodedBlockIndex;

	// Decode the modes data
	DecodeModes( pbi, pbi->YSBRows, pbi->YSBCols, YHExtra, YVExtra );

	// Unpack and decode the motion vectors.
	DecodeMVectors ( pbi, pbi->YSBRows, pbi->YSBCols, YHExtra, YVExtra );

	// Unpack and decode the actual video data.
    pbi->UnPackVideo(pbi);

	// Reconstruct and display the frame
    ReconRefFrames(pbi);

}

/****************************************************************************
 * 
 *  ROUTINE       :     DecodeModes
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     Reconstructed frame.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Decodes the coding mode list for this frame.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DecodeModes( PB_INSTANCE *pbi, UINT32 SBRows, UINT32 SBCols, UINT32 HExtra, UINT32 VExtra )
{
	INT32	FragIndex;			// Fragment number
	UINT32	MB;	    			// Macro-Block, Block indices
	UINT32	SBrow;				// Super-Block row number
	UINT32	SBcol;				// Super-Block row number
	UINT32	SB=0;			    // Super-Block index
    CODING_MODE  CodingMethod;  // Temp Storage for coding mode.

    UINT32  UVRow;
    UINT32  UVColumn;
    UINT32  UVFragOffset;
    
    UINT32  CodingScheme;       // Coding scheme used to code modes.

    UINT32  MBListIndex = 0;

    UINT32  i;

    // If the frame is an intra frame then all blocks have mode intra.
    if ( GetFrameType(pbi) == BASE_FRAME )
    {
        for ( i = 0; i < pbi->UnitFragments; i++ )
        {
            pbi->FragCodingMethod[i] = CODE_INTRA;
        }
    }
    else
    {
        UINT32  ModeEntry;                            // Mode bits read  

        // Read the coding method
        CodingScheme = bitread( &pbi->br,  MODE_METHOD_BITS );  

        // If the coding method is method 0 then we have to read in a custom coding scheme
        if ( CodingScheme == 0 )
        {
            // Read the coding scheme.
            for ( i = 0; i < MAX_MODES; i++ )
            {
                ModeAlphabet[0][ bitread( &pbi->br,  MODE_BITS) ] = (CODING_MODE)i;
            }
        }

	    // Unravel the quad-tree
	    for ( SBrow=0; SBrow<SBRows; SBrow++ )
	    {
		    for ( SBcol=0; SBcol<SBCols; SBcol++ )
		    {
				for ( MB=0; MB<4; MB++ )
				{
					// There may be MB's lying out of frame
					// which must be ignored. For these MB's
					// top left block will have a negative Fragment Index.
    				if ( QuadMapToMBTopLeft(pbi->BlockMap, SB,MB) >= 0 )
					{
						// Is the Macro-Block coded:
						if ( pbi->MBCodedFlags[MBListIndex++] )
						{
                            // Upack the block level modes and motion vectors
                            FragIndex = QuadMapToMBTopLeft( pbi->BlockMap, SB, MB );
                        
                            // Unpack the mode.
                            if ( CodingScheme == (MODE_METHODS-1) )
                            {
                                // This is the fall back coding scheme.
                                // Simply MODE_BITS bits per mode entry.
                                CodingMethod = (CODING_MODE)bitread( &pbi->br,  MODE_BITS );
                            }
                            else
                            {
                                ModeEntry = FrArrayUnpackMode(pbi);
                                CodingMethod =  ModeAlphabet[CodingScheme][ ModeEntry ];
                            }

                            // Note the coding mode for each block in macro block.
                            pbi->FragCodingMethod[FragIndex] = CodingMethod;
                            pbi->FragCodingMethod[FragIndex + 1] = CodingMethod;
                            pbi->FragCodingMethod[FragIndex + pbi->HFragments] = CodingMethod;
                            pbi->FragCodingMethod[FragIndex + pbi->HFragments + 1] = CodingMethod;

                            // Matching fragments in the U and V planes
                            UVRow = (FragIndex / (pbi->HFragments * 2));
                            UVColumn = (FragIndex % pbi->HFragments) / 2;
                            UVFragOffset = (UVRow * (pbi->HFragments / 2)) + UVColumn;

                            pbi->FragCodingMethod[pbi->YPlaneFragments + UVFragOffset] = CodingMethod;
                            pbi->FragCodingMethod[pbi->YPlaneFragments + pbi->UVPlaneFragments + UVFragOffset] = CodingMethod;

						}
					}
			    }

			    // Next Super-Block
			    SB++;
		    }
	    }
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     DecodeMVectors
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Decodes the motion vectors for this frame.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DecodeMVectors ( PB_INSTANCE *pbi, UINT32 SBRows, UINT32 SBCols, UINT32 HExtra, UINT32 VExtra )
{
	INT32	FragIndex;			// Fragment number
	UINT32	MB;		    		// Macro-Block, Block indices
	UINT32	SBrow;				// Super-Block row number
	UINT32	SBcol;				// Super-Block row number
	UINT32	SB=0;			    // Super-Block index
    UINT32  CodingMethod;       // Temp Storage for coding mode.

    MOTION_VECTOR MVect[6];     // temp storage for motion vector
    MOTION_VECTOR TmpMVect;
    MOTION_VECTOR LastInterMV;      // storage for last used Inter frame MB motion vector
    MOTION_VECTOR PriorLastInterMV; // storage for previous last used Inter frame MB motion vector
	INT32 (*ExtractMVectorComponent)(PB_INSTANCE *pbi);

    UINT32  UVRow;
    UINT32  UVColumn;
    UINT32  UVFragOffset;

    UINT32  MBListIndex = 0;

    UINT32  MVCode = 0;         // Temporary storage while decoding the MV

    // Should not be decoding motion vectors if in INTRA only mode.
    if ( GetFrameType(pbi) == BASE_FRAME )
    {
        return;
    }

    // set the default motion vector to 0,0
    MVect[0].x = 0;
    MVect[0].y = 0;
    LastInterMV.x = 0;
    LastInterMV.y = 0;
    PriorLastInterMV.x = 0;
    PriorLastInterMV.y = 0;

    // Read the entropy method used and set up the appropriate decode option
    if ( bitread1( &pbi->br) == 0 )
        ExtractMVectorComponent = ExtractMVectorComponentA;
    else
        ExtractMVectorComponent = ExtractMVectorComponentB;

    // Unravel the quad-tree
	for ( SBrow=0; SBrow<SBRows; SBrow++ )
	{
		for ( SBcol=0; SBcol<SBCols; SBcol++ )
		{
			for ( MB=0; MB<4; MB++ )
			{
				// There may be MB's lying out of frame
				// which must be ignored. For these MB's
				// the top left block will have a negative Fragment Index.
				if ( QuadMapToMBTopLeft(pbi->BlockMap, SB,MB) >= 0 )
				{
					// Is the Macro-Block further coded:
					if ( pbi->MBCodedFlags[MBListIndex++] )
					{
                        // Upack the block level modes and motion vectors
                        FragIndex = QuadMapToMBTopLeft( pbi->BlockMap, SB, MB );

                        // Clear the motion vector before we start.
                        MVect[0].x = 0;
                        MVect[0].y = 0;
                            
                        // Unpack the mode (and motion vectors if necessary).
   		                CodingMethod = pbi->FragCodingMethod[FragIndex];

                        // Read the motion vector or vectors if present. 
                        if ( (CodingMethod == CODE_INTER_PLUS_MV) || 
                             (CodingMethod == CODE_GOLDEN_MV) )
                        {
                            MVect[0].x = ExtractMVectorComponent(pbi); 
                            MVect[1].x = MVect[0].x;
                            MVect[2].x = MVect[0].x;
                            MVect[3].x = MVect[0].x;
                            MVect[4].x = MVect[0].x;
                            MVect[5].x = MVect[0].x;
                            MVect[0].y = ExtractMVectorComponent(pbi); 
                            MVect[1].y = MVect[0].y;
                            MVect[2].y = MVect[0].y;
                            MVect[3].y = MVect[0].y;
                            MVect[4].y = MVect[0].y;
                            MVect[5].y = MVect[0].y;
                        }
                        else if ( CodingMethod == CODE_INTER_FOURMV )
                        {
                            // Extrac the 4 Y MVs
                            MVect[0].x = ExtractMVectorComponent(pbi);
                            MVect[0].y = ExtractMVectorComponent(pbi);

                            MVect[1].x = ExtractMVectorComponent(pbi);
                            MVect[1].y = ExtractMVectorComponent(pbi);
                            
                            MVect[2].x = ExtractMVectorComponent(pbi);
                            MVect[2].y = ExtractMVectorComponent(pbi);
                            
                            MVect[3].x = ExtractMVectorComponent(pbi);
                            MVect[3].y = ExtractMVectorComponent(pbi);

                            // Calculate the U and V plane MVs as the average of the Y plane MVs.
                            // First .x component
                            MVect[4].x = MVect[0].x + MVect[1].x + MVect[2].x + MVect[3].x;
                            if ( MVect[4].x >= 0 )
                                MVect[4].x = (MVect[4].x + 2) / 4;
                            else
                                MVect[4].x = (MVect[4].x - 2) / 4;
                            MVect[5].x = MVect[4].x;
                            // Then .y component
                            MVect[4].y = MVect[0].y + MVect[1].y + MVect[2].y + MVect[3].y;
                            if ( MVect[4].y >= 0 )
                                MVect[4].y = (MVect[4].y + 2) / 4;
                            else
                                MVect[4].y = (MVect[4].y - 2) / 4;
                            MVect[5].y = MVect[4].y;
                        }

                        // Keep track of last and prior last inter motion vectors.
                        if ( CodingMethod == CODE_INTER_PLUS_MV )
                        {
                            PriorLastInterMV.x = LastInterMV.x;
                            PriorLastInterMV.y = LastInterMV.y;
                            LastInterMV.x = MVect[0].x;
                            LastInterMV.y = MVect[0].y;
                        }
                        else if ( CodingMethod == CODE_INTER_LAST_MV )
                        {
                            // Use the last coded Inter motion vector.
                            MVect[0].x = LastInterMV.x;
                            MVect[1].x = MVect[0].x;
                            MVect[2].x = MVect[0].x;
                            MVect[3].x = MVect[0].x;
                            MVect[4].x = MVect[0].x;
                            MVect[5].x = MVect[0].x;
                            MVect[0].y = LastInterMV.y;
                            MVect[1].y = MVect[0].y;
                            MVect[2].y = MVect[0].y;
                            MVect[3].y = MVect[0].y;
                            MVect[4].y = MVect[0].y;
                            MVect[5].y = MVect[0].y;
                        }
                        else if ( CodingMethod == CODE_INTER_PRIOR_LAST )
                        {
                            // Use the last coded Inter motion vector.
                            MVect[0].x = PriorLastInterMV.x;
                            MVect[1].x = MVect[0].x;
                            MVect[2].x = MVect[0].x;
                            MVect[3].x = MVect[0].x;
                            MVect[4].x = MVect[0].x;
                            MVect[5].x = MVect[0].x;
                            MVect[0].y = PriorLastInterMV.y;
                            MVect[1].y = MVect[0].y;
                            MVect[2].y = MVect[0].y;
                            MVect[3].y = MVect[0].y;
                            MVect[4].y = MVect[0].y;
                            MVect[5].y = MVect[0].y;

                            // Swap the prior and last MV cases over
                            TmpMVect.x = PriorLastInterMV.x;
                            TmpMVect.y = PriorLastInterMV.y;
                            PriorLastInterMV.x = LastInterMV.x;
                            PriorLastInterMV.y = LastInterMV.y;
                            LastInterMV.x = TmpMVect.x;
                            LastInterMV.y = TmpMVect.y;
                        }
                        else if ( CodingMethod == CODE_INTER_FOURMV )
                        {
                            // Update last MV and prior last mv
                            PriorLastInterMV.x = LastInterMV.x;
                            PriorLastInterMV.y = LastInterMV.y;
                            LastInterMV.x = MVect[3].x;
                            LastInterMV.y = MVect[3].y;
                        }

                        // Note the coding mode and vector for each block in the current macro block.
                        pbi->FragMVect[FragIndex].x = MVect[0].x;
                        pbi->FragMVect[FragIndex].y = MVect[0].y;
                
                        pbi->FragMVect[FragIndex + 1].x = MVect[1].x;
                        pbi->FragMVect[FragIndex + 1].y = MVect[1].y;

                        pbi->FragMVect[FragIndex + pbi->HFragments].x = MVect[2].x;
                        pbi->FragMVect[FragIndex + pbi->HFragments].y = MVect[2].y;

                        pbi->FragMVect[FragIndex + pbi->HFragments + 1].x = MVect[3].x;
                        pbi->FragMVect[FragIndex + pbi->HFragments + 1].y = MVect[3].y;

                        // Matching fragments in the U and V planes
                        UVRow = (FragIndex / (pbi->HFragments * 2));
                        UVColumn = (FragIndex % pbi->HFragments) / 2;
                        UVFragOffset = (UVRow * (pbi->HFragments / 2)) + UVColumn;

                        pbi->FragMVect[pbi->YPlaneFragments + UVFragOffset].x = MVect[4].x;
                        pbi->FragMVect[pbi->YPlaneFragments + UVFragOffset].y = MVect[4].y;

                        pbi->FragMVect[pbi->YPlaneFragments + pbi->UVPlaneFragments + UVFragOffset].x = MVect[5].x;
                        pbi->FragMVect[pbi->YPlaneFragments + pbi->UVPlaneFragments + UVFragOffset].y = MVect[5].y;
					}
				}
			}

			// Next Super-Block
			SB++;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     ExtractMVectorComponentA
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Extracts a motion vector component coded with method A.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
INT32 ExtractMVectorComponentA(PB_INSTANCE *pbi)
{
    INT32   MVectComponent;     // temp storage for motion vector
    UINT32  MVCode = 0;         // Temporary storage while decoding the MV
    UINT32  ExtraBits = 0;

    // Get group to which coded component belongs
    MVCode = bitread( &pbi->br,  3 ); 

    //  Now extract the appropriate number of bits to identify the component
    switch ( MVCode )
    {
    case 0:
        MVectComponent = 0;
        break;
    case 1:
        MVectComponent = 1;
        break;
    case 2:
        MVectComponent = -1;
        break;
    case 3:
        if ( bitread1( &pbi->br ))
            MVectComponent = -2;
        else 
            MVectComponent = 2;
        break;
    case 4:
        if ( bitread1( &pbi->br ) )
            MVectComponent = -3;
        else 
            MVectComponent = 3;
        break;
    case 5:
        ExtraBits = bitread( &pbi->br,  2 ); 
        MVectComponent = 4 + ExtraBits;
        if ( bitread1( &pbi->br ) )
            MVectComponent = -MVectComponent;
        break;
    case 6:
        ExtraBits = bitread( &pbi->br,  3 ); 
        MVectComponent = 8 + ExtraBits;
        if ( bitread1( &pbi->br ))
            MVectComponent = -MVectComponent;
        break;
    case 7:
        ExtraBits = bitread( &pbi->br,  4 ); 
        MVectComponent = 16 + ExtraBits;
        if ( bitread1( &pbi->br ) )
            MVectComponent = -MVectComponent;
        break;
    }

    return MVectComponent;
}

/****************************************************************************
 * 
 *  ROUTINE       :     ExtractMVectorComponentB
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Extracts an MV component coded using the fallback method
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
INT32 ExtractMVectorComponentB(PB_INSTANCE *pbi)
{
    INT32   MVectComponent;     // temp storage for motion vector

    // Get group to which coded component belongs
    MVectComponent = bitread( &pbi->br,  5 ); 
    if ( bitread1( &pbi->br ) )
        MVectComponent = -MVectComponent;

    return MVectComponent;
}




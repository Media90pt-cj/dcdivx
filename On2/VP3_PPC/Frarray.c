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
*   Module Title :     Frarray.C     
*
*   Description  :     Functions to read and write fragment arrays.
*
*
*****************************************************************************
*/

/****************************************************************************
*  Header Frames
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "BlockMapping.h"
#include "pbdll.h"

#include "stdio.h"

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

// The following hard wired tables and masks are used to decode block pattern tokens
UINT8 BlockPatternMask[4] = { 0x08, 0x04, 0x02, 0x01 };


UINT8 BlockDecode1[2][8] = {	{ 0x01, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 },
								{ 0x07, 0x0B, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00 } };

UINT8 BlockDecode2[2][16] = {	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 
                                  0x0A, 0x03, 0x0E, 0x07, 0x0D, 0x05, 0x0C, 0x02 },

								{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                                  0x05, 0x03, 0x02, 0x0C, 0x04, 0x0A, 0x08, 0x0D } };

UINT8 BlockDecode3[2][32] = {	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
								  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x09, 
								  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },

								{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
								  0x00, 0x00, 0x00, 0x00, 0x09, 0x06, 0x00, 0x00, 
								  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, } };

UINT32 BPPredictor[15] =		{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1  };	

/****************************************************************************
*  Forward References
*****************************************************************************
*/              
void QuadDecodeDisplayFragments ( PB_INSTANCE *pbi );
void QuadDecodeDisplayFragments2 ( PB_INSTANCE *pbi );

BOOL FrArrayDeCodeBlockRun(  PB_INSTANCE *pbi, UINT32 bit_value, INT32 * run_value );
BOOL FrArrayDeCodeMBRun(  PB_INSTANCE *pbi, UINT32 bit_value, INT32 * run_value );
BOOL FrArrayDeCodeSBRun(  PB_INSTANCE *pbi, UINT32 bit_value, INT32 * run_value );

void    GetNextBInit(PB_INSTANCE *pbi);
UINT8   GetNextBBit(PB_INSTANCE *pbi);
void    GetNextSbInit(PB_INSTANCE *pbi);
UINT8   GetNextSbBit(PB_INSTANCE *pbi);

void	GetNextMbInit(PB_INSTANCE *pbi);
UINT8   GetNextMbBit(PB_INSTANCE *pbi);
void	ReadBlockPatternInit (PB_INSTANCE *pbi);
UINT8	ReadNextBlockPattern (PB_INSTANCE *pbi);

void    InitialiseMBDecimatedArray ( PB_INSTANCE *pbi );


/****************************************************************************
*  Imports
*****************************************************************************
*/  


/****************************************************************************
 * 
 *  ROUTINE       :     ReadAndUnPackDFArray
 *
 *  INPUTS        :     The playback instance 
 *                      
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Reads and unpacks an array of display fragments.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void ReadAndUnPackDFArray( PB_INSTANCE *pbi )
{
    // Decode the Quad-tree coded display_fragments list
    QuadDecodeDisplayFragments ( pbi );
}

/****************************************************************************
 * 
 *  ROUTINE       :     QuadDeCodeDisplayFragments
 *
 *  INPUTS        :     PB instance
 *
 *  OUTPUTS       :     Mapping table BlockMap[SuperBlock][MacroBlock][Block]
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Creates mapping table between (SuperBlock, MacroBlock, Block)
 *						triplet and corresponding Fragment Index.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void QuadDecodeDisplayFragments ( PB_INSTANCE *pbi )
{
	UINT32	SB, MB, B;		// Super-block, Macro-block and Block values
    BOOL    DataToDecode; 

    INT32   dfIndex;
	UINT32  MBIndex = 0;

	// Reset various data structures common to key frames and inter frames.
	pbi->CodedBlockIndex = 0;
	memset ( pbi->display_fragments, 0, pbi->UnitFragments );

    // For "Key frames" mark all blocks as coded and return.
    // Else initialise the ArrayPtr array to 0 (all blocks uncoded by default) 
	if ( GetFrameType(pbi) == BASE_FRAME )
    {
        memset( pbi->SBFullyFlags, 1, pbi->SuperBlocks );
        memset( pbi->SBCodedFlags, 1, pbi->SuperBlocks );
        memset( pbi->MBCodedFlags, 0, pbi->MacroBlocks );
    }
    else
    {
        memset( pbi->SBFullyFlags, 0, pbi->SuperBlocks );
        memset( pbi->MBCodedFlags, 0, pbi->MacroBlocks );

		// Un-pack the list of partially coded Super-Blocks
		GetNextSbInit(pbi);
		for( SB = 0; SB < pbi->SuperBlocks; SB++)
		{
			pbi->SBCodedFlags[SB] = GetNextSbBit (pbi);
		}

		// Scan through the list of super blocks. 
		// Unless all are marked as partially coded we have more to do.
		DataToDecode = FALSE; 
		for ( SB=0; SB<pbi->SuperBlocks; SB++ )
		{
			if ( !pbi->SBCodedFlags[SB] )
			{
				DataToDecode = TRUE;
				break;
			}
		}

		// Are there further block map bits to decode ?
		if ( DataToDecode )
		{
			// Un-pack the Super-Block fully coded flags.
			GetNextSbInit(pbi);
			for( SB = 0; SB < pbi->SuperBlocks; SB++)
			{
				// Skip blocks already marked as partially coded
				while( (SB < pbi->SuperBlocks) && pbi->SBCodedFlags[SB] )  
					SB++;

				if ( SB < pbi->SuperBlocks )
				{
					pbi->SBFullyFlags[SB] = GetNextSbBit (pbi);

					if ( pbi->SBFullyFlags[SB] )         // If SB is fully coded.
						pbi->SBCodedFlags[SB] = 1;       // Mark the SB as coded
				}
			}
		}

		// Scan through the list of coded super blocks.
		// If at least one is marked as partially coded then we have a block list to decode.
		for ( SB=0; SB<pbi->SuperBlocks; SB++ )
		{
			if ( pbi->SBCodedFlags[SB] && !pbi->SBFullyFlags[SB] )
			{
				// Initialise the block list decoder.
				GetNextBInit(pbi);
				break;
			}
		}
	}

	// Decode the block data from the bit stream.
	for ( SB=0; SB<pbi->SuperBlocks; SB++ )
	{
		for ( MB=0; MB<4; MB++ )
		{
            // If MB is in the frame
			if ( QuadMapToMBTopLeft(pbi->BlockMap, SB,MB) >= 0 )
            {
				// Only read block level data if SB was fully or partially coded
				if ( pbi->SBCodedFlags[SB] )
				{
					for ( B=0; B<4; B++ )
					{
						// If block is valid (in frame)...
						dfIndex = QuadMapToIndex1( pbi->BlockMap, SB, MB, B );
						if ( dfIndex >= 0 )
						{
							if ( pbi->SBFullyFlags[SB] )
								pbi->display_fragments[dfIndex] = 1;
							else
								pbi->display_fragments[dfIndex] = GetNextBBit(pbi);

							// Create linear list of coded block indices
							if ( pbi->display_fragments[dfIndex] )
							{
								pbi->MBCodedFlags[MBIndex] = 1;
								pbi->CodedBlockList[pbi->CodedBlockIndex] = dfIndex;
								pbi->CodedBlockIndex++;
							}
						}
					}
				}
				MBIndex++;

            }
		}
    }
}


/****************************************************************************
 * 
 *  ROUTINE       :     QuadDeCodeDisplayFragments2
 *
 *  INPUTS        :     PB instance
 *
 *  OUTPUTS       :     Mapping table BlockMap[SuperBlock][MacroBlock][Block]
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Creates mapping table between (SuperBlock, MacroBlock, Block)
 *						triplet and corresponding Fragment Index.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void QuadDecodeDisplayFragments2 ( PB_INSTANCE *pbi )
{
	UINT32	SB, MB, B;	// Super-block, Macro-block and Block values
    BOOL    DataToDecode = FALSE; 

	INT32   dfIndex;
	UINT32  MBIndex = 0;

	UINT8 * MBFully;
	UINT8 * MBCoded;

	UINT8   BPattern;

	// Set up local pointers
	MBFully = pbi->MBFullyFlags;
	MBCoded = pbi->MBCodedFlags;

	// Reset various data structures common to key frames and inter frames.
	pbi->CodedBlockIndex = 0;
	memset( pbi->display_fragments, 0, pbi->UnitFragments );

    // For "Key frames" mark all blocks as coded and return.
    // Else initialise the ArrayPtr array to 0 (all blocks uncoded by default) 
	if ( GetFrameType(pbi) == BASE_FRAME )
    {
        memset( MBFully, 1, pbi->MacroBlocks );
    }
    else
    {
        memset( MBFully, 0, pbi->MacroBlocks );
	    memset( MBCoded, 0, pbi->MacroBlocks );

		// Un-pack MBlist1
		GetNextMbInit(pbi);
		for( MB = 0; MB < pbi->MacroBlocks; MB++)
		{
			MBFully[MB] = GetNextMbBit (pbi);
		}

		// If there are any macro blocks that are not fully coded then there is more to do.
		for( MB = 0; MB < pbi->MacroBlocks; MB++ )
		{
			if ( !MBFully[MB] )
			{
				DataToDecode = TRUE;
				break;
			}
		}

		// Build the coded MB list (Fully or partially coded)
		if ( DataToDecode )
		{
			// Un-pack MBlist2
			GetNextMbInit(pbi);
			for( MB = 0; MB < pbi->MacroBlocks; MB++)
			{
				if ( !MBFully[MB] )
					MBCoded[MB] = GetNextMbBit( pbi );
			}

		}
	}

	// Complete the block and macro block coded data structures.
	// Initialise the block pattern reader.
	ReadBlockPatternInit(pbi);

	// Follow the quad tree structure
	for ( SB=0; SB < pbi->SuperBlocks; SB++ )
	{
		for ( MB=0; MB<4; MB++ )
		{
            // If MB is in the frame
			if ( QuadMapToMBTopLeft(pbi->BlockMap, SB,MB) >= 0 )
            {
				// Set or read the block pattern for the macro block#
				if ( MBFully[MBIndex] )
				{
					MBCoded[MBIndex] = 1;
					BPattern = 0x0F;
				}
				else if ( MBCoded[MBIndex] )
				{
					BPattern = ReadNextBlockPattern(pbi);
				}
				else
				{
					BPattern = 0;
				}

				for ( B=0; B<4; B++ )
			    {
                    // If block is valid (in frame)...
                    dfIndex = QuadMapToIndex2( pbi->BlockMap, SB, MB, B );
				    if ( dfIndex >= 0 )
				    {
						// Work out if this block is coded or not.
						pbi->display_fragments[dfIndex] = (BPattern & BlockPatternMask[B]) ? 1 : 0;
				    }
			    }

				for ( B=0; B<4; B++ )
			    {
                    // If block is valid (in frame)...
                    dfIndex = QuadMapToIndex1( pbi->BlockMap, SB, MB, B );
				    if ( dfIndex >= 0 )
				    {
						if ( pbi->display_fragments[dfIndex] )
						{
							pbi->CodedBlockList[pbi->CodedBlockIndex] = dfIndex;
							pbi->CodedBlockIndex++;
						}
				    }
			    }

				// Increment the MB index.
				MBIndex++;
            }
		}
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     FrArrayDeCodeInit
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises the decoding of a run.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
__inline void FrArrayDeCodeInit(PB_INSTANCE *pbi)
{   
    // Initialise the decoding of a run. 
    pbi->bit_pattern = 0;
    pbi->bits_so_far = 0; 
}

/****************************************************************************
 * 
 *  ROUTINE       :     FrArrayCodeMBRun
 *
 *  INPUTS        :     UINT8 bit_value
 *
 *  OUTPUTS       :     UINT8 run_value
 *
 *  RETURNS       :     TRUE when a value has been fully decoded.
 *
 *  FUNCTION      :     Decodes a MB run value
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
BOOL FrArrayDeCodeMBRun(  PB_INSTANCE *pbi, UINT32 bit_value, INT32 * run_value )
{
    BOOL ret_val = FALSE;

    // Add in the new bit value.
    pbi->bits_so_far++;
    pbi->bit_pattern = (pbi->bit_pattern << 1) + (bit_value & 1);
        
    // Coding scheme:
	//	Codeword				RunLength
    //  0                       1
	//  10						2
	//	110x				    3-4
	//	1110xx					5-8
	//	11110xxx				9-16
	//	111110xxxx				17-32
	//	1111110xxxxx			33-64
	//	11111110xxxxxx			65-128
	//	111111110xxxxxxx		129-256
	//  111111111				256 repeats
	switch ( pbi->bits_so_far )
	{
	case 1:  
		if ( pbi->bit_pattern == 0 )
		{
			ret_val = TRUE;
			*run_value += 1;                             
		}
		break; 

	case 2:
        // Bit 1 clear
		if ( pbi->bit_pattern == 2 )
		{
			ret_val = TRUE;
			*run_value += 2;                             
		}
		break; 
        
	case 4:
        // Bit 1 clear
		if ( !(pbi->bit_pattern & 0x0002) )
		{
			ret_val = TRUE;
			*run_value += (pbi->bit_pattern & 0x0001) + 3;                             
		}
		break; 
             
	case 6:
        // Bit 2 clear
		if ( !(pbi->bit_pattern & 0x0004) )
		{
			ret_val = TRUE;
			*run_value += (pbi->bit_pattern & 0x0003) + 5;                             
		}
		break; 
        
	case 8:
        // Bit 3 clear
		if ( !(pbi->bit_pattern & 0x0008) )
		{
			ret_val = TRUE;
			*run_value += (pbi->bit_pattern & 0x0007) + 9;                             
		}
		break; 

	// This token is a special case repeat token and does not terminate the run
	case 9:
		if ( pbi->bit_pattern == 0x01FF )
		{
			ret_val = FALSE;
			*run_value += 256;                             
			
			// Reset the bit counter and pattern.
			FrArrayDeCodeInit(pbi);
		}
		break;

	case 10:
        // Bit 4 clear
		if ( !(pbi->bit_pattern & 0x0010) )
		{
			ret_val = TRUE;
			*run_value += (pbi->bit_pattern & 0x000F) + 17;                             
		}
		break; 

	case 12:
        // Bit 5 clear
		if ( !(pbi->bit_pattern & 0x0020) )
		{
			ret_val = TRUE;
			*run_value += (pbi->bit_pattern & 0x001F) + 33;                             
		}
		break; 

	case 14:
        // Bit 6 clear
		if ( !(pbi->bit_pattern & 0x0040) )
		{
			ret_val = TRUE;
			*run_value += (pbi->bit_pattern & 0x003F) + 65;                             
		}
		break; 

	case 16:
        // Bit 7 clear
		if ( !(pbi->bit_pattern & 0x0080) )
		{
			ret_val = TRUE;
			*run_value += (pbi->bit_pattern & 0x007F) + 129;                             
		}
		break; 


    default:
    	ret_val = FALSE;
		break;
	}

    return ret_val;
}
/****************************************************************************
 * 
 *  ROUTINE       :     FrArrayCodeSBRun
 *
 *  INPUTS        :     UINT8 bit_value
 *
 *  OUTPUTS       :     UINT8 run_value
 *
 *  RETURNS       :     TRUE when a value has been fully decoded.
 *
 *  FUNCTION      :     Decodes a SB run value
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
BOOL FrArrayDeCodeSBRun(  PB_INSTANCE *pbi, UINT32 bit_value, INT32 * run_value )
{
    BOOL ret_val = FALSE;

    // Add in the new bit value.
    pbi->bits_so_far++;
    pbi->bit_pattern = (pbi->bit_pattern << 1) + (bit_value & 1);
        
    // Coding scheme:
	//	Codeword				RunLength
    //  0                       1
	//	10x					    2-3
	//	110x					4-5
	//	1110xx				    6-9
    //  11110xxx                10-17
    //  111110xxxx              18-33
    //  111111xxxxxxxxxxxx      34-4129
	switch ( pbi->bits_so_far )
	{
	case 1:  
		if ( pbi->bit_pattern == 0 )
		{
			ret_val = TRUE;
			*run_value = 1;                             
		}
		break; 

	case 3:
        // Bit 1 clear
		if ( !(pbi->bit_pattern & 0x0002) )
		{
			ret_val = TRUE;
			*run_value = (pbi->bit_pattern & 0x0001) + 2;                             
		}
		break; 
        
	case 4:
        // Bit 1 clear
		if ( !(pbi->bit_pattern & 0x0002) )
		{
			ret_val = TRUE;
			*run_value = (pbi->bit_pattern & 0x0001) + 4;                             
		}
		break; 
             
	case 6:
        // Bit 2 clear
		if ( !(pbi->bit_pattern & 0x0004) )
		{
			ret_val = TRUE;
			*run_value = (pbi->bit_pattern & 0x0003) + 6;                             
		}
		break; 
        
	case 8:
        // Bit 3 clear
		if ( !(pbi->bit_pattern & 0x0008) )
		{
			ret_val = TRUE;
			*run_value = (pbi->bit_pattern & 0x0007) + 10;                             
		}
		break; 
          
	case 10:
        // Bit 4 clear
		if ( !(pbi->bit_pattern & 0x0010) )
		{
			ret_val = TRUE;
			*run_value = (pbi->bit_pattern & 0x000F) + 18;                             
		}
		break; 

    case 18:
		ret_val = TRUE;
		*run_value = (pbi->bit_pattern & 0x0FFF) + 34;                             
		break; 

    default:
    	ret_val = FALSE;
		break;
	}

    return ret_val;
}

/****************************************************************************
 * 
 *  ROUTINE       :     FrArrayDeCodeBlockRun
 *
 *  INPUTS        :     UINT8 bit_value
 *
 *  OUTPUTS       :     UINT8 run_value
 *
 *  RETURNS       :     TRUE when a value has been fully decoded.
 *
 *  FUNCTION      :     Decodes a block run value
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
BOOL FrArrayDeCodeBlockRun(  PB_INSTANCE *pbi, UINT32 bit_value, INT32 * run_value )
{
    BOOL ret_val = FALSE;

    // Add in the new bit value.
    pbi->bits_so_far++;
    pbi->bit_pattern = (pbi->bit_pattern << 1) + (bit_value & 1);
        
    // Coding scheme:
	//	Codeword				RunLength
	//	0x						1-2
	//	10x						3-4
	//	110x					5-6
	//	1110xx					7-10
	//	11110xx					11-14
	//	11111xxxx				15-30 	

	switch ( pbi->bits_so_far )
	{
	case 2: 
        // If bit 1 is clear
		if ( !(pbi->bit_pattern & 0x0002) )
		{
			ret_val = TRUE;
			*run_value = (pbi->bit_pattern & 0x0001) + 1;
		}           
		break;      

	case 3:  
        // If bit 1 is clear
		if ( !(pbi->bit_pattern & 0x0002) )
		{
			ret_val = TRUE;
			*run_value = (pbi->bit_pattern & 0x0001) + 3;
		}           
		break;      
            
	case 4:
        // If bit 1 is clear
		if ( !(pbi->bit_pattern & 0x0002) )
		{
			ret_val = TRUE;
			*run_value = (pbi->bit_pattern & 0x0001) + 5;
		}           
		break;      
             
	case 6:
        // If bit 2 is clear
		if ( !(pbi->bit_pattern & 0x0004) )
		{
			ret_val = TRUE;
			*run_value = (pbi->bit_pattern & 0x0003) + 7;
		}           
		break;      
        
	case 7:
        // If bit 2 is clear
		if ( !(pbi->bit_pattern & 0x0004) )
		{
			ret_val = TRUE;
			*run_value = (pbi->bit_pattern & 0x0003) + 11;
		}           
		break;      
           
	case 9:
		ret_val = TRUE;
		*run_value = (pbi->bit_pattern & 0x000F) + 15;
		break;      
	}

    return ret_val;
}

/****************************************************************************
 * 
 *  ROUTINE       :     FrArrayUnpackMode
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     A decoded mode token
 *
 *  FUNCTION      :     Unpacks a mode token
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
CODING_MODE FrArrayUnpackMode(PB_INSTANCE *pbi)
{
    // Coding scheme:
	//	Token                       Codeword	    Bits
	//	Entry   0 (most frequent)   0               1
	//	Entry   1       	        10 			    2
	//	Entry   2       	        110 		    3
	//	Entry   3       	        1110 		    4
	//	Entry   4       	        11110 		    5
	//	Entry   5       	        111110 	        6
	//	Entry   6       	        1111110 	    7
	//	Entry   7       	        1111111 	    7

    // Initialise the decoding.
    pbi->bit_pattern = 0;
    pbi->bits_so_far = 0; 

	pbi->bit_pattern = bitread1(&pbi->br);

    // Do we have a match 
    if ( pbi->bit_pattern == 0 )
		return (CODING_MODE)0;

    // Get the next bit
	pbi->bit_pattern = (pbi->bit_pattern << 1) | bitread1(&pbi->br);

    // Do we have a match 
    if ( pbi->bit_pattern == 0x0002 )
		return (CODING_MODE)1;

	pbi->bit_pattern = (pbi->bit_pattern << 1) | bitread1(&pbi->br);

    // Do we have a match 
    if ( pbi->bit_pattern == 0x0006 )
		return (CODING_MODE)2;

	pbi->bit_pattern = (pbi->bit_pattern << 1) | bitread1(&pbi->br);

    // Do we have a match 
    if ( pbi->bit_pattern == 0x000E )
		return (CODING_MODE)3;

	pbi->bit_pattern = (pbi->bit_pattern << 1) | bitread1(&pbi->br);

    // Do we have a match 
    if ( pbi->bit_pattern == 0x001E )
		return (CODING_MODE)4;

	pbi->bit_pattern = (pbi->bit_pattern << 1) | bitread1(&pbi->br);

    // Do we have a match 
    if ( pbi->bit_pattern == 0x003E )
		return (CODING_MODE)5;

	pbi->bit_pattern = (pbi->bit_pattern << 1) | bitread1(&pbi->br);

    // Do we have a match 
    if ( pbi->bit_pattern == 0x007E )
		return (CODING_MODE)6;
    else
		return (CODING_MODE)7;
}

/****************************************************************************
 * 
 *  ROUTINE       :     GetNextBInitSb
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises decode process for a Block RLC stream.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void GetNextBInit(PB_INSTANCE *pbi)
{
	pbi->NextBit = bitread1(&pbi->br);

	// Read run length
	FrArrayDeCodeInit(pbi);               
	while ( FrArrayDeCodeBlockRun( pbi, bitread1(&pbi->br), &pbi->BitsLeft ) == FALSE );
}

/****************************************************************************
 * 
 *  ROUTINE       :     GetNextSbBit
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Value of next bit in stream.
 *
 *  FUNCTION      :     Returns next available bit value from  a Block RLC stream.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT8 GetNextBBit (PB_INSTANCE *pbi)
{
	if ( !pbi->BitsLeft )
	{
		// Toggle the value.  
		pbi->NextBit = ( pbi->NextBit == 1 ) ? 0 : 1;

        // Read next run
		FrArrayDeCodeInit(pbi);
    	while ( FrArrayDeCodeBlockRun( pbi,bitread1(&pbi->br), &pbi->BitsLeft ) == FALSE );
	}

	// Have  read a bit
	pbi->BitsLeft--;
	
	// Return next bit value
	return pbi->NextBit;
}

/****************************************************************************
 * 
 *  ROUTINE       :     GetNextSbInitSb
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises decode process for an SB RLC stream.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void GetNextSbInit(PB_INSTANCE *pbi)
{
	pbi->NextBit = bitread1(&pbi->br);

	// Read run length
	FrArrayDeCodeInit(pbi);               
	while ( FrArrayDeCodeSBRun( pbi,bitread1(&pbi->br), &pbi->BitsLeft ) == FALSE );
}

/****************************************************************************
 * 
 *  ROUTINE       :     GetNextSbBit
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Value of next bit in stream.
 *
 *  FUNCTION      :     Returns next available bit value from an SB RLC stream.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT8 GetNextSbBit (PB_INSTANCE *pbi)
{
	if ( !pbi->BitsLeft )
	{
		// Toggle the value.  
		pbi->NextBit = ( pbi->NextBit == 1 ) ? 0 : 1;

        // Read next run
		FrArrayDeCodeInit(pbi);               
    	while ( FrArrayDeCodeSBRun( pbi, bitread1(&pbi->br), &pbi->BitsLeft ) == FALSE );
	}

	// Have  read a bit
	pbi->BitsLeft--;
	
	// Return next bit value
	return pbi->NextBit;
}

/****************************************************************************
 * 
 *  ROUTINE       :     GetNextMbInitSb
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises decode process for a MB RLC stream.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void GetNextMbInit(PB_INSTANCE *pbi)
{
	pbi->NextBit = bitread1(&pbi->br);
	pbi->BitsLeft = 0;

	// Read run length
	FrArrayDeCodeInit(pbi);               
	while ( FrArrayDeCodeMBRun( pbi,bitread1(&pbi->br), &pbi->BitsLeft ) == FALSE );
}

/****************************************************************************
 * 
 *  ROUTINE       :     GetNextSbBit
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Value of next bit in stream.
 *
 *  FUNCTION      :     Returns next available bit value from a MB RLC stream.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT8 GetNextMbBit (PB_INSTANCE *pbi)
{
	if ( !pbi->BitsLeft )
	{
		// Toggle the value.  
		pbi->NextBit = ( pbi->NextBit == 1 ) ? 0 : 1;

        // Read next run
		FrArrayDeCodeInit(pbi);       
    	while ( FrArrayDeCodeMBRun( pbi, bitread1(&pbi->br), &pbi->BitsLeft ) == FALSE );
	}

	// Decrement bits left in run counter
	pbi->BitsLeft--;
	
	// Return next bit value
	return pbi->NextBit;
}


/****************************************************************************
 * 
 *  ROUTINE       :     ReadBlockPatternInit
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises the block pattern reader
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ReadBlockPatternInit (PB_INSTANCE *pbi)
{
	pbi->BlockPatternPredictor = 0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     ReadNextBlockPattern
 *
 *  INPUTS        :     None. 
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     A pattern of coded or uncoded blocks for a macro block.
 *
 *  FUNCTION      :     Returns the block pattern for the next macro block.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT8 ReadNextBlockPattern (PB_INSTANCE *pbi)
{
	UINT8 BlockPattern = 0;
	UINT8 Bitpattern = 0;
	UINT32 BitCount = 3;

	// Read three bits and test to see if we have a valid token.
	Bitpattern = bitread(&pbi->br, 3);

	// Test pattern to see if is a valid token.
	BlockPattern = BlockDecode1[pbi->BlockPatternPredictor][Bitpattern];

	// if pattern was not a valid token
	if ( !BlockPattern )
	{
		BitCount++;
		Bitpattern = (Bitpattern << 1) + bitread1(&pbi->br);

		// Test pattern to see if is a valid token.
		BlockPattern = BlockDecode2[pbi->BlockPatternPredictor][Bitpattern];

		if ( !BlockPattern )
		{
			BitCount++;
			Bitpattern = (Bitpattern << 1) + bitread1(&pbi->br);
			BlockPattern = BlockDecode3[pbi->BlockPatternPredictor][Bitpattern];
		}
	}

	// Update the entropy predictor for next time.
	pbi->BlockPatternPredictor = BPPredictor[BlockPattern];

	return BlockPattern;
}

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
*   Module Title :     BlockMapping.c
*
*   Description  :     Common block mapping functions
*
*****************************************************************************
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */
#include "type_aliases.h"
#include "BlockMapping.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
 

/****************************************************************************
*  Module statics.
*****************************************************************************
*/        

INT32 MBOrderMap[4] = { 0, 2, 3, 1 };


// Old block mapping (encoder version < 6)
INT32 BlockOrderMap1[4][4] =	{	{ 0, 1, 3, 2 },
									{ 0, 2, 3, 1 },       
									{ 0, 2, 3, 1 },
									{ 3, 2, 0, 1 }
								};
// New block mapping (encoder version >= 6)
INT32 BlockOrderMap2[4][4] =	{	{ 0, 1, 2, 3 },
									{ 0, 1, 2, 3 },       
									{ 0, 1, 2, 3 },
									{ 0, 1, 2, 3 }
								};
                
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

/****************************************************************************
 * 
 *  ROUTINE       :     QuadMapToIndex
 *
 *  INPUTS        :     None
 *                   
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Index into a linear fragment array.
 *
 *  FUNCTION      :     Converts a [SB][MB][B] description into a fragment index.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
inline INT32 QuadMapToIndex1( INT32	(*BlockMap)[4][4], UINT32 SB, UINT32 MB, UINT32 B )
{
    return BlockMap[SB][MBOrderMap[MB]][BlockOrderMap1[MB][B]];
}
inline INT32 QuadMapToIndex2( INT32	(*BlockMap)[4][4], UINT32 SB, UINT32 MB, UINT32 B )
{
    return BlockMap[SB][MBOrderMap[MB]][BlockOrderMap2[MB][B]];
}

/****************************************************************************
 * 
 *  ROUTINE       :     QuadMapToMBTopLeft
 *
 *  INPUTS        :     None
 *                   
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Index into a linear fragment array.
 *
 *  FUNCTION      :     Converts a [SB][MB] description into the index of the
 *                      top left fragment in the macro block.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
inline INT32 QuadMapToMBTopLeft( INT32	(*BlockMap)[4][4], UINT32 SB, UINT32 MB )
{
    return BlockMap[SB][MBOrderMap[MB]][0];
}

/****************************************************************************
 * 
 *  ROUTINE       :     CreateBlockMapping
 *
 *  INPUTS        :     UINT32 image_width
 *						UINT32 image_height
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
void CreateBlockMapping ( INT32	(*BlockMap)[4][4], UINT32 YSuperBlocks, UINT32 UVSuperBlocks, UINT32 HFrags, UINT32 VFrags )
{
	UINT32 i, j;			// Loop counters.

	// Initialise mapping array
	for ( i=0; i<YSuperBlocks + UVSuperBlocks * 2; i++ )
	{
		for ( j=0; j<4; j++ )
		{
			BlockMap[i][j][0] = -1;
			BlockMap[i][j][1] = -1;
			BlockMap[i][j][2] = -1;
			BlockMap[i][j][3] = -1;
		}
	}

	// Create mappings for each component
	CreateMapping ( BlockMap, 0, 0, HFrags, VFrags );
	CreateMapping ( BlockMap, YSuperBlocks, HFrags*VFrags, HFrags/2, VFrags/2 );
	CreateMapping ( BlockMap, YSuperBlocks + UVSuperBlocks, (HFrags*VFrags*5)/4, HFrags/2, VFrags/2 );
}


/****************************************************************************
 * 
 *  ROUTINE       :     CreateMapping
 *
 *  INPUTS        :     UINT32 FirstSB
 *						UINT32 FirstFrag
 *						UINT32 HFrags
 *						UINT32 VFrags
 *
 *  OUTPUTS       :     INT32 BlockMap[][][]
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Creates mapping from quad-tree to linear fragment index
 *						for specified dimensions.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void CreateMapping ( INT32 (*BlockMap)[4][4], UINT32 FirstSB, UINT32 FirstFrag, UINT32 HFrags, UINT32 VFrags )
{
	UINT32 i, j;				// Loop counters
	UINT32 xpos;				// X co-ordinate in Block units
	UINT32 ypos;				// Y co-ordinate in Block units
	UINT32 SBrow, SBcol;		// Super-Block location
	UINT32 SBRows, SBCols;		// Number of Super-Block rows/cols
	UINT32 MB, B;				// Macro-Block, Block number
	
	UINT32 SB=FirstSB;			// Super-Block number
	UINT32 FragIndex=FirstFrag;	// Fragment number

	// Set Super-Block dimensions
	SBRows = VFrags/4 + ( VFrags%4 ? 1 : 0 );
	SBCols = HFrags/4 + ( HFrags%4 ? 1 : 0 );

	// Map each Super-Block
	for ( SBrow=0; SBrow<SBRows; SBrow++ )
	{
		for ( SBcol=0; SBcol<SBCols; SBcol++ )
		{
			// Y co-ordinate of Super-Block in Block units
			ypos = SBrow<<2;

			// Map Blocks within this Super-Block
			for ( i=0; (i<4) && (ypos<VFrags); i++, ypos++ )
			{
				// X co-ordinate of Super-Block in Block units
				xpos = SBcol<<2;
				
				for ( j=0; (j<4) && (xpos<HFrags); j++, xpos++ )
				{
					if ( i<2 )
					{
						MB = ( j<2 ? 0 : 1 );
					}
					else
					{
						MB = ( j<2 ? 2 : 3 );
					}

					if ( i%2 )
					{
						B = ( j%2 ? 3 : 2 );
					}
					else
					{
						B = ( j%2 ? 1 : 0 );
					}

					// Set mapping and move to next fragment
					BlockMap[SB][MB][B] = FragIndex++;
				}

				// Move to first fragment in next row in Super-Block
				FragIndex += HFrags-j;
			}

			// Move on to next Super-Block
			SB++;
			FragIndex -= i*HFrags-j;
		}

		// Move to first Super-Block in next row
		FragIndex += 3*HFrags;
	}
}



/****************************************************************************
 * 
 *  ROUTINE       :     GetFragIndex
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Gets a pixel index for the first pixel of the given fragment.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
__inline UINT32 GetFragIndex( UINT32 * pixel_index_table, UINT32 FragmentNo )
{   
    return pixel_index_table[ FragmentNo ];
}

/****************************************************************************
 * 
 *  ROUTINE       :     ReconGetFragIndex
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Gets a pixel index for the first pixel of the given fragment
 *                      in a reconstruction buffer.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
__inline UINT32 ReconGetFragIndex( UINT32 * recon_pixel_index_table, UINT32 FragmentNo )
{   
    return recon_pixel_index_table[ FragmentNo ];
}


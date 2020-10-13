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
*   Module Title :     DFrameR.C
*
*   Description  :     Functions to read
*
*
*****************************************************************************
*/

/****************************************************************************
*  Header Frames
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "vfw_pb_interface.h"
#include "pbdll.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

#define START_SIZE  0
#define END_SIZE    1

#define READ_BUFFER_EMPTY_WAIT  20
 
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

/****************************************************************************
*  Module Statics
*****************************************************************************
*/              
const UINT32 loMaskTbl_VP31[] = { 0,
	1, 3, 7, 15,
	31, 63, 127, 255,
	0x1ff, 0x3ff, 0x7ff, 0xfff,
	0x1fff, 0x3fff, 0x7fff, 0xffff,
	0x1FFFF, 0x3FFFF, 0x7FFFF, 0xfFFFF,
	0x1fFFFF, 0x3fFFFF, 0x7fFFFF, 0xffFFFF,
	0x1ffFFFF, 0x3ffFFFF, 0x7ffFFFF, 0xfffFFFF,
	0x1fffFFFF, 0x3fffFFFF, 0x7fffFFFF, 0xffffFFFF
};

const UINT32 hiMaskTbl_VP31[] = { 0,
	0x80000000, 0xC0000000, 0xE0000000, 0xF0000000,
	0xF8000000, 0xFC000000, 0xFE000000, 0xFF000000,
	0xFF800000, 0xFFC00000, 0xFFE00000, 0xFFF00000,
	0xFFF80000, 0xFFFC0000, 0xFFFE0000, 0xFFFF0000,
	0xFFFF8000, 0xFFFFC000, 0xFFFFE000, 0xFFFFF000,
	0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00, 0xFFFFFF00,
	0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0, 0xFFFFFFF0,
	0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE, 0xFFFFFFFF
};


/****************************************************************************
*  Forward References.
*****************************************************************************
*/              
BOOL PbBuildBitmapHeader( PB_INSTANCE *pbi, UINT32 ImageWidth, UINT32 ImageHeight );
BOOL ValidateImageSize( UINT32 Width, UINT32 Height );


/****************************************************************************
*  Imports
*****************************************************************************
*/              


//extern UINT8  * bmp_dptr0;

/****************************************************************************
 * 
 *  ROUTINE       :     LoadFrame
 *
 *  INPUTS        :     None 
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     FALSE if an Error is detected or the frame is empty else TRUE.
 *
 *  FUNCTION      :     Loads a frame and decodes the fragment arrays.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
BOOL LoadFrame(PB_INSTANCE *pbi)
{ 
    BOOL RetVal = TRUE;           

    // Initialise the bit extractor.
    //ExtractInit(pbi);

    // Load the frame header (including the frame size).     
    if ( LoadFrameHeader(pbi) )
    {
        // Read in the updated block map 
        ReadAndUnPackDFArray( pbi );
    }
    else
    {
        RetVal = FALSE;
    }

    return RetVal;
}


/****************************************************************************
 * 
 *  ROUTINE       :     LoadFrameHeader
 *
 *  INPUTS        :     fptr - The file pointer for the data file.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     FALSE if and Error is detected else TRUE.
 *
 *  FUNCTION      :     Loads and interprets the frame header.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

// VFW codec version
#define ROUNDUP32(X) ( ( ( (unsigned long) X ) + 31 )&( 0xFFFFFFE0 ) ) 
BOOL LoadFrameHeader(PB_INSTANCE *pbi)
{
	UINT8  VersionByte0;    // Must be 0 for VP30b and later
    UINT8  DctQMask;
//    UINT8  SpareBits;       // Spare cfg bits
	UINT8  Unused;

    BOOL   RetVal = TRUE;

    // Is the frame and inter frame or a key frame
    pbi->FrameType = bitread1(&pbi->br);
    
	// unused bit
    Unused = bitread1(&pbi->br);

    // Quality (Q) index
    DctQMask = (UINT8)bitread( &pbi->br,   6 );

	// If the frame was a base frame then read the frame dimensions and build a bitmap structure. 
	if ( (pbi->FrameType == BASE_FRAME) )
	{
        // Read the frame dimensions bytes (0,0 indicates vp31 or later)
    	VersionByte0 = (UINT8)bitread( &pbi->br,   8 );
	    pbi->Vp3VersionNo = (UINT8)bitread( &pbi->br,   5 );

//		if(pbi->Vp3VersionNo > CURRENT_DECODE_VERSION)
//		{
//			RetVal = FALSE;
//			return RetVal;
//		}
		// Initialise version specific quantiser values
		InitQTables( pbi );

        // Read the type / coding method for the key frame.
        Unused = (UINT8)bitread( &pbi->br,   3 );

//        SpareBits = (UINT8)bitread( &pbi->br,   2 );


        // Select huffman tables
		SelectHuffmanSet( pbi );

    }
	
	// Set this frame quality value from Q Index
    pbi->ThisFrameQualityValue = pbi->QThreshTable[DctQMask];

    return RetVal;                    
}

/****************************************************************************
 * 
 *  ROUTINE       :     SetFrameType
 *
 *  INPUTS        :     A Frame type.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Sets the current frame type.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void SetFrameType( PB_INSTANCE *pbi,UINT8 FrType )
{ 
    /* Set the appropriate frame type according to the request. */
    switch ( FrType )
    {  
    
    case BASE_FRAME:
        pbi->FrameType = FrType;
        break;
        
    default:
        pbi->FrameType = FrType;
        break;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     GetFrameType
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     The current frame type.
 *
 *  FUNCTION      :     Gets the current frame type.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT8 GetFrameType(PB_INSTANCE *pbi)
{
    return pbi->FrameType; 
}


/****************************************************************************
 * 
 *  ROUTINE       :     Read32FromBuffer
 *
 *  INPUTS        :     UINT8 * Data
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Reads 32 bits from the input buffer for processing and
 *                      reverts data to little endian.
 *                          
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

#	define BitsAreBigEndian 1
#	if BitsAreBigEndian
#		define NextWord \
{ br->remainder = (br->position[0] << 24) + (br->position[1] << 16) + (br->position[2] << 8) + br->position[3];  br->position += 4;}
#	else
#		define NextWord \
{ br->remainder = (br->position[3] << 24) + (br->position[2] << 16) + (br->position[1] << 8) + br->position[0];  br->position += 4;}
#	endif


/****************************************************************************
 * 
 *  ROUTINE       :     bitread
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     returns specified number of bits from reader in uint32
 *
 *
 *  FUNCTION      :     Extracts bits from the encoded data buffer 
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
__inline UINT32 bitread(BITREADER *br, int bits)
{
	UINT32 z = 0;
	br->remainder &= loMaskTbl_VP31[ br->bitsinremainder];
	
	if( (bits -= br->bitsinremainder) > 0) 
	{
		z |= br->remainder << bits;
		NextWord
			bits -= 32;
	}
	return z | br->remainder >> (br->bitsinremainder = -bits);
}

/****************************************************************************
 * 
 *  ROUTINE       :     bitread1
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     1 if the next bit is set else 0. (or the number of decoded bytes)
 *
 *
 *  FUNCTION      :     Extracts bits from the encoded data buffer one at a time.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
__inline UINT32 bitread1(BITREADER *br) 
{
	if( br->bitsinremainder)
		return (br->remainder >> --br->bitsinremainder) & 1;
	NextWord
		return br->remainder  >> (br->bitsinremainder = 31);
}

#	undef NextWord


/****************************************************************************
 * 
 *  ROUTINE       :     PbBuildBitmapHeader
 *
 *  INPUTS        :     UINT32 ImageWidth and Image Height
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Builds a bitmap of the given size etc.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
BOOL PbBuildBitmapHeader( PB_INSTANCE *pbi, UINT32 ImageWidth, UINT32 ImageHeight )
{

    /* Initialise the other bitmap details. */    
    if(!InitFrameDetails(pbi))
        return FALSE;

    return TRUE;
}


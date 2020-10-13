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
*   Module Title :     vfwpbdll_if.c
*
*   Description  :     Video codec demo playback dll interface
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */
#include <stdio.h> 
#include "Huffman.h"
#include "pbdll.h"
#include "blockmapping.h"
#include <math.h>
#include "vfw_pb_interface.h"
//#include "VP31DVERSION.h"
//#define CommentString "\nON2.COM VERSION VP31D " VP31DVERSION "\n"
//#pragma comment(exestr,CommentString)

/****************************************************************************
*  Implicit Imports
*****************************************************************************
*/        

//extern unsigned int CPUFrequency;

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
 
/****************************************************************************
*  Exported data structures.
*****************************************************************************
*/        


/****************************************************************************
*  Module statics.
*****************************************************************************
*/        


//static const char vp31dVersion[] = VP31DVERSION;

/****************************************************************************
*  Imports
*****************************************************************************
*/
extern void CCONV ScaleOrCenter( PB_INSTANCE (*pbi), YUV_BUFFER_CONFIG * YuvConfig );

#if !defined(MACPPC)   
#if defined(POSTPROCESS)
extern void PostProcess(PB_INSTANCE *pbi);
const unsigned long PP_MACHINE_LOWLIMIT = 350; //Lowest CPU (MHz) to enable PostProcess
const unsigned long PP_MACHINE_MIDLIMIT = 400; //Lowest CPU (MHz) to enable PostProcess
const unsigned long PP_MACHINE_TOPLIMIT = 590; //Lowest CPU (MHz) to enable PostProcess
/*
const unsigned long PP_MACHINE_LOWLIMIT = 340; //Lowest CPU (MHz) to enable PostProcess
const unsigned long PP_MACHINE_MIDLIMIT = 440; //Lowest CPU (MHz) to enable PostProcess
*/
#endif
#endif

extern void InitialiseConfiguration(PB_INSTANCE *pbi);
//extern UINT32 FrameSize;  /* The number of bytes in the frame (read from the frame header). */


/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

/****************************************************************************
 * 
 *  ROUTINE       :     LoadAndDecode
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Loads and decodes a frame.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
int LoadAndDecode(PB_INSTANCE *pbi)
{    
    BOOL    LoadFrameOK;


    /* Load the next frame. */
    LoadFrameOK = LoadFrame(pbi); 
            
    if ( (pbi->ThisFrameQualityValue != pbi->LastFrameQualityValue) )
    {
        /* Initialise DCT tables. */
        UpdateQ( pbi, pbi->ThisFrameQualityValue );  

        pbi->LastFrameQualityValue = pbi->ThisFrameQualityValue;    
    }   
    
   
    /* Decode the data into the fragment buffer. */
    DecodeData(pbi);                    

    return 0;

}                          



/****************************************************************************
 * 
 *  ROUTINE       :     StartDecoder
 *
 *  INPUTS        :     The handle of the display window.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     TRUE if succeeds else FALSE.
 *
 *  FUNCTION      :     Starts the compressor grabber
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
BOOL CCONV StartDecoder( PB_INSTANCE **pbi, UINT32 ImageWidth, UINT32 ImageHeight )
{
		// set up our structure holding all formerly global information about a playback instance
		*pbi = CreatePBInstance();

        // Validate the combination of height and width.
		(*pbi)->Configuration.VideoFrameWidth = ImageWidth;
		(*pbi)->Configuration.VideoFrameHeight = ImageHeight;
        
//        (*pbi)->ProcessorFrequency = CPUFrequency;
		// Fills in fragment counts as well
		if(!PbBuildBitmapHeader( (*pbi), (*pbi)->Configuration.VideoFrameWidth, (*pbi)->Configuration.VideoFrameHeight ))
        {
            DeletePBInstance(pbi);
            return FALSE;
        }

        /* Initialise the input buffer pointers. */

	    /* Set last_dct_thresh to an illegal value to make sure the
	    *  Q tables are initialised for the new video sequence. 
	    */
	    (*pbi)->LastFrameQualityValue = 0;

	    // Set up various configuration parameters.
	    InitialiseConfiguration(*pbi);


	    return TRUE;


}

/****************************************************************************
 * 
 *  ROUTINE       :     DecodeFrameToYUV
 *
 *  INPUTS        :     UINT8 * VideoBufferPtr
 *                              Compressed input video data
 *
 *                      UINT32  ByteCount 
 *                              Number of bytes compressed data in buffer. *  
 *
 *                      UINT32  Height and width of image to be decoded
 *
 *  OUTPUTS       :     None
 *                      None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Decodes a frame into the internal YUV reconstruction buffer.
 *                      Details of this buffer can be obtained by calling GetYUVConfig().
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
int CCONV DecodeFrameToYUV( PB_INSTANCE (*pbi), char * VideoBufferPtr, unsigned int ByteCount,
                             UINT32 ImageWidth,     UINT32 ImageHeight )
{
        // Sanity check to make sure that the decoder has been initialised
        // If it has not then we must initialise it before we proceed.

		pbi->br.bitsinremainder = 0;
		pbi->br.position = (cuchar *)VideoBufferPtr;

   	    // Decode the frame. 
	    LoadAndDecode(pbi);

    return 0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     StopDecoder
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None .
 *
 *  FUNCTION      :     Stops the encoder and grabber
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
BOOL CCONV StopDecoder(PB_INSTANCE **pbi)
{
		if(*pbi)
		{
			DeleteFragmentInfo(* pbi);
			DeleteFrameInfo(* pbi);
			DeletePBInstance(pbi);
		
	        return TRUE;
		}
	return TRUE;
}


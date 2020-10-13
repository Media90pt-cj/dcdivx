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
*   Module Title :     PB_Globals.c
*
*   Description  :     Video CODEC Demo: playback dll global declarations
*
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "pbdll.h"
#include <string.h>
#include <stdlib.h>



//#include "codec_common_interface.h"

/****************************************************************************
*  Imports
*****************************************************************************
*/        
 
//extern unsigned long GetProcessorFrequency();

                
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/
// Process Frequency
//unsigned int CPUFrequency;

// Truth table to indicate if the given mode uses motion estimation
BOOL ModeUsesMC[MAX_MODES] = { FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE };

/****************************************************************************
 * 
 *  ROUTINE       :     DeleteTmpBuffers
 *
 *
 *  INPUTS        :     Instance of PB to be initialized
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Initializes the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DeleteTmpBuffers(PB_INSTANCE * pbi)
{

	if(pbi->ReconDataBufferAlloc)
		free(pbi->ReconDataBufferAlloc);
	if(pbi->DequantBufferAlloc)
		free(pbi->DequantBufferAlloc);
	if(pbi->TmpDataBufferAlloc)
		free(pbi->TmpDataBufferAlloc);
	if(pbi->TmpReconBufferAlloc)
		free(pbi->TmpReconBufferAlloc);
	if(pbi->dequant_Y_coeffsAlloc)
		free(pbi->dequant_Y_coeffsAlloc);
	if(pbi->dequant_UV_coeffsAlloc)
		free(pbi->dequant_UV_coeffsAlloc);
	if(pbi->dequant_Inter_coeffsAlloc);
		free(pbi->dequant_Inter_coeffsAlloc);
	if(pbi->dequant_InterUV_coeffsAlloc)
		free(pbi->dequant_InterUV_coeffsAlloc);

    pbi->dequant_UV_coeffsAlloc = 0;
	pbi->ReconDataBufferAlloc=0;
	pbi->DequantBufferAlloc = 0;
	pbi->TmpDataBufferAlloc = 0;
	pbi->TmpReconBufferAlloc = 0;
	pbi->dequant_Y_coeffsAlloc = 0;
	pbi->dequant_UV_coeffsAlloc = 0;
	pbi->dequant_InterUV_coeffsAlloc = 0;
	pbi->dequant_Inter_coeffsAlloc = 0;
    pbi->ScaleBuffer = 0;
	pbi->ReconDataBuffer=0;
	pbi->DequantBuffer = 0;
	pbi->TmpDataBuffer = 0;
	pbi->TmpReconBuffer = 0;
	pbi->dequant_Y_coeffs = 0;
	pbi->dequant_UV_coeffs = 0;
	pbi->dequant_Inter_coeffs = 0;
    pbi->dequant_UV_coeffs = 0;
	pbi->dequant_InterUV_coeffs = 0;
	

}


/****************************************************************************
 * 
 *  ROUTINE       :     AllocateTmpBuffers
 *
 *
 *  INPUTS        :     Instance of PB to be initialized
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Initializes the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
#define ROUNDUP32(X) ( ( ( (unsigned long) X ) + 31 )&( 0xFFFFFFE0 ) )
BOOL AllocateTmpBuffers(PB_INSTANCE * pbi)
{

	// clear any existing info
	DeleteTmpBuffers(pbi);

	// Adjust the position of all of our temporary
	pbi->ReconDataBufferAlloc      = (INT16 *)malloc(32+64*sizeof(INT16));
//    if(!pbi->ReconDataBufferAlloc)      { DeleteTmpBuffers(pbi); return FALSE;};

    pbi->DequantBufferAlloc        = (INT16 *)malloc(32 + 64 * sizeof(INT16));
//    if(!pbi->DequantBufferAlloc)        { DeleteTmpBuffers(pbi); return FALSE;};

    pbi->TmpDataBufferAlloc        = (INT16 *)malloc(32 + 64 * sizeof(INT16));
//    if(!pbi->TmpDataBufferAlloc)        { DeleteTmpBuffers(pbi); return FALSE;};

    pbi->TmpReconBufferAlloc       = (INT16 *)malloc(32 + 64 * sizeof(INT16));
//    if(!pbi->TmpReconBufferAlloc)       { DeleteTmpBuffers(pbi); return FALSE;};

    pbi->dequant_Y_coeffsAlloc     = (INT16 *)malloc(32 + 64 * sizeof(INT16));
//    if(!pbi->dequant_Y_coeffsAlloc)     { DeleteTmpBuffers(pbi); return FALSE;};

    pbi->dequant_UV_coeffsAlloc    = (INT16 *)malloc(260+32 + 64 * sizeof(INT16));
//    if(!pbi->dequant_UV_coeffsAlloc)    { DeleteTmpBuffers(pbi); return FALSE;};

    pbi->dequant_Inter_coeffsAlloc = (INT16 *)malloc(32 + 64 * sizeof(INT16));
//    if(!pbi->dequant_Inter_coeffsAlloc) { DeleteTmpBuffers(pbi); return FALSE;};

    pbi->dequant_InterUV_coeffsAlloc = (INT16 *)malloc(32 + 64 * sizeof(INT16));
//    if(!pbi->dequant_InterUV_coeffsAlloc) { DeleteTmpBuffers(pbi); return FALSE;};

    pbi->ReconDataBuffer           = (INT16 *)ROUNDUP32(pbi->ReconDataBufferAlloc);
    pbi->DequantBuffer             = (INT16 *)ROUNDUP32(pbi->DequantBufferAlloc);
    pbi->TmpDataBuffer             = (INT16 *)ROUNDUP32(pbi->TmpDataBufferAlloc);
    pbi->TmpReconBuffer            = (INT16 *)ROUNDUP32(pbi->TmpReconBufferAlloc);
    pbi->dequant_Y_coeffs          = (Q_LIST_ENTRY *)ROUNDUP32(pbi->dequant_Y_coeffsAlloc);
    pbi->dequant_UV_coeffs         = (Q_LIST_ENTRY *)ROUNDUP32(pbi->dequant_UV_coeffsAlloc);
    pbi->dequant_Inter_coeffs      = (Q_LIST_ENTRY *)ROUNDUP32(pbi->dequant_Inter_coeffsAlloc);
    pbi->dequant_InterUV_coeffs      = (Q_LIST_ENTRY *)ROUNDUP32(pbi->dequant_InterUV_coeffsAlloc);

    return TRUE;

}

/****************************************************************************
 * 
 *  ROUTINE       :     DeletePBInstance
 *
 *
 *  INPUTS        :     Instance of PB to be deleted
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     frees the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DeletePBInstance(PB_INSTANCE **pbi)
{
	// clear any existing info
    if(*pbi)
    {
		DeleteTmpBuffers(*pbi);
    }

	free(*pbi);
	*pbi=0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     CreatePBInstance
 *
 *
 *  INPUTS        :     Instance of PB to be initialized
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Initializes the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
PB_INSTANCE * CreatePBInstance(void)
{
	PB_INSTANCE *pbi=0;
	CONFIG_TYPE ConfigurationInit = 
	{
		0,0,0,0,
	    8,8,
	};


	int pbi_size = sizeof(PB_INSTANCE);
	pbi=(PB_INSTANCE *) malloc(pbi_size);
//    if(!pbi)
//    {
//        return 0;
//    }

	// initialize whole structure to 0
	memset((unsigned char *) pbi, 0, sizeof(PB_INSTANCE));
	
	memcpy((void *) &pbi->Configuration, (void *) &ConfigurationInit, sizeof(CONFIG_TYPE));

	AllocateTmpBuffers(pbi);
//    {
//        free(pbi);
//        return 0;
//    }

	// variables needing initialization (not being set to 0)

//	pbi->KeyFrameType = DCT_KEY_FRAME;	
	return pbi;
}


/****************************************************************************
 * 
 *  ROUTINE       :     VPInitLibrary
 *
 *
 *  INPUTS        :     init VP library
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Fully initializes the playback library
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT8 LimitVal_VP31[VAL_RANGE * 3];

void VPInitLibrary(void)
{
 	INT32 i;
   
    #define Clamp255( x)	(unsigned char) ( (x) < 0 ? 0 : ( (x) <= 255 ? (x) : 255 ) )


	LimitVal_VP31[0] = 0;
	for( i = 0; i < VAL_RANGE * 3; i++) {
		int x = i - VAL_RANGE;
		LimitVal_VP31[ i] = Clamp255( x);
	}
    #undef Clamp255

	CreateHuffmanTrees();

}

/*********************************************************/


/****************************************************************************
 * 
 *  ROUTINE       :     VPDeinitLibrary
 *
 *
 *  INPUTS        :     init VP library
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Fully initializes the playback library
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VPDeInitLibrary(void)
{
	DestroyHuffmanTrees();
}




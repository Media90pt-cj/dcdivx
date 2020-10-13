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
*   Module Title :     SystemDependant.c
*
*   Description  :     Miscellaneous system dependant functions
*
*****************************************************************************
*/

/*******************************************3*********************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */
#include <string.h>
//#include <time.h>
#include <stdlib.h>   
#include <stdio.h>  

#include "pbdll.h"
#include "Quantize.h"
#include "Reconstruct.h"
#include "dct.h"


/****************************************************************************
*  Explicit imports
*****************************************************************************
*/

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
 

/****************************************************************************
*  Module statics.
*****************************************************************************
*/        

              
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/


/****************************************************************************
*  Functions
*****************************************************************************
*/
extern void UnPackVideo(PB_INSTANCE *pbi);



/****************************************************************************
 * 
 *  ROUTINE       :     MachineSpecificConfig
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Checks for machine specifc features such as MMX support 
 *                      sets approipriate flags and function pointers.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DMachineSpecificConfig(PB_INSTANCE *pbi)
{
    UINT32 FeatureFlags = 0;
    BOOL   CPUID_Supported = TRUE;   // Is the CPUID instruction supported
    BOOL   TestMmx = TRUE;
	UINT32 i;
    
	//setup the function pointers for inverse dct
	for(i=0;i<=64;i++)
	{
		if(i<=1)pbi->idct[i]=IDct1;
		else if(i<=10)pbi->idct[i]=IDct10;
		else pbi->idct[i]=IDctSlow;
	}

    // Reconstruction functions
    pbi->ReconIntra = ScalarReconIntra;
    pbi->ReconInter = ScalarReconInter;
    pbi->ReconInterHalfPixel2 = ScalarReconInterHalfPixel2;
	pbi->ClearDownQFrag = ClearDownQFragData;
	
	pbi->ExtractToken=ExtractToken;
	pbi->UnPackVideo=UnPackVideo;
	pbi->CopyBlock = CopyBlock;

    pbi->FilterHoriz = FilterHoriz_Generic;
    pbi->FilterVert = FilterVert_Generic;
    pbi->SetupBoundingValueArray = SetupBoundingValueArray_Generic;

    pbi->BuildQuantIndex = BuildQuantIndex_Generic;

        
}


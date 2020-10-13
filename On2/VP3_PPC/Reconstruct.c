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
*   Module Title :     Reconstruct.c
*
*   Description  :     Block Reconstruction functions
*
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              // Strict type checking. 

#include "pbdll.h"

#include "Reconstruct.h"


/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

/****************************************************************************
*  Explicit imports
*****************************************************************************
*/

               
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

/****************************************************************************
*  Forward References
*****************************************************************************
*/  

void SatUnsigned8( UINT8 * ResultPtr, INT16 * DataBlock, UINT32 ResultLineStep, UINT32 DataLineStep );

/****************************************************************************
*  Module Variables.
*****************************************************************************
*/  



/****************************************************************************
 * 
 *  ROUTINE       :     ScalarReconIntra
 *
 *  INPUTS        :     INT16 *  ChangePtr
 *                               Pointer to the change data
 *
 *                      UINT32   LineStep
 *                               Line Length in pixels in recon and ref images
 *                               
 *
 *                     
 *
 *  OUTPUTS       :     UINT8 *  ReconPtr
 *                               The reconstruction
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Reconstructs an intra block - scalar version
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ScalarReconIntra( PB_INSTANCE *pbi, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep )
{
    UINT32 i;
	INT32 *TmpDataPtr = (INT32*)pbi->TmpDataBuffer;
	int bhw2=BLOCK_HEIGHT_WIDTH>>1;

    for ( i = 0; i < BLOCK_HEIGHT_WIDTH; i++ )
   	{	
        // Convert the data back to 8 bit unsigned
        TmpDataPtr[0] = ( ChangePtr[0] + 128 )|(INT32)(( ChangePtr[1] + 128 )<<16);
        //TmpDataPtr[1] = ( ChangePtr[1] + 128 );
        TmpDataPtr[1] = ( ChangePtr[2] + 128 )|(INT32)(( ChangePtr[3] + 128 )<<16);
        //TmpDataPtr[3] = ( ChangePtr[3] + 128 );
        TmpDataPtr[2] = ( ChangePtr[4] + 128 )|(INT32)(( ChangePtr[5] + 128 )<<16);
        //TmpDataPtr[5] = ( ChangePtr[5] + 128 );
        TmpDataPtr[3] = ( ChangePtr[6] + 128 )|(INT32)(( ChangePtr[7] + 128 )<<16);
        //TmpDataPtr[7] = ( ChangePtr[7] + 128 );

        TmpDataPtr += bhw2;
        ChangePtr += BLOCK_HEIGHT_WIDTH;
    }

    // Saturate the output to unsigend 8 bit values
    SatUnsigned8( ReconPtr, pbi->TmpDataBuffer, LineStep, BLOCK_HEIGHT_WIDTH );
}

/****************************************************************************
 * 
 *  ROUTINE       :     ScalarReconInter
 *
 *  INPUTS        :     UINT8 *  RefPtr
 *                               The last frame reference
 *
 *                      INT16 *  ChangePtr
 *                               Pointer to the change data
 *
 *                      UINT32   LineStep
 *                               Line Length in pixels in recon and ref images
 *                               
 *
 *                     
 *
 *  OUTPUTS       :     UINT8 *  ReconPtr
 *                               The reconstruction
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Reconstructs data fro last data and change
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ScalarReconInter( PB_INSTANCE *pbi, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep )
{
    UINT32 i;
	INT32 *TmpDataPtr = (INT32*) pbi->TmpDataBuffer;
	int bhw2=BLOCK_HEIGHT_WIDTH>>1;
    for ( i = 0; i < BLOCK_HEIGHT_WIDTH; i++ )
   	{	
		// Form each row
   	    TmpDataPtr[0] = (INT32)(RefPtr[0] + ChangePtr[0])|((INT32)(RefPtr[1] + ChangePtr[1])<<16);
//   	    TmpDataPtr[1] = (INT16)(RefPtr[1] + ChangePtr[1]);
   	    TmpDataPtr[1] = (INT32)(RefPtr[2] + ChangePtr[2])|((INT32)(RefPtr[3] + ChangePtr[3])<<16);
//   	    TmpDataPtr[3] = (INT16)(RefPtr[3] + ChangePtr[3]);
   	    TmpDataPtr[2] = (INT32)(RefPtr[4] + ChangePtr[4])|((INT32)(RefPtr[5] + ChangePtr[5])<<16);
//   	    TmpDataPtr[5] = (INT16)(RefPtr[5] + ChangePtr[5]);
   	    TmpDataPtr[3] = (INT32)(RefPtr[6] + ChangePtr[6])|((INT32)(RefPtr[7] + ChangePtr[7])<<16);
//   	    TmpDataPtr[7] = (INT16)(RefPtr[7] + ChangePtr[7]);

        // Next row of Block
		ChangePtr += BLOCK_HEIGHT_WIDTH;             // Local 8x8 data structure
        TmpDataPtr += bhw2;
        RefPtr += LineStep; 
    }

    // Saturate the output to unsigend 8 bit values
    SatUnsigned8( ReconPtr, pbi->TmpDataBuffer, LineStep, BLOCK_HEIGHT_WIDTH );

}

/****************************************************************************
 * 
 *  ROUTINE       :     ScalarReconInterHalfPixel2
 *
 *  INPUTS        :     UINT8 *  RefPtr1, RefPtr2
 *                               The last frame reference
 *
 *                      INT16 *  ChangePtr
 *                               Pointer to the change data
 *
 *                      UINT32   LineStep
 *                               Line Length in pixels in recon and ref images
 *, 
 *
 *  OUTPUTS       :     UINT8 *  ReconPtr
 *                               The reconstruction
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Reconstructs data from half pixel reference data and change. 
 *                      Half pixel data interpolated from 2 references.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ScalarReconInterHalfPixel2( PB_INSTANCE *pbi, UINT8 * ReconPtr, 
		         	             UINT8 * RefPtr1, UINT8 * RefPtr2, 
						         INT16 * ChangePtr, UINT32 LineStep )
{
    UINT32  i;
	INT16 *TmpDataPtr = pbi->TmpDataBuffer;

    for ( i = 0; i < BLOCK_HEIGHT_WIDTH; i++ )
   	{	
		// Form each row
        TmpDataPtr[0] = (INT16)( (((INT32)RefPtr1[0] + (INT32)RefPtr2[0]) >> 1) + ChangePtr[0] );
   	    TmpDataPtr[1] = (INT16)( (((INT32)RefPtr1[1] + (INT32)RefPtr2[1]) >> 1) + ChangePtr[1] );
   	    TmpDataPtr[2] = (INT16)( (((INT32)RefPtr1[2] + (INT32)RefPtr2[2]) >> 1) + ChangePtr[2] );
   	    TmpDataPtr[3] = (INT16)( (((INT32)RefPtr1[3] + (INT32)RefPtr2[3]) >> 1) + ChangePtr[3] );
   	    TmpDataPtr[4] = (INT16)( (((INT32)RefPtr1[4] + (INT32)RefPtr2[4]) >> 1) + ChangePtr[4] );
   	    TmpDataPtr[5] = (INT16)( (((INT32)RefPtr1[5] + (INT32)RefPtr2[5]) >> 1) + ChangePtr[5] );
   	    TmpDataPtr[6] = (INT16)( (((INT32)RefPtr1[6] + (INT32)RefPtr2[6]) >> 1) + ChangePtr[6] );
   	    TmpDataPtr[7] = (INT16)( (((INT32)RefPtr1[7] + (INT32)RefPtr2[7]) >> 1) + ChangePtr[7] );

        // Next row of Block
		ChangePtr += BLOCK_HEIGHT_WIDTH;             // Local 8x8 data structure
        TmpDataPtr += BLOCK_HEIGHT_WIDTH;
        RefPtr1 += LineStep; 
        RefPtr2 += LineStep; 
    }

    // Saturate the output to unsigend 8 bit values
    SatUnsigned8( ReconPtr, pbi->TmpDataBuffer, LineStep, BLOCK_HEIGHT_WIDTH );

}

/****************************************************************************
 * 
 *  ROUTINE       :     SatUnsigned8
 *
 *  INPUTS        :     INT16 * DataBlock
 *
 *                      UINT32  DataLineStep
 *                      UINT32  ResultLineStep
 *  
 *
 *  OUTPUTS       :     UINT8 * ResultPtr
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Saturates the input data to 8 bits unsigned and store
 *                      in the output buffer
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void SatUnsigned8( UINT8 * ResultPtr, INT16 * DataBlock, UINT32 ResultLineStep, UINT32 DataLineStep )
{
    INT32 i;
    INT32* tmp=(INT32*) ResultPtr;   
	ResultLineStep>>=2;
     // Partly expanded loop
    for ( i = 0; i < BLOCK_HEIGHT_WIDTH; i++ )
    {
        tmp[0] = LIMIT(DataBlock[0])|(INT32)(LIMIT(DataBlock[1])<<8)|(INT32)(LIMIT(DataBlock[2])<<16)|(INT32)(LIMIT(DataBlock[3])<<24);
//        ResultPtr[1] = LIMIT(DataBlock[1]);
  //      ResultPtr[2] = LIMIT(DataBlock[2]);
    //    ResultPtr[3] = LIMIT(DataBlock[3]);
        tmp[1] = LIMIT(DataBlock[4])|(INT32)(LIMIT(DataBlock[5])<<8)|(INT32)(LIMIT(DataBlock[6])<<16)|(INT32)(LIMIT(DataBlock[7])<<24);
//        ResultPtr[4] = LIMIT(DataBlock[4]);
  //      ResultPtr[5] = LIMIT(DataBlock[5]);
    //    ResultPtr[6] = LIMIT(DataBlock[6]);
      //  ResultPtr[7] = LIMIT(DataBlock[7]);

        DataBlock += DataLineStep;
        tmp += ResultLineStep;
    }
}



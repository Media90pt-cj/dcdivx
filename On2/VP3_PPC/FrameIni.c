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
*   Module Title :     FrameIni.c
*
*   Description  :     Video CODEC playback module
*
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#include "pbdll.h"
#include "BlockMapping.h"
#include "stdlib.h"
#include "vfw_pb_interface.h"
/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
                
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/  

/****************************************************************************
*  Imports 
*****************************************************************************
*/  

/****************************************************************************
*  Module Static Variables
*****************************************************************************
*/  


/****************************************************************************
*  Forward References
*****************************************************************************
*/  
void InitializeFragCoordinates(PB_INSTANCE *pbi);
/****************************************************************************
*  Explicit Imports
*****************************************************************************
*/

/* Last Inter frame DC values */
//extern Q_LIST_ENTRY InvLastInterDC;
//extern Q_LIST_ENTRY InvLastIntraDC;


/****************************************************************************
 * 
 *  ROUTINE       :     CalcPixelIndexTable
 *
 *  INPUTS        :     Nonex.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises the pixel index table.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void CalcPixelIndexTable( PB_INSTANCE *pbi)
{
    UINT32 i;
    UINT32 * PixelIndexTablePtr;

    // Calculate the pixel index table for normal image buffers
    PixelIndexTablePtr = pbi->pixel_index_table;
    for ( i = 0; i < pbi->YPlaneFragments; i++ )
    {
        PixelIndexTablePtr[ i ] = ((i / pbi->HFragments) * pbi->Configuration.VFragPixels * pbi->Configuration.VideoFrameWidth);  
        PixelIndexTablePtr[ i ] += ((i % pbi->HFragments) * pbi->Configuration.HFragPixels);
    }
 
    PixelIndexTablePtr = &pbi->pixel_index_table[pbi->YPlaneFragments];


    for ( i = 0; i < ((pbi->HFragments >> 1) * pbi->VFragments); i++ )
    {
        PixelIndexTablePtr[ i ] =  ((i / (pbi->HFragments / 2) ) * 
                                   (pbi->Configuration.VFragPixels * (pbi->Configuration.VideoFrameWidth / 2)) );   
        PixelIndexTablePtr[ i ] += ((i % (pbi->HFragments / 2) ) * pbi->Configuration.HFragPixels) + pbi->YPlaneSize;
    }

    /****************************************************************************************************************/
    // Now calculate the pixel index table for image reconstruction buffers
    PixelIndexTablePtr = pbi->recon_pixel_index_table;
    for ( i = 0; i < pbi->YPlaneFragments; i++ )
    {
        PixelIndexTablePtr[ i ] = ((i / pbi->HFragments) * pbi->Configuration.VFragPixels * pbi->Configuration.YStride);  
        PixelIndexTablePtr[ i ] += ((i % pbi->HFragments) * pbi->Configuration.HFragPixels) + pbi->ReconYDataOffset;
    }
 
    
    // U blocks
    PixelIndexTablePtr = &pbi->recon_pixel_index_table[pbi->YPlaneFragments];

    for ( i = 0; i < pbi->UVPlaneFragments; i++ )
    {
        PixelIndexTablePtr[ i ] =  ((i / (pbi->HFragments / 2) ) * 
                                   (pbi->Configuration.VFragPixels * (pbi->Configuration.UVStride)) );   
        PixelIndexTablePtr[ i ] += ((i % (pbi->HFragments / 2) ) * pbi->Configuration.HFragPixels) + pbi->ReconUDataOffset;
    }

    // V blocks
    PixelIndexTablePtr = &pbi->recon_pixel_index_table[pbi->YPlaneFragments + pbi->UVPlaneFragments];

    for ( i = 0; i < pbi->UVPlaneFragments; i++ )
    {
        PixelIndexTablePtr[ i ] =  ((i / (pbi->HFragments / 2) ) * 
                                   (pbi->Configuration.VFragPixels * (pbi->Configuration.UVStride)) );   
        PixelIndexTablePtr[ i ] += ((i % (pbi->HFragments / 2) ) * pbi->Configuration.HFragPixels) + pbi->ReconVDataOffset;
    }
}
/****************************************************************************
 * 
 *  ROUTINE       :     DeleteFragmentInfo
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
void DeleteFragmentInfo(PB_INSTANCE * pbi)
{

	// free prior allocs if present
	if(	pbi->display_fragmentsAlloc)
		free(pbi->display_fragmentsAlloc);
	if(	pbi->pixel_index_tableAlloc )
		free(pbi->pixel_index_tableAlloc);
	if(	pbi->recon_pixel_index_tableAlloc )
		free(pbi->recon_pixel_index_tableAlloc);
//	if(	pbi->FragTokenCountsAlloc)
//		free(pbi->FragTokenCountsAlloc);
	if(	pbi->CodedBlockListAlloc)
		free(pbi->CodedBlockListAlloc);
	if(	pbi->FragMVectAlloc)
		free(pbi->FragMVectAlloc);
	if(pbi->FragCoeffsAlloc)
		free(pbi->FragCoeffsAlloc);
	if(pbi->FragCoefEOBAlloc)
		free(pbi->FragCoefEOBAlloc);
//	if(pbi->skipped_display_fragmentsAlloc)
//		free(pbi->skipped_display_fragmentsAlloc);
	if(pbi->QFragDataAlloc)
		free(pbi->QFragDataAlloc);
//	if(pbi->TokenListAlloc)
//		free(pbi->TokenListAlloc);
	if(pbi->FragCodingMethodAlloc)
		free(pbi->FragCodingMethodAlloc);
    if(pbi->FragCoordinates)
		free(pbi->FragCoordinates);


#if defined(POSTPROCESS)	
	if(	pbi->FragQIndexAlloc)
		free(pbi->FragQIndexAlloc);
	if( pbi->PPCoefBufferAlloc)
		free(pbi->PPCoefBufferAlloc);
	if( pbi->FragmentVariancesAlloc)
		free(pbi->FragmentVariancesAlloc);
#endif	
	if(pbi->BlockMap)
		free(pbi->BlockMap);

	if(pbi->SBCodedFlags)
		free(pbi->SBCodedFlags);
	if(pbi->SBFullyFlags)
		free(pbi->SBFullyFlags);
	if(pbi->MBFullyFlags)
		free(pbi->MBFullyFlags);
	if(pbi->MBCodedFlags)
		free(pbi->MBCodedFlags);

//    if(pbi->_Nodes)
 //       free(pbi->_Nodes);
//    pbi->_Nodes = 0;    


	pbi->QFragDataAlloc = 0;
//	pbi->TokenListAlloc = 0;
//	pbi->skipped_display_fragmentsAlloc = 0;
	pbi->FragCoeffsAlloc = 0;
	pbi->FragCoefEOBAlloc = 0;
	pbi->display_fragmentsAlloc = 0;
	pbi->pixel_index_tableAlloc = 0;
	pbi->recon_pixel_index_tableAlloc = 0;
//	pbi->FragTokenCountsAlloc = 0;
	pbi->CodedBlockListAlloc = 0;
	pbi->FragCodingMethodAlloc = 0;
	pbi->FragMVectAlloc = 0;
	pbi->MBCodedFlags = 0;
	pbi->MBFullyFlags = 0;
	pbi->BlockMap = 0;

	pbi->SBCodedFlags = 0;
	pbi->SBFullyFlags = 0;
	pbi->QFragData = 0;                  
	pbi->TokenList = 0;
//	pbi->skipped_display_fragments = 0;
	pbi->FragCoeffs = 0;
	pbi->FragCoefEOB = 0;
	pbi->display_fragments = 0;
	pbi->pixel_index_table = 0;
	pbi->recon_pixel_index_table = 0;
//	pbi->FragTokenCounts = 0;
	pbi->CodedBlockList = 0;
	pbi->FragCodingMethod = 0;
    pbi->FragCoordinates = 0;
	pbi->FragMVect = 0;

#if defined(POSTPROCESS)
	pbi->PPCoefBufferAlloc=0;
	pbi->PPCoefBuffer=0;
	pbi->FragQIndexAlloc = 0;
	pbi->FragQIndex = 0;
    pbi->FragmentVariancesAlloc= 0;
    pbi->FragmentVariances = 0 ;
#endif
}


/****************************************************************************
 * 
 *  ROUTINE       :     AllocateFragmentInfo
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
BOOL AllocateFragmentInfo(PB_INSTANCE * pbi)
{

	// clear any existing info
	DeleteFragmentInfo(pbi);

	// Perform Fragment Allocations
    pbi->display_fragmentsAlloc = (UINT8 *) malloc(32+pbi->UnitFragments * sizeof(UINT8));
//    if(!pbi->display_fragmentsAlloc) { DeleteFragmentInfo(pbi); return FALSE;}

    pbi->pixel_index_tableAlloc = (UINT32 *)malloc(32+pbi->UnitFragments * sizeof(UINT32));
//    if(!pbi->pixel_index_tableAlloc) { DeleteFragmentInfo(pbi); return FALSE;}

    pbi->recon_pixel_index_tableAlloc = (UINT32 *)malloc(32+pbi->UnitFragments * sizeof(UINT32));
//    if(!pbi->recon_pixel_index_tableAlloc) { DeleteFragmentInfo(pbi); return FALSE;}

//    pbi->FragTokenCountsAlloc = (UINT32 *)malloc(32+pbi->UnitFragments * sizeof(UINT32));
//    if(!pbi->FragTokenCountsAlloc) { DeleteFragmentInfo(pbi); return FALSE;}

    pbi->CodedBlockListAlloc = (INT32 *)malloc(32+pbi->UnitFragments * sizeof(INT32));
//    if(!pbi->CodedBlockListAlloc) { DeleteFragmentInfo(pbi); return FALSE;}
    
    pbi->FragMVectAlloc = (MOTION_VECTOR *)malloc(32+pbi->UnitFragments * sizeof(MOTION_VECTOR));
//    if(!pbi->FragMVectAlloc) { DeleteFragmentInfo(pbi); return FALSE;}
    
    pbi->FragCoeffsAlloc = (UINT8 *)malloc(32+pbi->UnitFragments * sizeof(UINT8 ));
//    if(!pbi->FragCoeffsAlloc) { DeleteFragmentInfo(pbi); return FALSE;}
    
    pbi->FragCoefEOBAlloc = (UINT8 *)malloc(32+pbi->UnitFragments * sizeof(UINT8 ));
//    if(!pbi->FragCoefEOBAlloc) { DeleteFragmentInfo(pbi); return FALSE;}
    
//    pbi->skipped_display_fragmentsAlloc = (UINT8 *)malloc(32+pbi->UnitFragments * sizeof(UINT8 ));
 //   if(!pbi->skipped_display_fragmentsAlloc) { DeleteFragmentInfo(pbi); return FALSE;}
    
    pbi->QFragDataAlloc = (INT16 (*)[64])malloc(32+pbi->UnitFragments * sizeof(Q_LIST_ENTRY) * 64);
//    if(!pbi->QFragDataAlloc) { DeleteFragmentInfo(pbi); return FALSE;}
    
//    pbi->TokenListAlloc = (UINT32 (*)[128])malloc(32+pbi->UnitFragments * sizeof(UINT32) * 128);
 //   if(!pbi->TokenListAlloc) { DeleteFragmentInfo(pbi); return FALSE;}
    
    pbi->FragCodingMethodAlloc = (CODING_MODE *)malloc(32+pbi->UnitFragments * sizeof(CODING_MODE));
//    if(!pbi->FragCodingMethodAlloc) { DeleteFragmentInfo(pbi); return FALSE;}

    pbi->FragCoordinates = (COORDINATE *)malloc(pbi->UnitFragments * sizeof(COORDINATE));
//    if(!pbi->FragCoordinates) { DeleteFragmentInfo(pbi); return FALSE;}



//	pbi->_Nodes                                = malloc(32+pbi->UnitFragments * sizeof(COEFFNODE) );
	
	// Super Block Initialization
	pbi->SBCodedFlags = (UINT8 *)malloc(32+pbi->SuperBlocks * sizeof(UINT8));
//	if(!pbi->SBCodedFlags) { DeleteFragmentInfo(pbi); return FALSE;}

	pbi->SBFullyFlags = (UINT8 *)malloc(32+pbi->SuperBlocks * sizeof(UINT8));
//	if(!pbi->SBFullyFlags) { DeleteFragmentInfo(pbi); return FALSE;}

	// Macro Block Initialization
	pbi->MBCodedFlags = (UINT8 *)malloc(32+pbi->MacroBlocks * sizeof(UINT8));
  //  if(!pbi->MBCodedFlags) { DeleteFragmentInfo(pbi); return FALSE;}

	pbi->MBFullyFlags = (UINT8 *)malloc(32+pbi->MacroBlocks * sizeof(UINT8));
//    if(!pbi->MBFullyFlags) { DeleteFragmentInfo(pbi); return FALSE;}

	pbi->BlockMap = (INT32 (*)[4][4])malloc(32+pbi->SuperBlocks * sizeof(INT32) * 4 * 4);
//    if(!pbi->BlockMap) { DeleteFragmentInfo(pbi); return FALSE;}

	// adjust to the next 32 byte border (for cache reasons)
	pbi->QFragData = (short (*) [64]) ROUNDUP32(pbi->QFragDataAlloc);
//	pbi->TokenList = (UINT32 (*) [128]) ROUNDUP32(pbi->TokenListAlloc );
//	pbi->skipped_display_fragments = (unsigned char *) ROUNDUP32(pbi->skipped_display_fragmentsAlloc );
	pbi->FragCoeffs = (unsigned char *) ROUNDUP32(pbi->FragCoeffsAlloc );
	pbi->FragCoefEOB = (unsigned char *) ROUNDUP32(pbi->FragCoefEOBAlloc );
	pbi->display_fragments = (unsigned char *) ROUNDUP32(pbi->display_fragmentsAlloc );
	pbi->pixel_index_table = (UINT32 *) ROUNDUP32(pbi->pixel_index_tableAlloc );
	pbi->recon_pixel_index_table = (UINT32 *) ROUNDUP32(pbi->recon_pixel_index_tableAlloc );
//	pbi->FragTokenCounts = (UINT32 *) ROUNDUP32(pbi->FragTokenCountsAlloc );
	pbi->CodedBlockList = (INT32 *) ROUNDUP32(pbi->CodedBlockListAlloc );
	pbi->FragCodingMethod = (CODING_MODE *) ROUNDUP32(pbi->FragCodingMethodAlloc );
	pbi->FragMVect = (MOTION_VECTOR *) ROUNDUP32(pbi->FragMVectAlloc );

    return TRUE;
}

/****************************************************************************
 * 
 *  ROUTINE       :     DeleteFrameInfo
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
void DeleteFrameInfo(PB_INSTANCE * pbi)
{
	if(pbi->ThisFrameReconAlloc )
		free(pbi->ThisFrameReconAlloc );
	if(pbi->GoldenFrameAlloc)
		free(pbi->GoldenFrameAlloc);
	if(pbi->LastFrameReconAlloc)
		free(pbi->LastFrameReconAlloc);
//	if(pbi->PostProcessBufferAlloc)
//		free(pbi->PostProcessBufferAlloc);


	pbi->ThisFrameReconAlloc = 0;
	pbi->GoldenFrameAlloc = 0;
	pbi->LastFrameReconAlloc = 0;
//	pbi->PostProcessBufferAlloc = 0;


	pbi->ThisFrameRecon = 0;
	pbi->GoldenFrame = 0;
	pbi->LastFrameRecon = 0;
//	pbi->PostProcessBufferAlloc = 0;

}


/****************************************************************************
 * 
 *  ROUTINE       :     AllocateFrameInfo
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
BOOL AllocateFrameInfo(PB_INSTANCE * pbi, unsigned int FrameSize)
{

	// clear any existing info
	DeleteFrameInfo(pbi);

	// allocate frames
	pbi->ThisFrameReconAlloc = (UINT8 *)malloc(32+FrameSize*sizeof(YUV_BUFFER_ENTRY));
//    if(!pbi->ThisFrameReconAlloc) { DeleteFrameInfo(pbi); return FALSE;}

	pbi->GoldenFrameAlloc = (UINT8 *)malloc(32+FrameSize*sizeof(YUV_BUFFER_ENTRY ));
//    if(!pbi->GoldenFrameAlloc) { DeleteFrameInfo(pbi); return FALSE;}

	pbi->LastFrameReconAlloc = (UINT8 *)malloc(32+FrameSize*sizeof(YUV_BUFFER_ENTRY));
//    if(!pbi->LastFrameReconAlloc) { DeleteFrameInfo(pbi); return FALSE;}

//	pbi->PostProcessBufferAlloc = (UINT8 *)malloc(32+FrameSize*sizeof(YUV_BUFFER_ENTRY));
 //   if(!pbi->PostProcessBufferAlloc) { DeleteFrameInfo(pbi); return FALSE;}


	// RGB24 is 2x the size of YUV411 

	// adjust up to the next 32 byte boundary
	pbi->ThisFrameRecon = (unsigned char *) ROUNDUP32(pbi->ThisFrameReconAlloc );
	pbi->GoldenFrame = (unsigned char *) ROUNDUP32(pbi->GoldenFrameAlloc );
	pbi->LastFrameRecon = (unsigned char *) ROUNDUP32(pbi->LastFrameReconAlloc );
//	pbi->PostProcessBuffer = (unsigned char *) ROUNDUP32( pbi->PostProcessBufferAlloc );


    return TRUE;
}

/****************************************************************************
 * 
 *  ROUTINE       :     InitFrameDetails
 *
 *  INPUTS        :     Nonex.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises the frame details.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
BOOL InitFrameDetails(PB_INSTANCE *pbi)
{
	int FrameSize;

	//if(pbi->CPUFree > 0 )
	//	SetPbParam( pbi, PBC_SET_CPUFREE, pbi->CPUFree );

    /* Set the frame size etc. */                                                        
    pbi->YPlaneSize = pbi->Configuration.VideoFrameWidth * pbi->Configuration.VideoFrameHeight; 
    pbi->UVPlaneSize = pbi->YPlaneSize / 4;  
    pbi->HFragments = pbi->Configuration.VideoFrameWidth / pbi->Configuration.HFragPixels;
    pbi->VFragments = pbi->Configuration.VideoFrameHeight / pbi->Configuration.VFragPixels;
    pbi->UnitFragments = ((pbi->VFragments * pbi->HFragments)*3)/2;
	pbi->YPlaneFragments = pbi->HFragments * pbi->VFragments;
	pbi->UVPlaneFragments = pbi->YPlaneFragments / 4;

    pbi->Configuration.YStride = (pbi->Configuration.VideoFrameWidth + STRIDE_EXTRA);
    pbi->Configuration.UVStride = pbi->Configuration.YStride / 2;
    pbi->ReconYPlaneSize = pbi->Configuration.YStride * (pbi->Configuration.VideoFrameHeight + STRIDE_EXTRA);
    pbi->ReconUVPlaneSize = pbi->ReconYPlaneSize / 4;
	FrameSize = pbi->ReconYPlaneSize + 2 * pbi->ReconUVPlaneSize;

    pbi->YDataOffset = 0;
    pbi->UDataOffset = pbi->YPlaneSize;
    pbi->VDataOffset = pbi->YPlaneSize + pbi->UVPlaneSize;
    pbi->ReconYDataOffset = (pbi->Configuration.YStride * UMV_BORDER) + UMV_BORDER;
    pbi->ReconUDataOffset = pbi->ReconYPlaneSize + (pbi->Configuration.UVStride * (UMV_BORDER/2)) + (UMV_BORDER/2);
    pbi->ReconVDataOffset = pbi->ReconYPlaneSize + pbi->ReconUVPlaneSize + (pbi->Configuration.UVStride * (UMV_BORDER/2)) + (UMV_BORDER/2);

	// Image dimensions in Super-Blocks
	pbi->YSBRows = (pbi->Configuration.VideoFrameHeight/32)  + ( pbi->Configuration.VideoFrameHeight%32 ? 1 : 0 );
	pbi->YSBCols = (pbi->Configuration.VideoFrameWidth/32)  + ( pbi->Configuration.VideoFrameWidth%32 ? 1 : 0 );
	pbi->UVSBRows = ((pbi->Configuration.VideoFrameHeight/2)/32)  + ( (pbi->Configuration.VideoFrameHeight/2)%32 ? 1 : 0 );
	pbi->UVSBCols = ((pbi->Configuration.VideoFrameWidth/2)/32)  + ( (pbi->Configuration.VideoFrameWidth/2)%32 ? 1 : 0 );
	
	// Super-Blocks per component
	pbi->YSuperBlocks = pbi->YSBRows * pbi->YSBCols;
	pbi->UVSuperBlocks = pbi->UVSBRows * pbi->UVSBCols;
	pbi->SuperBlocks = pbi->YSuperBlocks+2*pbi->UVSuperBlocks;

	// Useful  externals
	pbi->YMacroBlocks = ((pbi->VFragments+1)/2)*((pbi->HFragments+1)/2);	
	pbi->UVMacroBlocks = ((pbi->VFragments/2+1)/2)*((pbi->HFragments/2+1)/2);
	pbi->MacroBlocks = pbi->YMacroBlocks+2*pbi->UVMacroBlocks;

	if(!AllocateFragmentInfo(pbi))
        return FALSE;

	AllocateFrameInfo(pbi, FrameSize);

    InitializeFragCoordinates(pbi);

    // Configure mapping between quad-tree and fragments
	CreateBlockMapping ( pbi->BlockMap, pbi->YSuperBlocks, pbi->UVSuperBlocks, pbi->HFragments, pbi->VFragments);
    
    /* Re-initialise the pixel index table. */                                     
	CalcPixelIndexTable( pbi );
    return TRUE;

}

/****************************************************************************
 * 
 *  ROUTINE       :     InitialiseConfiguration
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Sets up the default starting pbi->Configuration.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void InitialiseConfiguration(PB_INSTANCE *pbi)
{  
    // Machine / platform specific initialisation
    DMachineSpecificConfig(pbi);

    // IDCT table initialisation
    //InitDctTables();

    pbi->Configuration.HFragPixels = 8;
    pbi->Configuration.VFragPixels = 8;
} 

/****************************************************************************
 * 
 *  ROUTINE       :     InitializeFragCoordinates
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Sets up the default starting pbi->Configuration.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void InitializeFragCoordinates(PB_INSTANCE *pbi)
{

    UINT32 i, j;

    UINT32 HorizFrags = pbi->HFragments;
    UINT32 VertFrags = pbi->VFragments;
    UINT32 StartFrag = 0;
    //Y
    
    for(i = 0; i< VertFrags; i++)
    {
        for(j = 0; j< HorizFrags; j++)
        {
            
            UINT32 ThisFrag = i * HorizFrags + j;
            pbi->FragCoordinates[ ThisFrag ].x=j * BLOCK_HEIGHT_WIDTH;
            pbi->FragCoordinates[ ThisFrag ].y=i * BLOCK_HEIGHT_WIDTH;            

        }
    }
    //U
    HorizFrags >>= 1;
    VertFrags >>= 1;
    StartFrag = pbi->YPlaneFragments;

    for(i = 0; i< VertFrags; i++)
    {
        for(j = 0; j< HorizFrags; j++)
        {
            UINT32 ThisFrag = StartFrag + i * HorizFrags + j;
            pbi->FragCoordinates[ ThisFrag ].x=j * BLOCK_HEIGHT_WIDTH;
            pbi->FragCoordinates[ ThisFrag ].y=i * BLOCK_HEIGHT_WIDTH;            

        }
    }
 
    //V
    StartFrag = pbi->YPlaneFragments + pbi->UVPlaneFragments;
    for(i = 0; i< VertFrags; i++)
    {
        for(j = 0; j< HorizFrags; j++)
        {
            UINT32 ThisFrag = StartFrag + i * HorizFrags + j;
            pbi->FragCoordinates[ ThisFrag ].x=j * BLOCK_HEIGHT_WIDTH;
            pbi->FragCoordinates[ ThisFrag ].y=i * BLOCK_HEIGHT_WIDTH;            

        }
    }

}
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
*   Module Title :     PBDLL
*
*   Description  :     Video CODEC DEMO playback dll header
*
*
*****************************************************************************
*/

#ifndef __INC_PBDLL_H
#define __INC_PBDLL_H

#define VAL_RANGE   256

#include "rawtypes.h"

#include "codec_common.h"
#include "huffman.h"
//#include "YUVtofromRGB.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

                                     
#define INPUT_BUFFER_SIZE       MAX_FRAME_SIZE    // VFW must be able to take a whole frame at once.
#define IB_HIGH_WATER           ((INPUT_BUFFER_SIZE * 3) / 4)
#define IB_LOW_WATER            (INPUT_BUFFER_SIZE / 4)
#define VAL_RANGE   256


/****************************************************************************
*  Types
*****************************************************************************
*/



/****************************************************************************
*  MACROS
*****************************************************************************
*/

/****************************************************************************
*  Global Variables
*****************************************************************************
*/
extern UINT8 LimitVal_VP31[VAL_RANGE * 3];

// Truth table to indicate if the given mode uses motion estimation
extern BOOL ModeUsesMC[MAX_MODES];

// Bit reader function copied and de c++ ified from tims code 
typedef struct _BITREADER
{
	int bitsinremainder;	// # of bits still used in remainder
	uint32 remainder;		// remaining bits from original long
	cuchar * position;		// character pointer position within data
	cuchar * const origin;	// starting point of original data 
} BITREADER;



typedef struct coeffNode
{
    int i;
    struct coeffNode *next;
} COEFFNODE;


typedef struct PB_INSTANCE * xPB_INST;
typedef struct PB_INSTANCE
{
	//****************************************************************************************
	// Bitstream input and Output Pointers
	//****************************************************************************************

	/* Current access points fopr input and output buffers */
	BITREADER br;

	//****************************************************************************************
	// Decoder and Frame Type Information
	UINT8 Vp3VersionNo;
//	BOOL  MmxEnabled;
//	BOOL  XmmEnabled;
//	BOOL  WmtEnabled;


//	int   DecoderErrorCode;
//	BOOL  FramesHaveBeenSkipped;
//	BOOL  SkipYUVtoRGB;		       /* Skip conversion */
//	BOOL  OutputRGB;			   /* Output To RGB */


	// Frame Info
	CODING_MODE	 CodingMode;
	UINT8  FrameType;       
//	UINT8  KeyFrameType;
	UINT32 QualitySetting;
	UINT32 FrameQIndex;            /* Quality specified as a table index */
	UINT32 ThisFrameQualityValue;  /* Quality value for this frame  */
	UINT32 LastFrameQualityValue;  /* Last Frame's Quality */
	INT32  CodedBlockIndex;		   /* Number of Coded Blocks */
	UINT32 CodedBlocksThisFrame;   /* Index into coded blocks */
	UINT32 FrameSize;              /* The number of bytes in the frame. */

	//****************************************************************************************

	
	//****************************************************************************************
	// Frame Size & Index Information
	CONFIG_TYPE Configuration;	// frame configuration

	UINT32  YPlaneSize;  
	UINT32  UVPlaneSize;  
	UINT32  VFragments;
	UINT32  HFragments;
	UINT32  UnitFragments;
	UINT32  YPlaneFragments;
	UINT32  UVPlaneFragments;
	
	UINT32  ReconYPlaneSize;
	UINT32  ReconUVPlaneSize;
	
	UINT32  YDataOffset;
	UINT32  UDataOffset;
	UINT32  VDataOffset;
	UINT32  ReconYDataOffset;
	UINT32  ReconUDataOffset;
	UINT32  ReconVDataOffset;
	UINT32  YSuperBlocks;	// Number of SuperBlocks in a Y frame
	UINT32  UVSuperBlocks;	// Number of SuperBlocks in a U or V frame
	UINT32  SuperBlocks;	// Total number of SuperBlocks in a Y,U,V frame
	
	UINT32  YSBRows;		// Number of rows of SuperBlocks in a Y frame
	UINT32  YSBCols;		// Number of cols of SuperBlocks in a Y frame
	UINT32  UVSBRows;		// Number of rows of SuperBlocks in a U or V frame
	UINT32  UVSBCols;		// Number of cols of SuperBlocks in a U or V frame
	
	UINT32  YMacroBlocks;	// Number of Macro-Blocks in Y component
	UINT32  UVMacroBlocks;	// Number of Macro-Blocks in U/V component
	UINT32  MacroBlocks;	// Total number of Macro-Blocks
	//****************************************************************************************

	//****************************************************************************************
	// Frames 
	YUV_BUFFER_ENTRY *ThisFrameRecon;
	YUV_BUFFER_ENTRY *GoldenFrame; 
	YUV_BUFFER_ENTRY *LastFrameRecon;
	//YUV_BUFFER_ENTRY *PostProcessBuffer;
	YUV_BUFFER_ENTRY *ScaleBuffer;     /* new buffer for testing new loop filtering scheme */

//	UINT8   * bmp_dptr0;

    INT32 * BoundingValuePtr;
	//****************************************************************************************

	//****************************************************************************************
	// Fragment Information
	UINT32  *pixel_index_table;			// start address of first pixel of fragment in source
	UINT32  *recon_pixel_index_table;   // start address of first pixel in recon buffer

	
	UINT8   *display_fragments;         // Fragment update map
//	UINT8	*skipped_display_fragments;	// whether fragment YUV Conversion and update is to be skipped
	INT32   *CodedBlockList;			// A list of fragment indices for coded blocks.
	MOTION_VECTOR  *FragMVect;			// fragment motion vectors

//	UINT32  *FragTokenCounts;			// Number of tokens per fragment
	UINT32  (*TokenList)[128];			// Fragment Token Pointers

	UINT8  *FragCoeffs;					// # of coeffs decoded so far for fragment
	UINT8  *FragCoefEOB;				// Position of last non 0 coef within QFragData
	Q_LIST_ENTRY (*QFragData)[64];		// Fragment Coefficients Array Pointers
	CODING_MODE  *FragCodingMethod;		// coding method for the fragment 

	//****************************************************************************************
	// pointers to addresses used for allocation and deallocation the others are rounded 
	// up to the nearest 32 bytes
	YUV_BUFFER_ENTRY *ThisFrameReconAlloc;
	YUV_BUFFER_ENTRY *GoldenFrameAlloc; 
	YUV_BUFFER_ENTRY *LastFrameReconAlloc;
//	YUV_BUFFER_ENTRY *ScaleBufferAlloc; 	
//	YUV_BUFFER_ENTRY *PostProcessBufferAlloc;

	UINT32  *pixel_index_tableAlloc;		// start address of first pixel of fragment in source
	UINT32  *recon_pixel_index_tableAlloc;  // start address of first pixel in recon buffer

	UINT8   *display_fragmentsAlloc;        // Fragment update map
//	UINT8	*skipped_display_fragmentsAlloc;// whether fragment YUV Conversion and update is to be skipped
	INT32   *CodedBlockListAlloc;			// A list of fragment indices for coded blocks.
	MOTION_VECTOR  *FragMVectAlloc;			// fragment motion vectors

	//UINT32  *FragTokenCountsAlloc;			// Number of tokens per fragment
	//UINT32  (*TokenListAlloc)[128];			// Fragment Token Pointers

	UINT8  *FragCoeffsAlloc;				// # of coeffs decoded so far for fragment

    //COEFFNODE *_Nodes;
    //UINT32 *transIndex;                     // ptr to table of transposed indexes

	UINT8  *FragCoefEOBAlloc;				// Position of last non 0 coef within QFragData

	Q_LIST_ENTRY (*QFragDataAlloc)[64];		// Fragment Coefficients Array Pointers
	CODING_MODE  *FragCodingMethodAlloc;	// coding method for the fragment 

	//****************************************************************************************

	//****************************************************************************************
//    INT32 bumpLast;

	// Macro Block and SuperBlock Information
	INT32	(*BlockMap)[4][4];			// super block + sub macro block + sub frag -> FragIndex

	// Coded flag arrays and counters for them
	UINT8   *SBCodedFlags;
	UINT8   *SBFullyFlags;
	UINT8   *MBCodedFlags;
	UINT8   *MBFullyFlags;
	//****************************************************************************************

	UINT32 EOB_Run;

    COORDINATE *FragCoordinates;
	MOTION_VECTOR MVector;
	INT32    ReconPtr2Offset;        // Offset for second reconstruction in half pixel MC
	Q_LIST_ENTRY * quantized_list;  
	INT16 *ReconDataBuffer;
	INT16 *ReconDataBufferAlloc;
//	INT32 IDCT_codes[64];      
//	Q_LIST_ENTRY InvLastIntraDC;
//	Q_LIST_ENTRY InvLastInterDC;
//	Q_LIST_ENTRY LastIntraDC;
//	Q_LIST_ENTRY LastInterDC;

	UINT32 BlocksToDecode;				// Blocks to be decoded this frame
	UINT32 DcHuffChoice;				// Huffman table selection variables
	UINT8  ACHuffChoice; 
//	UINT32 QuadMBListIndex;

//	INT32 ByteCount;

	UINT32  bit_pattern ;
	UINT8   bits_so_far ; 
	UINT8   NextBit;
	INT32   BitsLeft;
	
	INT16 * DequantBuffer;
	INT16 * DequantBufferAlloc;

//	INT32 fp_quant_InterUV_coeffs[64];
//	INT32 fp_quant_InterUV_round[64];
//	INT32 fp_ZeroBinSize_InterUV[64];

	INT16 * TmpReconBuffer;
	INT16 * TmpReconBufferAlloc;
	INT16 * TmpDataBuffer;
	INT16 * TmpDataBufferAlloc;

	// Loop filter bounding values
	INT32 FiltBoundingValue[512];

    // Dequantiser and rounding tables
	UINT32 QThreshTable[Q_TABLE_SIZE];
	Q_LIST_ENTRY * dequant_InterUV_coeffs;
	Q_LIST_ENTRY * dequant_InterUV_coeffsAlloc;
	unsigned quant_index[64];
	//INT32 quant_Y_coeffs[64];
	//INT32 quant_UV_coeffs[64];
	//INT32 fp_quant_Y_coeffs[64];		// used in reiniting quantizers

	HUFF_ENTRY ** HuffRoot_VP3x;
	UINT32 * HuffCodeArray_VP3x[NUM_HUFF_TABLES];
	UINT8  * HuffCodeLengthArray_VP3x[NUM_HUFF_TABLES];

    // Quantiser and rounding tables
 /*   INT32 fp_quant_UV_coeffs[64];
	INT32 fp_quant_Inter_coeffs[64];
	INT32 fp_quant_Y_round[64];
	INT32 fp_quant_UV_round[64];
	INT32 fp_quant_Inter_round[64];
	INT32 fp_ZeroBinSize_Y[64];
	INT32 fp_ZeroBinSize_UV[64];
	INT32 fp_ZeroBinSize_Inter[64];
	INT32 *fquant_coeffs;
	INT32 *fquant_round;
	INT32 *fquant_ZbSize;*/
	Q_LIST_ENTRY * dequant_Y_coeffs;
	Q_LIST_ENTRY * dequant_Y_coeffsAlloc;
	Q_LIST_ENTRY * dequant_UV_coeffs;
	Q_LIST_ENTRY * dequant_UV_coeffsAlloc;
	Q_LIST_ENTRY * dequant_Inter_coeffs;
	Q_LIST_ENTRY * dequant_Inter_coeffsAlloc;
	Q_LIST_ENTRY * dequant_coeffs;

	// Predictor used in choosing entropy table for decoding block patterns.
	UINT8	BlockPatternPredictor;

	//****************************************************************

	

	//****************************************************************
	// Function Pointers some probably could be library globals!
	void (*ReconIntra)( xPB_INST pbi, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep );

	void (*ReconInter)( xPB_INST pbi, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep );

	void (*ReconInterHalfPixel2)( xPB_INST pbi, UINT8  * ReconPtr, UINT8  * RefPtr1, UINT8 * RefPtr2, 
		INT16  * ChangePtr, UINT32 LineStep );

	void (*idct[65])( INT16 *InputData, INT16 *QuantMatrix, INT16 * OutputData );


//	void (*YUVtoRGB)( xPB_INST pbi, YUV_BUFFER_ENTRY_PTR yblock, YUV_BUFFER_ENTRY_PTR ublock,	
//		YUV_BUFFER_ENTRY_PTR vblock, int uvoffset, BGR_TYPE * RGBPtr, BOOL ReconBuffer );	/* Yuv Conversion Function */		

	void (*ClearDownQFrag)(xPB_INST pbi);

	UINT32 (*ExtractToken)(BITREADER *,HUFF_ENTRY *);

	void (*UnPackVideo)(xPB_INST pbi);
	void (*CopyBlock) (unsigned char *src, unsigned char *dest, unsigned int srcstride);

    void (*FilterHoriz)(xPB_INST pbi, UINT8 * PixelPtr, INT32 LineLength, INT32*);
    void (*FilterVert)(xPB_INST pbi, UINT8 * PixelPtr, INT32 LineLength, INT32*);
    INT32 *(*SetupBoundingValueArray)(xPB_INST pbi, INT32 FLimit);

    void (*BuildQuantIndex)(xPB_INST pbi);

//	void (*ClearSysState)(void);


	
//	UINT8 *  DataOutputInPtr;		  

	//****************************************************************

} PB_INSTANCE;

/****************************************************************************
*  Functions.
*****************************************************************************
*/  

extern void FilterHoriz_Generic(PB_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr);
extern void FilterVert_Generic(PB_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr);
extern INT32 *SetupBoundingValueArray_Generic(PB_INSTANCE *pbi, INT32 FLimit);

extern void FilterHoriz_MMX(PB_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr);
extern void FilterVert_MMX(PB_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr);
extern INT32 *SetupBoundingValueArray_ForMMX(PB_INSTANCE *pbi, INT32 FLimit);


extern PB_INSTANCE * CreatePBInstance(void);

extern void DeletePBInstance(PB_INSTANCE **);

extern void ReadAndUnPackDFArray( PB_INSTANCE *pbi );

extern int LoadAndDecode(PB_INSTANCE *pbi);


/* Frame header functions. */   
extern BOOL LoadFrame(PB_INSTANCE *pbi);
extern BOOL LoadFrameHeader(PB_INSTANCE *pbi);
extern void SetFrameType(PB_INSTANCE *pbi, UINT8 FrType );
extern UINT8 GetFrameType(PB_INSTANCE *pbi);

extern void DecodeData(PB_INSTANCE *pbi);  
extern void UnpackAndExpandAcToken( PB_INSTANCE *pbi, Q_LIST_ENTRY * ExpandedBlock, UINT8 * CoeffIndex  );
extern void UnpackAndExpandDcToken( PB_INSTANCE *pbi, Q_LIST_ENTRY * ExpandedBlock, UINT8 * CoeffIndex  );

extern BOOL PbBuildBitmapHeader( PB_INSTANCE *pbi, UINT32 ImageWidth, UINT32 ImageHeight );

extern __inline UINT8 ExtractBitFromBuffer(PB_INSTANCE *pbi);
extern __inline UINT32 ReadBits( PB_INSTANCE *pbi, UINT32 BitsToRead );
extern void  ExtractInit(PB_INSTANCE *pbi);

extern __inline void FrArrayDeCodeInit(PB_INSTANCE *pbi);
extern CODING_MODE FrArrayUnpackMode(PB_INSTANCE *pbi);
extern void ClearDownQFragData(PB_INSTANCE *pbi);

// Expand quantised viodeo data and reconstruct reference frames.
extern void ExpandToken( PB_INSTANCE *pbi, Q_LIST_ENTRY * ExpandedBlock, UINT8 * CoeffIndex, UINT32 Token, INT32 ExtraBits );
extern void ReconRefFrames (PB_INSTANCE *pbi);
extern void CopyBlock(unsigned char *src, unsigned char *dest, unsigned int srcstride);
extern void ExpandBlock ( PB_INSTANCE *pbi, INT32 FragmentNumber );
extern void ExpandKFBlock ( PB_INSTANCE *pbi, INT32 FragmentNumber );
extern UINT32 ExtractToken(BITREADER *br, HUFF_ENTRY * CurrentRoot);

// Set up Entropy Tables
extern void SelectHuffmanSet( PB_INSTANCE *pbi );
extern void CreateHuffmanTrees();
extern void DestroyHuffmanTrees();
extern void select_InterUV_quantiser ( PB_INSTANCE *pbi );
extern void DestroyHuffTree( HUFF_ENTRY * * root_ptr );
extern void InitPostProcessing(void);

// Indexing into block mapping
extern void CalcPixelIndexTable( PB_INSTANCE *pbi );
extern UINT32 GetFragIndex( UINT32 * , UINT32 FragmentNo );
extern UINT32 ReconGetFragIndex( UINT32 * , UINT32 FragmentNo );

// DCT Functions 
extern void IDctSlow(  Q_LIST_ENTRY * InputData, int16 *QuantMatrix, int16 * OutputData );

// Quantizer Selections 
extern void InitQTables( PB_INSTANCE *pbi );
extern void UpdateQ( PB_INSTANCE *pbi, UINT32 NewQ );
extern void select_Y_dequantiser ( PB_INSTANCE *pbi);
extern void select_UV_dequantiser ( PB_INSTANCE *pbi );
extern void select_Inter_dequantiser ( PB_INSTANCE *pbi );
extern void select_Y_quantiser ( PB_INSTANCE *pbi );
extern void select_Inter_quantiser ( PB_INSTANCE *pbi );
extern void select_UV_quantiser ( PB_INSTANCE *pbi );

extern void BuildQuantIndex_Generic(PB_INSTANCE *pbi);

extern BOOL InitFrameDetails(PB_INSTANCE *pbi); 

// RGB YUV Conversions
//extern void ScalarYUVtoRGB ( PB_INSTANCE * pbi,YUV_BUFFER_ENTRY_PTR yblock, YUV_BUFFER_ENTRY_PTR ublock,		
//				             YUV_BUFFER_ENTRY_PTR vblock, int uvoffset,					    
  //                           BGR_TYPE * RGBPtr, BOOL ReconBuffer );

//extern void CopyYUVtoBmp( PB_INSTANCE * pbi,YUV_BUFFER_ENTRY_PTR YUVPtr, UINT8 * BmpPtr, BOOL ConvertAll, BOOL ReconBuffer );
//extern void SetupRgbYuvAccelerators(void);
//extern void ErrorTrap( PB_INSTANCE *pbi, int ErrorCode );

// Initialization Routines
extern BOOL AllocateFragmentInfo(PB_INSTANCE * pbi);
extern BOOL AllocateFrameInfo(PB_INSTANCE * pbi, unsigned int FrameSize);
extern void DeleteFragmentInfo(PB_INSTANCE * pbi);
extern void DeleteFrameInfo(PB_INSTANCE * pbi);
//extern void fillidctconstants(void);
extern void DMachineSpecificConfig(PB_INSTANCE *pbi);

// Tim's bit reading functions
extern UINT32 bitread1(BITREADER *br) ;
extern UINT32 bitread(BITREADER *br, int bits);

//extern void ClearSysState(void);
#endif

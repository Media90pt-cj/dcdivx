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
#include "rawtypes.h"

#ifndef vp3d_h
#define vp3d_h 1

// Interface between vp3d.dll and Albany's DXV adaptor/blitter.


// The main object "defined" here.

struct VP3decompressor;


// Some conveniences.


// FourCC codes.  Should agree with microsoft's definition
// sans their stupid types and include files.

typedef uint32 FourCC;
#define MakeFourCC( a, b, c, d) ( \
	(uint32) (uchar) a \
	| (uint32) (uchar) b << 8 \
	| (uint32) (uchar) c << 16 \
	| (uint32) (uchar) d << 24 \
)

// The actual fourCC for now; similar remarks apply.

#define VP30 1


// Array of fourCC codes, has length _and_ is null-terminated.
// As Donald Knuth once said,
// "Some people occasionally like a little extra redundancy sometimes."

typedef struct { const FourCC * codes;  uint numCodes;}  FourCClist;


// YUV buffer configuration.

typedef struct {

	uint32 Ywidth, Yheight, UVwidth, UVheight;

	long Ystride, UVstride;

	const uchar *Ybuf, *Ubuf, *Vbuf;

} YUVbufferLayout;


#if __cplusplus
#	define Decompressor VP3decompressor
	extern "C" {
#else
#	define Decompressor struct VP3decompressor
#endif

#if defined(MACPPC)
#define _stdcall 
#endif

// Return array of fourCC codes supported.

const FourCClist *  VP3DfourCClist();


// Create a decompressor for a particular supported stream type.
// Returns 0 on failure.

Decompressor *  VP3DcreateDecompressor( FourCC streamType);

void  VP3DdestroyDecompressor( Decompressor *);


// Advance to next frame, returning reference to updated YUV buffer.

const YUVbufferLayout *  VP3DnextFrame
( 
	Decompressor *, const uchar * CXdata, uint32 CXdataLengthInBytes
);

void  VP3DblitBGR(
	const Decompressor *, uchar * outRGB, long outStride, long outHeight
);


#if __cplusplus
	}
#endif

#undef Decompressor

#endif	// vp3d_h

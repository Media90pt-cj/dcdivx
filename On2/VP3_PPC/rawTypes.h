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


#ifndef rawTypes_h
#define rawTypes_h 1

// Typedefs for known width integral types and convenience.
#include "kos.h"
#include <stddef.h>
#include <limits.h>
#include "machine.h"

// We define types int16 and int32.
// They are the smallest native types having at least 16 and 32 bits.

// Cannot count on existence of int64, so we put that in a special header
// (int64.h) to better track our dependence.

// No integral type has less than 8 bits, so no "int8" typedef is necessary.

typedef unsigned char uchar;
typedef signed char schar;

typedef const char cchar;
typedef const uchar cuchar;
typedef const schar cschar;

// Shorts are guaranteed to have at least 16 bits;
// it's possible (though unlikely) that chars have 16 bits.


typedef const short cshort;
typedef const ushort cushort;



// Longs are guaranteed to have at least 32 bits.
// Ints are somewhere between shorts and longs.


typedef const int cint;
typedef const uint cuint;
typedef const long clong;
typedef const long culong;


typedef const int32 cint32;
typedef const uint32 cuint32;

typedef const float cfloat;
typedef const double cdouble;

//typedef const bool cbool;

typedef const size_t csize_t;

#endif 	// rawTypes_h

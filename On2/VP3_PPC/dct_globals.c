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
*   Module Title :     DCT_GLOBALS.c
*
*   Description  :     DCT Global declarations
*
*****************************************************************************
*/						

#include "pbdll.h"

/*	Cos and Sin constant multipliers used during DCT and IDCT */
const double C1S7 = (double)0.9807852804032;
const double C2S6 = (double)0.9238795325113;
const double C3S5 = (double)0.8314696123025;
const double C4S4 = (double)0.7071067811865;
const double C5S3 = (double)0.5555702330196;
const double C6S2 = (double)0.3826834323651;
const double C7S1 = (double)0.1950903220161;

// Gives the initial bytes per block estimate for each Q value
double BpbTable[Q_TABLE_SIZE] =     {   0.42,  0.45,  0.46,  0.49,  0.51,  0.53,  0.56,  0.58,
                                        0.61,  0.64,  0.68,  0.71,  0.74,  0.77,  0.80,  0.84,
                                        0.89,  0.92,  0.98,  1.01,  1.04,  1.13,  1.17,  1.23,
                                        1.28,  1.34,  1.41,  1.45,  1.51,  1.59,  1.69,  1.80,
                                        1.84,  1.94,  2.02,  2.15,  2.23,  2.34,  2.44,  2.50,
                                        2.69,  2.80,  2.87,  3.04,  3.16,  3.29,  3.59,  3.66, 
                                        3.86,  3.94,  4.22,  4.50,  4.64,  4.70,  5.24,  5.34,
                                        5.61,  5.87,  6.11,  6.41,  6.71,  6.99,  7.36,  7.69
                                    };

double KfBpbTable[Q_TABLE_SIZE] =   {   0.74,  0.81,  0.88,  0.94,  1.00,  1.06,  1.14,  1.19,
                                        1.27,  1.34,  1.42,  1.49,  1.54,  1.59,  1.66,  1.73,
                                        1.80,  1.87,  1.97,  2.01,  2.08,  2.21,  2.25,  2.36,
                                        2.39,  2.50,  2.55,  2.65,  2.71,  2.82,  2.95,  3.01,
                                        3.11,  3.19,  3.31,  3.42,  3.58,  3.66,  3.78,  3.89,
                                        4.11,  4.26,  4.36,  4.39,  4.63,  4.76,  4.85,  5.04, 
                                        5.26,  5.29,  5.47,  5.64,  5.76,  6.05,  6.35,  6.67,
                                        6.91,  7.17,  7.40,  7.56,  8.02,  8.45,  8.86,  9.38
                                    };

 /*This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The GPL can be found at: http://www.gnu.org/copyleft/gpl.html						*
 *																						*
 * Authors:																					*
 *          Marc Dukette      
 *			Pedro Mateu
 **************************************************************************************/

		
#ifdef __cplusplus
extern "C" {
#endif 

#include "portab.h"

#ifndef _YUVRGB_H_
#define _YUVRGB_H_
void (*convert_yuv)(uint8_t *puc_y, int stride_y, 
uint8_t *puc_u, uint8_t *puc_v, int stride_uv, 
uint8_t *puc_out, int width_y, int height_y,int stride_dest, int Dither);

void yuv2rgb_565(uint8_t *puc_y, int stride_y, 
uint8_t *puc_u, uint8_t *puc_v, int stride_uv, 
uint8_t *puc_out, int width_y, int height_y,int stride_dest, int Dither);
#endif
#ifdef __cplusplus
}
#endif 

 /*This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The GPL can be found at: http://www.gnu.org/copyleft/gpl.html						*
 *																						*
 *																						*	
 * Authors:																				*
 *			Marc Dukette
 *			Jacob Alberty
 **************************************************************************************/


#ifndef __DIVX_H
#define __DIVX_H

#include <kos.h>

/* Floating-point Sin/Cos; 256 angles, -1.0 to 1.0 */
extern float sintab[];
#define msin(angle) sintab[angle]
#define mcos(angle) sintab[((angle)+64) % 256]

void play_s3m(char *fn);

/* bkg.c */
void bkg_setup();
void bkg_render();
void loading_setup();
void loading_render();

/* texture.c */
extern uint32 util_texture;
uint32 texture_alloc(uint32 size);
void setup_util_texture();

/* 3dutils.c */
void rotate(int zang, int xang, int yang, float *x, float *y, float *z);
void draw_poly_mouse(int ptrx, int ptry);
void draw_poly_char(float x1, float y1, float z1, float a, float r, float g, float b, int c);
void draw_poly_strf(float x1, float y1, float z1, float a, float r, float g, float b, char *fmt, ...);
void draw_poly_box(float x1, float y1, float x2, float y2, float z,
		float a1, float r1, float g1, float b1,
		float a2, float r2, float g2, float b2);

/* songmenu.c */
void song_menu_render();

#define MAKE_SYSCALL(rs, p1, p2, idx) \
        uint32 *syscall_bc = (uint32*)0x8c0000bc; \
        int (*syscall)() = (int (*)())(*syscall_bc); \
        rs syscall((p1), (p2), 0, (idx));
uint32  params[4];

extern char curdir[256];
extern int num_entries;
extern int real_num_entries;

#endif	/* __DIVX_H */

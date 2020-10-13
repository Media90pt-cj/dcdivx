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

#include "divx.h"
//#include <png/png.h>
#include <jpeg/jpeg.h>
#include <zlib/zlib.h>
#include <zlib.h>


/* This module will manage the spiffy background effects */

static uint32 lchktexture = 0;

/* Make a nice (now familiar =) XOR pattern texture */
void loading_setup() {
    file_t infile;                /* source file */

    if (!lchktexture)
	{
		lchktexture = ta_txr_allocate(256*256*2);
		jpeg_to_texture("/rd/loading.jpg", lchktexture, 256,1);
	}
    if ((infile = fs_open("/cd/loading.jpg", O_RDONLY)) == 0) {
		return;
    }
	fs_close(infile);
    jpeg_to_texture("/cd/loading.jpg", lchktexture, 256,1);
}

void loading_render() {
    poly_hdr_t poly;
    vertex_ot_t vert;

    ta_poly_hdr_txr(&poly, TA_OPAQUE, TA_RGB565_TWID, 256, 256, lchktexture, TA_NO_FILTER);
    ta_commit_poly_hdr(&poly);
    
    vert.r = vert.g = vert.b = 1.0f;
    vert.a = 1.0f;
    vert.oa = vert.or = vert.ob = 0.0f;
    vert.flags = TA_VERTEX_NORMAL;
    
    vert.x = 1;
    vert.y = 1;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 0.0;
    ta_commit_vertex(&vert, sizeof(vert));
    
    vert.x = 640;
    vert.y = 1;
    vert.z = 1;
    vert.u = 1.0;
    vert.v = 0.0;
    ta_commit_vertex(&vert, sizeof(vert));
    
    vert.x = 1;
    vert.y = 624;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 1.0;
    ta_commit_vertex(&vert, sizeof(vert));
    
    vert.x = 640;
    vert.y = 624;
    vert.z = 1;
    vert.u = 1.0;
    vert.v = 1.0;
    vert.flags = TA_VERTEX_EOL;
    ta_commit_vertex(&vert, sizeof(vert));
}


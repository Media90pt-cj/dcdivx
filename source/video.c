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
 *			Dan Potter
 **************************************************************************************/

#include <kos.h>

int stretchwidth=1024;
int stretchheight=1024;
int x=1;
int y=1;
uint32 buf1;
uint32 buf2;

uint32* bufptr1;
uint32* bufptr2;

int whichpoly = 1;
void txclear(int width, int height)
{

    int i,j;
    unsigned int *texp;
	unsigned int p1,p2;
    texp = (unsigned int*) bufptr1; //ta->txr_map(tex);

	for(i = 0; i < 512*512; i++) 
	{
		p1=(128 | (((unsigned short)16)<<8));
		p2=(128 | (((unsigned short)16)<<8));
		texp[i] = p1|(p2<<16); //(*(cb++) | (((unsigned short)*(y++))<<8))|((*(cr++) | (((unsigned short)*(y++))<<8))<<16);
	}		
    texp = (unsigned int*) bufptr2; //ta->txr_map(tex);

	for(i = 0; i < 512*512; i++) 
	{
		p1=(128 | (((unsigned short)16)<<8));
		p2=(128 | (((unsigned short)16)<<8));
		texp[i] = p1|(p2<<16); //(*(cb++) | (((unsigned short)*(y++))<<8))|((*(cr++) | (((unsigned short)*(y++))<<8))<<16);
	}		
//	memset(bufptr1,192,512*512*2);
//	memset(bufptr2,192,512*512*2);
	if (width>height)
	{
		stretchheight=stretchwidth=(int) (512.0000000*((double)640/(double)width));
		x=0;
		y=((480-(int) (((double)height)*((double)640/(double)width)))/2);
		//printf("stretchx=%d stretchy=%d y=%d\r\n",stretchwidth,stretchheight,y);
	}
	else
	{
		stretchheight=stretchwidth=(int) (512.0000000*((double)480/(double)height));
		y=0;
		x=((640-(int) (((double)width)*((double)480/(double)height)))/2);
	}
}

uint32 *txinit() {
	buf1 = ta_txr_allocate(512*512*2);
	buf2 = ta_txr_allocate(512*512*2);
   bufptr1 = (uint32 *)ta_txr_map(buf1);
   bufptr2 = (uint32 *)ta_txr_map(buf2);
	return (uint32 *)bufptr2;
}

uint32 *txset() {
	poly_hdr_t poly;
	vertex_ot_t vert;

	ta_begin_render();
	if (whichpoly==1) {
 		ta_poly_hdr_txr(&poly, TA_OPAQUE, TA_YUV422, 512, 512, buf1, TA_BILINEAR_FILTER); 
		whichpoly++;
	} else {
		ta_poly_hdr_txr(&poly, TA_OPAQUE, TA_YUV422, 512, 512, buf2, TA_BILINEAR_FILTER);
		whichpoly--;
	}
	ta_commit_poly_hdr(&poly);
	vert.r = vert.g = vert.b = 1.0f;
	vert.a = 1.0f;
	vert.oa = vert.or = vert.ob = 0.0f;
	vert.flags = TA_VERTEX_NORMAL;

    vert.x = x+1;
    vert.y = y+1;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 0.0;
    ta_commit_vertex(&vert, sizeof(vert));
   vert.x = x+stretchwidth;
   vert.y = y+1;
   vert.z = 1;
   vert.u = 1.0;
   vert.v = 0.0;
   ta_commit_vertex(&vert, sizeof(vert));

   vert.x = x+1;
   vert.y = y+stretchheight;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 1;
    ta_commit_vertex(&vert, sizeof(vert));

    vert.x =x+stretchwidth;
    vert.y = y+stretchheight;
    vert.z = 1;
    vert.u = 1;
    vert.v = 1;
    vert.flags = TA_VERTEX_EOL;
	ta_commit_vertex(&vert, sizeof(vert));
	ta_commit_eol();

	ta_poly_hdr_col(&poly, TA_TRANSLUCENT);
	ta_commit_poly_hdr(&poly);
	ta_commit_eol();

	ta_finish_frame();
//   thd_create(ta_finish_frame, NULL);

	if (whichpoly == 1) {
		return (uint32 *)bufptr1;
	} else {
		return (uint32 *)bufptr2;
	}

}

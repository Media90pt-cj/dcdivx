 /*This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The GPL can be found at: http://www.gnu.org/copyleft/gpl.html						*
 *																						*
 *
 *	Authors:
 *			Marc Dukette
 *			Jacob Alberty
 **************************************************************************************/

#include <stdio.h>
#include "divx.h"

int vblank=0;
int menu=0;
static int get_drv_stat(void *param) { MAKE_SYSCALL(return, param, 0, 4); }
static void drv_reset() { MAKE_SYSCALL(/**/, 0, 0, 9); }
static void drv_main() { MAKE_SYSCALL(/**/, 0, 0, 2); }


/********************************************************************************/
//no you're not dreaming these functions are GARBAGE, but they serve the very 
//improtant role of padding out the binary to a near even 1KB, which gets us up
//to 5% performance boost.  Add as many copies as necessary to just break the next
//1KB boundry for the binary.  I need to find a better way of doing this though.
/********************************************************************************/
int check_start2() {
	static uint8 mcont = 0;
	cont_cond_t cond;

	if (!mcont) {
		mcont = maple_first_controller();
		if (!mcont) {
			printf("No controllers attached\n");
			return 1;
		}
	}

	if (cont_get_cond(mcont, &cond)) {
		printf("Error getting controller status\n");
		return 1;
	}
		
	if (!(cond.buttons & CONT_START)) {
		printf("Pressed start\n");
		return 1;
	}

	return 0;
}

int check_start3() {
	static uint8 mcont = 0;
	cont_cond_t cond;

	if (!mcont) {
		mcont = maple_first_controller();
		if (!mcont) {
			printf("No controllers attached\n");
			return 1;
		}
	}

	if (cont_get_cond(mcont, &cond)) {
		printf("Error getting controller status\n");
		return 1;
	}
		
	if (!(cond.buttons & CONT_START)) {
		printf("Pressed start\n");
		return 1;
	}

	return 0;
}
int check_start4() {
	static uint8 mcont = 0;
	cont_cond_t cond;

	if (!mcont) {
		mcont = maple_first_controller();
		if (!mcont) {
			printf("No controllers attached\n");
			return 1;
		}
	}


	return 0;
}

/********************************************************************************/
//end of padding
/********************************************************************************/


int check_start() {
	static uint8 mcont = 0;
	cont_cond_t cond;

	if (!mcont) {
		mcont = maple_first_controller();
		if (!mcont) {
			printf("No controllers attached\n");
			return 1;
		}
	}

	/* Check for start on the controller */
	if (cont_get_cond(mcont, &cond)) {
		printf("Error getting controller status\n");
		return 1;
	}
		
	if (!(cond.buttons & CONT_START)) {
		printf("Pressed start\n");
		return 1;
	}

	return 0;
}



void blit_font_texture() {
	poly_hdr_t poly;
	vertex_ot_t vert;
	
	ta_poly_hdr_txr(&poly, TA_TRANSLUCENT, TA_ARGB4444, 256, 256, util_texture,
		TA_NO_FILTER);
	ta_commit_poly_hdr(&poly);

	vert.flags = TA_VERTEX_NORMAL;
	vert.x = 50.0f;
	vert.y = 50.0f + 256.0f;
	vert.z = 256.0f;
	vert.u = 0.0f;
	vert.v = 1.0f;
	vert.dummy1 = vert.dummy2 = 0;
	vert.a = 1.0f; vert.r = 1.0f; vert.g = 1.0f; vert.b = 1.0f;
	vert.oa = vert.or = vert.og = vert.ob = 0.0f;
	ta_commit_vertex(&vert, sizeof(vert));

	vert.x = 50.0f;
	vert.y = 50.0f;
	vert.u = 0.0f;
	vert.v = 0.0f;
	ta_commit_vertex(&vert, sizeof(vert));

	vert.x = 50.0f + 256.0f;
	vert.y = 50.0f + 256.0f;
	vert.u = 1.0f;
	vert.v = 1.0f;
	ta_commit_vertex(&vert, sizeof(vert));

	vert.flags = TA_VERTEX_EOL;
	vert.x = 50.0f + 256.0f;
	vert.y = 50.0f;
	vert.u = 1.0f;
	vert.v = 0.0f;
	ta_commit_vertex(&vert, sizeof(vert));
}

extern uint8 romdisk[];

/* Program entry */
int main(int argc, char **argv) {
	int status=1;
	int i=0;
	/* Do basic setup */
	kos_init_all(IRQ_ENABLE | THD_ENABLE | TA_ENABLE, romdisk);
	vid_init(DM_640x480,  PM_RGB565);
//	vid_init(DM_640x480_PAL_IL,  PM_RGB565);
	/* Setup the mouse/font texture */
	setup_util_texture();
	bkg_setup();
	loading_setup();
	/* Setup background display */

	/* Find a mouse if there is one */
	//mmouse = maple_first_mouse();
	
	/* Find an LCD if there is one */
	//mvmu = maple_first_lcd();
	//stream_load_driver();
	while (1) {
		ta_begin_render();

		/* Opaque list *************************************/
		bkg_render();

		/* End of opaque list */
		ta_commit_eol();

		/* Translucent list ********************************/

		/* Top Banner */
//		draw_poly_box(0.0f, 10.0f, 640.0f, 20.0f+24.0f+10.0f, 90.0f, 
//			0.3f, 0.2f, 0.5f, 0.0f, 0.5f, 0.1f, 0.8f, 0.2f);
//		draw_poly_strf(5.0f, 20.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
//			"DC Divx Player");
		switch (menu) {
			case 0: {
				song_menu_render();
			}
			case 1: {
				
			} break;
		}
		/* Render the mouse if they move it.. it doesn't do anything
		   but it's cool looking ^_^ */
//		mouse_render();

		/* End of translucent list */
		ta_commit_eol();

		/* Finish the frame *******************************/
		ta_finish_frame();
		if ((vblank % 4) == 0) {
                       get_drv_stat(params);
                       drv_main();
               }

                /* check for disk swap */
                if (status == 6) {
                if (params[0] == 1) { menu = 0; status = 1;drv_reset;}
                }
                if (status == 1) {
                if (params[0] == 6) {
                        status = 6;
						menu = 1;
						//fs_iso9660_init();
                        strcpy(curdir, "/cd");
                        num_entries = 0;
						iso_ioctl(1, NULL, 1);
						//real_num_entries=0;
                        }
                }
                vblank++;
//		printf("params[0]=%i, status=%i, menu=%i\r\n", params[0], status, menu);
		while (i < 900000) {
		i++;
		}
//		timer_spin_sleep(100);
		i=0;

		/* Update the VMU LCD */
//		vmu_lcd_update();
	}
	
	/* Stop the sound */
	spu_disable();

	return 0;
}






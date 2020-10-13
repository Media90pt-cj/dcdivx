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

#include <string.h>
#include "divx.h"

/* Takes care of the song menu */
static int jlast=0;

int play_divx(char* );
int shuffle=0;
/* Song menu choices */
typedef struct {
	char	fn[256];
	int	size;
} entry;
char curdir[256] = "/cd";

static entry entries[200];
int num_entries = 0, load_queued = 0;
static int selected = 0, top = 0;

static int framecnt = 0;
static float throb = 0.2f, dthrob = 0.01f;
/* optimal random number function. */
unsigned long seed=0;
#define RNDC 1013904223
#define RNDM 1164525

int rand() {
	seed = seed * RNDM + RNDC;
	return seed;
}

int randnum(int limit) { return (rand() & 0x7fffffff) % limit; }

void Shuffle()
{
	entry temp[100];
	int PlayCount[100];

	int NextFile;
	int n=0;
	for (n=0;n<=num_entries;n++)
	{
		PlayCount[n]=0;
	}
	n=0;
//	srand(GetTickCount());
	if (seed==0) seed=jiffies;
	while(n<num_entries&&(entries[n].size<=0))
	{
		strcpy(temp[n].fn,entries[n].fn);
		temp[n].size=entries[n].size;
		PlayCount[n]=1;
		n++;
	}
	while(n<num_entries)
	{
		do{
		NextFile=randnum(num_entries); //((double)rand()/(double)RAND_MAX)*(((double)num_entries));
		printf("Radnom=%d Num=%d count=%d\r\n",NextFile,num_entries,n);
		}while(PlayCount[NextFile]);
		strcpy(temp[n].fn,entries[NextFile].fn);
		temp[n].size=entries[NextFile].size;
		PlayCount[NextFile]=1;
		n++;
	}
	for (n=0;n<num_entries;n++)
	{
		strcpy(entries[n].fn,temp[n].fn);
		entries[NextFile].size=temp[n].size;

	}

}

static void load_song_list() {
	file_t d;
	char *ext;

	d = fs_open(curdir, O_RDONLY | O_DIR);
	if (!d) {
		strcpy(curdir, "/");
		d = fs_open(curdir, O_RDONLY | O_DIR);
		if (!d) {
			num_entries = 1;
			strcpy(entries[0].fn,"Error!");
			entries[0].size = 0;
			return;
		}
	}
	{
		dirent_t *de;
		num_entries = 0;
		if (strcmp(curdir, "/cd")) {
			strcpy(entries[0].fn, "<..>"); entries[0].size = -1;
			num_entries++;
		}
		while ( (de = fs_readdir(d)) && num_entries < 200)
		{
			printf("read entry '%s' sise=%d\n", de->name,de->size);
			if (de->size <= 0) 
			{
				strcpy(entries[num_entries].fn, de->name);
				entries[num_entries].size = de->size;
				num_entries++;
			} 
			else 
			{
				ext = (char *)(strchr(de->name, '.')+1);
				if(!stricmp(ext,"avi")||(!stricmp(ext,"mp3"))||(!stricmp(ext,"ogg"))) 
				{
					strcpy(entries[num_entries].fn, de->name);
					entries[num_entries].size = de->size;
					num_entries++;
				}
			}
		}
	}
	fs_close(d);
	if (shuffle) Shuffle();
}

/* Draws the song listing */
static void draw_listing() {
	float y = 110.0f;
	int i, esel;
	char temp[250];
	/* Draw all the song titles */	
	for (i=0; i<10 && (top+i)<num_entries; i++) {
		strcpy(temp,entries[top+i].fn);
		if (strlen(temp)>25) temp[25]=0;
		draw_poly_strf(60.0f, y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			temp);
		if (entries[top+i].size >= 0) {
			draw_poly_strf(60.0f+360.0f, y, 30.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				"%d", entries[top+i].size/1000);
			draw_poly_strf(60.0f+450.0f, y, 20.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				"KB");
		} else {
			draw_poly_strf(60.0f+430.0f, y, 50.0f, 1.0f, 1.0f, 1.0f, 1.0f, "<DIR>");
		}
		y += 24.0f;
	}
	
	/* Put a highlight bar under one of them */
	esel = (selected - top);
	draw_poly_box(60.0f, 110.0f+esel*24.0f - 1.0f,
		570.0f, 110.0f+esel*24.0f + 25.0f, 95.0f,
		throb, 0.2f, 0.2f, throb, throb, 0.2f, 0.2f, throb);
}

/* Handle controller input */
void check_controller() {
	char filen[256];
	static uint8 mcont = 0;
	int bStart;
	static int up_moved = 0, down_moved = 0;
	int j;
	if (jlast==0) jlast=jiffies;
	cont_cond_t cond;
	j=jiffies-jlast;
	if (j<3000)
	{
		
		if (!mcont) {

			mcont = maple_first_controller();
			if (!mcont) { return; }
		}
		if (cont_get_cond(mcont, &cond)) { return; }
	}
	else
	{
		cond.buttons=0xFFFF;
	}
	if (!(cond.buttons & CONT_DPAD_UP)) {
		if ((framecnt - up_moved) > 10) {
			if (selected > 0) {
				selected--;
				if (selected < top) {
					top = selected;
				}
			}
			up_moved = framecnt;
		}
		jlast=jiffies;
	}
	if (!(cond.buttons & CONT_DPAD_DOWN)) {
		if ((framecnt - down_moved) > 10) {
			if (selected < (num_entries - 1)) {
				selected++;
				if (selected >= (top+10)) {
					top++;
				}
			}
			down_moved = framecnt;
		}
		jlast=jiffies;
	}
	if (cond.rtrig > 0)
	{
		shuffle=1;
		printf("Trigger\r\n");
		load_song_list();
		load_queued = 0;
		jlast=jiffies;
	}
	if (cond.ltrig > 0)
	{
		printf("Trigger\r\n");
		shuffle=0;
		load_song_list();
		load_queued = 0;
		jlast=jiffies;
	}
	bStart=(!(cond.buttons & CONT_START)||(j>3000));
	if ((!(cond.buttons & CONT_A))||(bStart))
	{
		poly_hdr_t poly;
		int i=0;
		if (!strcmp(entries[selected].fn, "Error!")) 
		{
			num_entries = 0;
			load_song_list();
		}
		else if (entries[selected].size >= 0) 
		{
			int bNext=1;
			ta_poly_hdr_col(&poly, TA_TRANSLUCENT);
			ta_commit_poly_hdr(&poly);
			ta_commit_eol();
			/* Finish the frame *******************************/
			ta_finish_frame();
			while (bNext)
			{
				int ret;
				int i=0;
				strcpy(filen,curdir);
				strcat(filen,"/");
				strcat(filen,entries[selected].fn);
				if (strstr(filen,".avi")||(strstr(filen,".AVI")))
				{
					for (i=0;i<3;i++)
					{
						ta_begin_render();
						loading_render();

						ta_commit_eol();
						ta_poly_hdr_col(&poly, TA_TRANSLUCENT);
						ta_commit_poly_hdr(&poly);
						ta_commit_eol();
						ta_finish_frame();
					}
				}
				ret=play_divx(filen);
				if (ret==2||(bStart&&(ret==0)))
				{
					selected++;
					printf("Next=%d\r\n",selected);
					while(entries[selected].size<=0||(selected>num_entries))
					{
						if (selected>num_entries) selected=0;
						if (entries[selected].size<=0) selected++;
					}
					for (i=0;i<3;i++)
					{
						ta_begin_render();
						bkg_render();

						ta_commit_eol();

						draw_listing();
						ta_commit_eol();

						ta_finish_frame();
					}
					bNext=1;
				}
				else if (ret==3)
				{
					selected--;
					if (selected<0) selected=num_entries-1;
					printf("Next=%d\r\n",selected);
					while(entries[selected].size<=0||(selected<0))
					{
						if (selected<0) selected=num_entries-1;
						if (entries[selected].size<=0) selected--;
					}
					for (i=0;i<3;i++)
					{
						ta_begin_render();
						bkg_render();

						ta_commit_eol();

						draw_listing();
						ta_commit_eol();

						ta_finish_frame();
					}
					bNext=1;
				}
				else
				{
					bNext=0;
				}
			
			}
			ta_begin_render();

			/* Opaque list *************************************/
			bkg_render();

			/* End of opaque list */
			ta_commit_eol();

		}
		else if (!(cond.buttons & CONT_A))
		{
			if (!strcmp(entries[selected].fn, "<..>")) {
				int i;
				for (i=strlen(curdir); i>0; i--) {
					if (curdir[i] == '/') {
						curdir[i] = 0;
						break;
					} else
						curdir[i] = 0;
				}
			} else {
				if (strcmp(curdir, "/"))
					strcat(curdir, "/");
				strcat(curdir, entries[selected].fn);
			}
			selected = top = num_entries = 0;
			printf("current directory is now '%s'\n", curdir);
			printf("Directory\r\n");
			load_song_list();

		}
		i=1;
		while (i) 
		{
			int n=0;
			i=0;
			if (cont_get_cond(mcont, &cond)) i=0;
			else if (!(cond.buttons & CONT_A)) i=1;
			while (n<900000)
			{
				n++;
			}
		}
		jlast=jiffies;

	}
/*	if (!(cond.buttons & CONT_START)||(j>3000)) 
	{
		int n;
		int bStop=0;
 		poly_hdr_t poly;
		//mouse_render();

		ta_commit_eol();

		ta_finish_frame();
		for (n=selected;n<num_entries&&(!bStop);n++)
		{
			if (entries[n].size >= 0) 
			{
				int i;
				strcpy(filen,curdir);
				strcat(filen,"/");
				strcat(filen,entries[n].fn);
				if (strstr(filen,".avi")||(strstr(filen,".AVI")))
				{
					for (i=0;i<3;i++)
					{
						ta_begin_render();
						bkg_render();

						ta_commit_eol();

						song_menu_render();
						ta_commit_eol();

						ta_finish_frame();
					}
				}
				bStop=(play_divx(filen)==-2);
			}
		}
		selected=0;
//		ta_begin_render();

//		bkg_render();

//		ta_commit_eol();
		jlast=jiffies;
	}*/
}

/* Check maple bus inputs */
void check_inputs() {
	 check_controller();
}

/* Main rendering of the song menu */
void song_menu_render() {
	static int last=0;
	/* Draw a background box */
//	draw_poly_box(30.0f, 70.0f, 610.0f, 430.0f, 90.0f, 
//		0.0f, 0.8f, 0.2f, 0.8f, 0.2f, 0.2f, 0.8f, 5.0f);
	/* If we don't have a file listing, get it now */
	if (num_entries == 0) {
		last=0;
		if (load_queued < 4) {
			draw_poly_strf(60.0f, 110.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				"Scanning Directory...");
			load_queued++;
			jlast=jiffies;
			return;
		} else {
			printf("No Entries\r\n");
			load_song_list();
			load_queued = 0;
		}
	}
	else
	{
		if (last==0)
		{
			bkg_setup();
			loading_setup();
		}
		last=1;
	}
	/* Draw the song listing */
	draw_listing();
	
	/* Adjust the throbber */
	throb += dthrob;
	if (throb < 0.2f || throb > 0.8f) {
		dthrob = -dthrob;
		throb += dthrob;
	}
	
	/* Check maple inputs */
	check_inputs();
	
	framecnt++;
}





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

int bPause;

CVSID("$Id: sndstream.c,v 1.3 2002/01/06 00:40:33 bardtx Exp $");

extern int basesynchpos,tbasesynchpos;
extern int nextsynchpos,tnextsynchpos;

/*

Here we implement a very simple double-buffering, not the rather complex
circular queueing done in most DMA'd sound architectures. This is done for
simplicity's sake. At some point we may upgrade that the circular queue
system. 

Basically this poll routine checks to see which buffer is playing, and
whether a new one is available or not. If not, we ask the user routine
for more sound data and load it up. That's about it.

*/

#include "sndstream.h"

/* What we think is the currently playing buffer */
static int curbuffer = 0;

/* Var to check for channel position */
static vuint32 *cmd = (vuint32*)(0xa0810000);
static volatile aica_channel *chans = (volatile aica_channel*)(0xa0810004);

/* Seperation buffers (for stereo) */
static int16 *sep_buffer[2] = {NULL, NULL};

/* "Get data" callback */
static void* (*str_get_data)(int cnt,int curbuffer) = NULL;

/* SPU RAM mutex (to avoid queue overflow) */
static spinlock_t mutex;

/* SPU RAM malloc pointer */
static uint32 ram_base, ram_top;

/* Stereo/mono flag for stream */
static int stereo;

/* In-ram copy of the stream.drv file in case the CD gets swapped */
static char *streamdrv = NULL;
static int streamdrv_size;
static int lastpos=0;
extern int jaudio;
/* Set "get data" callback */
void stream_set_callback(void *(*func)(int)) {
	str_get_data = func;
}

/* "Kicks" a channel command */
static void chn_kick(int chn) {
	*cmd = AICA_CMD_KICK | (chn+1);
	spu_write_wait();
}

/* Performs stereo seperation for the two channels; this routine
   has been optimized for the SH-4. */
static void sep_data(void *buffer) {
	register int16	*bufsrc, *bufdst;
	register int	x, y, cnt;

	if (stereo) {
		bufsrc = (int16*)buffer;
		bufdst = sep_buffer[0];
		x = 0; y = 0; cnt = 16384;
		do {
			*bufdst = *bufsrc;
			bufdst++; bufsrc+=2; cnt--;
		} while (cnt > 0);

		bufsrc = (int16*)buffer; bufsrc++;
		bufdst = sep_buffer[1];
		x = 1; y = 0; cnt = 16384;
		do {
			*bufdst = *bufsrc;
			bufdst++; bufsrc+=2; cnt--;
			x+=2; y++;
		} while (cnt > 0);
	} else {
		memcpy(sep_buffer[0], buffer, 32768);
		memcpy(sep_buffer[1], buffer, 32768);
	}
}

/* Load sample data from SH-4 ram into SPU ram (auto-allocate RAM) */
uint32 stream_load_sample(const uint16 *src, uint32 len) {
	uint32 where;
	
//	spinlock_lock(&mutex);
	where = ram_top;
	spu_memload(where, (uint8*)src, len);
	ram_top = (ram_top + len + 3) & (~3);
//	spinlock_unlock(&mutex);
	
	return where;
}

/* Dump all loaded sample data */
void stream_dump_samples() {
//	spinlock_lock(&mutex);
	ram_top = ram_base;
//	spinlock_unlock(&mutex);
}

/* Prefill buffers -- do this before calling start() */
void stream_prefill() {
	void *buf;

	if (!str_get_data) return;

	/* Load first buffer */
	if (stereo)
		buf = str_get_data(65536,0);
	else
		buf = str_get_data(32768,0);
	sep_data(buf);
	//spinlock_lock(&mutex);
	//printf("got lock");
	spu_memload(0x11000 + 32768*0, (uint8*)sep_buffer[0], 32768);
	spu_memload(0x21000 + 32768*0, (uint8*)sep_buffer[1], 32768);
	printf("memload  data");
	//spinlock_unlock(&mutex);

	/* Load second buffer */
	if (stereo)
		buf = str_get_data(65536,1);
	else
		buf = str_get_data(32768,1);
	sep_data(buf);
	//spinlock_lock(&mutex);
	spu_memload(0x11000 + 32768*1, (uint8*)sep_buffer[0], 32768);
	spu_memload(0x21000 + 32768*1, (uint8*)sep_buffer[1], 32768);
	//spinlock_unlock(&mutex);
	basesynchpos=tbasesynchpos;
	nextsynchpos=tnextsynchpos;

	/* Start with playing on buffer 0 */
	curbuffer = 0;
}
int OnBuffer=0;
/* Initialize stream system */
int stream_init(void* (*callback)(int)) {
	uint32	hnd, size;
	printf("in init\r\n");
	/* Load the AICA driver */
	if (!streamdrv) {
		printf("try load driver\r\n");
		hnd = fs_open("/rd/stream.drv", O_RDONLY);
		if (!hnd) {
			printf("Can't open sound driver\r\n");
			return -1;
		}
		printf("driver loaded\r\n");
		streamdrv_size = fs_total(hnd);
		streamdrv = malloc(fs_total(hnd));
		fs_read(hnd, streamdrv, streamdrv_size);
		fs_close(hnd);
	}

	/* Create stereo seperation buffers */
	if (!sep_buffer[0]) {
		sep_buffer[0] = memalign(32, 32768);
		sep_buffer[1] = memalign(32, 32768);
	}

	/* Finish loading the stream driver */
	spu_disable();
	spu_memset(0, 0, 0x31000);
	spu_memload(0, streamdrv, streamdrv_size);

	spu_enable();
	bPause=0;
	//thd_sleep(10);
	//printf("thd_sleep\r\n");

	/* Setup a mem load mutex */
	//spinlock_init(&mutex);
	ram_base = ram_top = 0x31000;

	/* Setup the callback */
	stream_set_callback(callback);
	printf("set callback\r\n");
	OnBuffer=0;
	lastpos=0;
//	chans[0].pos=0;
	
	return 0;
}
void stream_pause()
{
	if (!bPause)
	{
		chans[0].cmd = AICA_CMD_STOP;
		chn_kick(0);
	}
	else
	{
		chans[0].cmd = AICA_CMD_START;
		chn_kick(0);
	}
	bPause=!bPause;
}
/* Shut everything down and free mem */
void stream_shutdown() {
	OnBuffer=0;
	lastpos=0;
	spu_shutdown();
	if (sep_buffer[0]) {
		free(sep_buffer[0]);	sep_buffer[0] = NULL;
		free(sep_buffer[1]);	sep_buffer[1] = NULL;
	}
	if (streamdrv) {
		free(streamdrv);
		streamdrv = NULL;
	}
	spu_init();
}

/* Start streaming */
void stream_start(uint32 freq, int st) {
	if (!str_get_data) return;
	OnBuffer=0;
	lastpos=0;
	/* Select "observation" channel */
	/* *chsel = 0; */

	stereo = st;

	/* Prefill buffers */
	stream_prefill();

	/* Start streaming */
	chans[0].cmd = AICA_CMD_START;
	chans[0].freq = freq;
	chn_kick(0);
}

/* Stop streaming */
void stream_stop() {
	if (!str_get_data) return;

	/* Stop stream */
	chans[0].cmd = AICA_CMD_STOP;
	chn_kick(0);
}

float GetPos()
{
	int pos=chans[0].pos;
	if (lastpos>pos)
	{
		basesynchpos=tbasesynchpos;
		nextsynchpos=tnextsynchpos;

		OnBuffer++;
	}
	lastpos=pos;
//	spu_write_wait();
//	OnBuffer=0;
//	return ((OnBuffer*32768)+pos)*2*(stereo+1);
	return (float)pos/(float)(32768); //*2*(stereo+1));

}

/* Poll streamer to load more data if neccessary */
int stream_poll() {
	int	realbuffer;
	uint32	val;
	void	*data;
	int t;

	if (!str_get_data) return -1;
	if (bPause) return 0;
	/* Get "real" buffer */
	val = chans[0].pos;
	//spu_write_wait();
	realbuffer = !(val < 0x4000);

	/* Has the channel moved on from the "current" buffer? */
	if (curbuffer != realbuffer) {
		/* Yep, adjust "current" buffer and initiate a load */

		/*printf("Playing in buffer %d, loading %d\r\n",
			realbuffer, curbuffer);*/
		if (stereo)
			data = str_get_data(65536,curbuffer);
		else
			data = str_get_data(32768,curbuffer);
		if (data == NULL) {
			/* Fill the "other" buffer with zeros */
			spu_memset(0x11000 + 32768*curbuffer, 0, 32768);
			spu_memset(0x21000 + 32768*curbuffer, 0, 32768);
			/* Wait for the current buffer to complete */
//			do {
//				val = chans[0].pos;
//				spu_write_wait();
//				realbuffer = !(val < 0x4000);
//				if (realbuffer != curbuffer) thd_pass();
//			} while (curbuffer != realbuffer);
			return -1;
		}
		sep_data(data);
//		spinlock_lock(&mutex);
		t=jiffies;
		spu_memload(0x11000 + 32768*curbuffer, (uint8*)sep_buffer[0], 32768);
		//printf("load=%d\r\n",32768*curbuffer);

		spu_memload(0x21000 + 32768*curbuffer, (uint8*)sep_buffer[1], 32768);
	jaudio+=jiffies-t;
//		spinlock_unlock(&mutex);
		curbuffer = realbuffer;
	}
	return 0;
}

/* Start a sound sample on the given channel */
void stream_play_effect(int chn, uint32 src, uint32 freq, uint32 len, uint32 vol, uint32 pan) {
//	spinlock_lock(&mutex);
	chans[chn].cmd = AICA_CMD_START;
	chans[chn].pos = src;
	chans[chn].length = len;
	chans[chn].freq = freq;
	chans[chn].vol = vol;
	chans[chn].pan = pan;
	chn_kick(chn);
//	spinlock_unlock(&mutex);
}

/* Stop a sound sample on the given channel */
void stream_stop_effect(int chn) {
//	spinlock_lock(&mutex);
	chans[chn].cmd = AICA_CMD_STOP;
	chn_kick(chn);
//	spinlock_unlock(&mutex);
}

/* Set the volume on the streaming channels */
void stream_volume(int vol) {
//	spinlock_lock(&mutex);
	chans[0].cmd = AICA_CMD_VOL;
	chans[0].vol = vol;
	chn_kick(0);
//	spinlock_unlock(&mutex);
}



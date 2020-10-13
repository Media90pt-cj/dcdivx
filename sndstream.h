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

#ifndef __SNDSTREAM_H
#define __SNDSTREAM_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>
#include "arm/aica_cmd_iface.h"

/* Load sample data from SH-4 ram into SPU ram (auto-allocate RAM) */
uint32 stream_load_sample(const uint16 *src, uint32 len);

/* Dump all loaded sample data */
void stream_dump_samples();

/* Set "get data" callback */
void stream_set_callback(void *(*func)(int));

/* Prefill buffers -- do this before calling start() */
void stream_prefill();

/* Initialize stream system */
int stream_init(void* (*callback)(int));

/* Shut everything down and free mem */
void stream_shutdown();

/* Start streaming */
void stream_start(uint32 freq, int stereo);

/* Stop streaming */
void stream_stop();

/* Poll streamer to load more data if neccessary; zero if ok, -1 if the
   poll function returns NULL. */
int stream_poll();

/* Start a sound sample on the given channel */
void stream_play_effect(int chn, uint32 src, uint32 freq, uint32 len, uint32 vol, uint32 pan);

/* Stop a sound sample on the given channel */
void stream_stop_effect(int chn);

/* Set the volume on the streaming channels */
void stream_volume(int vol);

__END_DECLS

#endif	/* __SNDSTREAM_H */


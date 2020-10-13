// OGGLib.cpp : Defines the entry point for the DLL application.
//
#include "kos.h"

#include "lib/vorbis_codec.h"

	ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
	ogg_stream_state os; /* take physical pages, weld into a logical
						  stream of packets */
	ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet       op; /* one raw packet of data for decode */

	vorbis_info      vi; /* struct that stores all the static vorbis bitstream
		  settings */
	vorbis_comment   vc; /* struct that stores all the bitstream user comments */
	vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
	vorbis_block     vb; /* local working space for packet->PCM decode */

	char *buffer;
	int  bytes;

	int eos=0;
	int i;
	int decode_header=0;
	int streaminit=0;
	FIXP **pcm;
	int samples;
	int in_loop=0;
	int currentpos;
ogg_int16_t* convbuffer; //[4096]; /* take 8k out of the data segment, not the stack */
int convsize=4096;
int Seeking=0;
#define NEED_MORE 32768
// This is an example of an exported function.
void SeekStart_OGG()
{
}
void SeekEnd_OGG()
{
}
void Init_OGG(int Equalizer,char* eq)
{
	ogg_sync_init(&oy); /* Now we can read pages */
	streaminit=0;
	in_loop=0;
	decode_header=0;
	currentpos=0;
	i=0;
}
// This is an example of an exported function.
void Exit_OGG(void)
{
	ogg_stream_clear(&os);

	/* ogg_page and ogg_packet structs always point to storage in
	   libvorbis.  They're never freed or manipulated directly */

	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);  /* must be called last */

	ogg_sync_clear(&oy);

}

//static char buffer[40000];
int GetHeaderInfo_OGG(unsigned char * inbuff, int insize, int* Freq, int* ch, int* BitRate)
{
	*Freq=44100;
	*BitRate=128000;
	*ch=2;
//	Exit();
//	Init(0,0);
	return 0;
}

int DecompressAudio_OGG(unsigned char * inbuff, int bytes, char *outmemory, int outmemsize, int *done, int* inputpos)
{
	*done=0;
	if (inbuff)
	{
		buffer=ogg_sync_buffer(&oy,bytes);
		memcpy(buffer,inbuff,bytes);
		ogg_sync_wrote(&oy,bytes);
	}
	convbuffer=(ogg_int16_t*) outmemory;
	if (!decode_header)
	{
		if (!streaminit)
		{


			/* submit a 4k block to libvorbis' Ogg layer */
			/* Get the first page. */
			if(ogg_sync_pageout(&oy,&og)!=1)
			{
				printf("pageout error\r\n");
				return -1;
			}

			/* Get the serial number and set up the rest of decode. */
			/* serialno first; use it to set up a logical stream */
			ogg_stream_init(&os,ogg_page_serialno(&og));

			/* extract the initial header from the first page and verify that the
			   Ogg bitstream is in fact Vorbis data */

			/* I handle the initial header first instead of just having the code
			   read all three Vorbis headers at once because reading the initial
			   header is an easy way to identify a Vorbis bitstream and it's
			   useful to see that functionality seperated out. */

			vorbis_info_init(&vi);
			vorbis_comment_init(&vc);
			if(ogg_stream_pagein(&os,&og)<0)
			{ 
				/* error; stream version mismatch perhaps */
				printf("pagein error\r\n");
				return -1;
			}

			if(ogg_stream_packetout(&os,&op)!=1)
			{ 
				/* no page? must not be vorbis */
				printf("packeteout error\r\n");
				return -1;
			}

			if(vorbis_synthesis_headerin(&vi,&vc,&op)<0)
			{ 
				/* error case; not a vorbis header */
				printf("headerin error\r\n");
				return -1;
			}
			streaminit=1;
		}
			/* At this point, we're sure we're Vorbis.  We've set up the logical
		(Ogg) bitstream decoder.  Get the comment and codebook headers and
		set up the Vorbis decoder */

		/* The next two packets in order are the comment and codebook headers.
		They're likely large and may span multiple pages.  Thus we reead
		and submit data until we get our two pacakets, watching that no
		pages are missing.  If a page is missing, error out; losing a
		header page is the only place where missing data is fatal. */

		while(i<2)
		{
			while(i<2)
			{
				int result=ogg_sync_pageout(&oy,&og);
				if(result==0)return NEED_MORE; /* Need more data */
				/* Don't complain about missing or corrupt data yet.  We'll
				catch it at the packet output phase */
				if(result==1)
				{
					ogg_stream_pagein(&os,&og); /* we can ignore any errors here
					as they'll also become apparent
					at packetout */
					while(i<2)
					{
						result=ogg_stream_packetout(&os,&op);
						if(result==0) return NEED_MORE;
						if(result<0)
						{
							/* Uh oh; data at some point was corrupted or missing!
							We can't tolerate that in a header.  Die. */
							return -1;
						}
						vorbis_synthesis_headerin(&vi,&vc,&op);
						i++;
					}
				}
			}
			/* no harm in not checking before adding more */
		}


		convsize=4096/vi.channels;

		/* OK, got and parsed all three headers. Initialize the Vorbis
		   packet->PCM decoder. */
		vorbis_synthesis_init(&vd,&vi); /* central decode state */
		vorbis_block_init(&vd,&vb);     /* local state for most of the decode
						   so multiple block decodes can
						   proceed in parallel.  We could init
						   multiple vorbis_block structures
						   for vd here */
		decode_header=1;
	}
	/* The rest is just a straight decode loop until end of stream */
	eos=0;
	while(!eos)
	{
		while(!eos)
		{
			if (inbuff)
			{
				int result=ogg_sync_pageout(&oy,&og);
				if(result==0) return NEED_MORE; /* need more data */
				if(result<0) /* missing or corrupt data at this page position */
				{
					return NEED_MORE;
				}
				ogg_stream_pagein(&os,&og); 
			}							 
			while(1)
			{
				int result=1;
				result=ogg_stream_packetout(&os,&op);
				if(result==0) return NEED_MORE; /* need more data */
				if(result<0)
				{ /* missing or corrupt data at this page position */
				  /* no reason to complain; already complained above */
				}
				else
				{
					/* we have a packet.  Decode it */

					if(vorbis_synthesis(&vb,&op)==0) /* test for success! */
						vorbis_synthesis_blockin(&vd,&vb);
					/* 

					**pcm is a multichannel float vector.  In stereo, for
					example, pcm[0] is left, and pcm[1] is right.  samples is
					the size of each channel.  Convert the float values
					(-1.<=range<=1.) to whatever PCM format and write it out */
					while((samples=vorbis_synthesis_pcmout(&vd,&pcm))>0)
					{
						int j;
						int clipflag=0;
						int bout=(samples<convsize?samples:convsize);
		
						/* convert floats to 16 bit signed ints (host order) and
						   interleave */
						for(i=0;i<vi.channels;i++)
						{
							ogg_int16_t *ptr=convbuffer+i;
							FIXP  *mono=pcm[i];
							for(j=0;j<bout;j++)
							{
								int val = mono[j] >> (FIXP_FRACBITS - 15);

								/* might as well guard against clipping */
								if(val>32767)
								{
									val=32767;
									clipflag=1;
								}
								if(val<-32768)
								{
									val=-32768;
									clipflag=1;
								}
								*ptr=val;
								ptr+=vi.channels;
							}
						}
		
						//memcpy(outmemory,convbuffer,2*vi.channels*bout);
						*inputpos=currentpos+op.bytes;
						currentpos+=op.bytes;
						*done=2*vi.channels*bout;
						vorbis_synthesis_read(&vd,bout); 
						return 0;
					}	    
				}
			}
			if(ogg_page_eos(&og))eos=1;
		}
	}

	/* clean up this logical bitstream; before exit we see if we're
	   followed by another [chained] */
	return 0;
}


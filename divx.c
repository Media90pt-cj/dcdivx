 /*This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The GPL can be found at: http://www.gnu.org/copyleft/gpl.html						*
 *																						*
 * This project is uses technology from the MAD MP3 decoder also released under GPL,but	*																			  *
 * Optimized for use on DreamCast.														*
 *																						*	
 * Authors:																				*
 *          Rob Leslie(MP3 Decoder)          
 *			Marc Dukette(DreamCast optimizations and Player)
 *			Jacob Alberty
 **************************************************************************************/


#include "MADDecoder/global1.h"
#include "kos.h"
#include "MADDecoder/fixed.h"
#include "decore.h"
#include "avidecaps.h"
#include "yuv2rgb.h"
#include <malloc.h>
#include "math.h"
#include "MADDecoder/frame.h"
#include "MADDecoder/synth.h"

#include "Divx3Decoder/msmpeg.h"
#include "Divx3Decoder/avcodec.h"
#include "sndstream.h"
  struct mad_stream stream;
  struct mad_frame frame;
   struct mad_synth synth;
static int currentpos;
static int framepos;
static int begin=1;
static int seeking=0;
unsigned long  clipped=0;
int bytes_left=0;
unsigned int *txinit();
uint32 *txset();
void txclear(int,int);
int	g_nWidth;
int	g_nHeight;
int status;
int bEnd;
int basesynchpos,tbasesynchpos;
int nextsynchpos,tnextsynchpos;
void stream_pause();
extern int bPause;
mad_fixed_t left_err=0, right_err=0;
reader RD;
#define MP3_BUFFER_SIZE 4096 //32768
char* mp3_buffer=NULL; 
int Freq,ch,br,played,pos;
int jtot;
int jdec;
int jconv;
int jdraw;
int jmp3;
int jaudio;
int jmath;
int FastForward=false;
int pos;
int totMsTime=0;
int baseTick=0;
// msmpeg.cpp : Defines the entry point for the DLL application.
//
int BUFF_SIZE=0;

AVCodec *codec;
AVCodecContext codec_context, *c = &codec_context;
int got_picture, len;
AVPicture picture;

static void* get_data(int req_size,int curbuffer) ;


extern unsigned char *decore_stream;
extern int decore_length;

void SeekStart_OGG();
void SeekEnd_OGG();
void Init_OGG(int Equalizer,char* eq);
void Exit_OGG(void);
int GetHeaderInfo_OGG(unsigned char * inbuff, int insize, int* Freq, int* ch, int* BitRate);
int DecompressAudio_OGG(unsigned char * inbuff, int bytes, char *outmemory, int outmemsize, int *done, int* inputpos);

int decore_vp3(unsigned long handle, unsigned long dec_opt,
	void *param1, void *param2);

int decore_frame_vp3(unsigned char *stream, int length, unsigned char *bmp, int render_flag);

int decore_Div3(unsigned long handle, unsigned long dec_opt,
	void *param1, void *param2)
{
	switch (dec_opt)
	{
		case DEC_OPT_INIT:
		{
			DEC_PARAM *dec_param = (DEC_PARAM *) param1;
 			int x_size = dec_param->x_dim;
 			int y_size = dec_param->y_dim;
			unsigned long color_depth = dec_param->color_depth;
			int output_format = dec_param->output_format;

			divxinit(x_size,y_size);

			return DEC_OK;
		}
		break; 
		case DEC_OPT_RELEASE:
		{
			divxdeinit();
			return DEC_OK;
		}
		break;
		default:
		{

			return DEC_OK;
		}
		break;
	}
}

/***/

/***/

int decore_frame_Div3(unsigned char *stream, int length, unsigned char *bmp, int render_flag, int Extra)
{
    avcodec_decode_video(c, &picture, &got_picture, 
                               stream, 100000,bmp,render_flag,Extra);
	return 1;
}



void divxinit(int x, int y)
{

	register_all();
    /* find the mpeg1 video decoder */
    codec = avcodec_find_decoder(CODEC_ID_MSMPEG4);
    if (!codec) {
    }

    /* put default values */
    memset(c, 0, sizeof(*c));
	c->width=x;
	c->height=y;
    /* open it */
    if (avcodec_open(c, codec) < 0) {
    }
    
}


void divxdeinit()
{
    avcodec_close(c);
}






int (*decode) (unsigned long handle, unsigned long dec_opt,void *param1, void *param2);
int (*decode_frame)(unsigned char *stream, int length, unsigned char *bmp, int render_flag, int Extra);


cont_cond_t cond;
AVPicture avpict;
videoinfo* vidinfo=NULL;
int tot;
char vid_temp[100000];
uint64 totAudio;
int 	nCurrentFrame=1;
int nShouldbe=0;
char output[100];
char* outputbuffer=0;
int Drop=0;



static inline
signed long linear_dither(unsigned int bits, mad_fixed_t sample,
			  mad_fixed_t *error, unsigned long *clipped,
			  mad_fixed_t *clipping)
{
  mad_fixed_t quantized, check;

  /* dither */
  sample += *error;

  /* clip */
  quantized = sample;
  check = (sample >> MAD_F_FRACBITS) + 1;
  if (check & ~1) {
    if (sample >= MAD_F_ONE) {
      quantized = MAD_F_ONE - 1;
      ++*clipped;
      if (sample - quantized > *clipping &&
	  mad_f_abs(*error) < (MAD_F_ONE >> (MAD_F_FRACBITS + 1 - bits)))
	*clipping = sample - quantized;
    }
    else if (sample < -MAD_F_ONE) {
      quantized = -MAD_F_ONE;
      ++*clipped;
      if (quantized - sample > *clipping &&
	  mad_f_abs(*error) < (MAD_F_ONE >> (MAD_F_FRACBITS + 1 - bits)))
	*clipping = quantized - sample;
    }
  }

  /* quantize */
  quantized &= ~((1L << (MAD_F_FRACBITS + 1 - bits)) - 1);

  /* error */
  *error = sample - quantized;

  /* scale */
  return quantized >> (MAD_F_FRACBITS + 1 - bits);
}

static
unsigned int pack_pcm(unsigned char *data, unsigned int nsamples,
		      mad_fixed_t const *left, mad_fixed_t const *right,
		      int resolution, unsigned long *clipped,
			  mad_fixed_t *clipping)
{
  unsigned char const *start;
  register signed long sample0, sample1;
  start     = data;

  if (right) {  /* stereo */
    while (nsamples--) {
      sample0 = linear_dither(16, *left++, &left_err, clipped,clipping);
      sample1 = linear_dither(16, *right++, &right_err, clipped,clipping);

	data[0] = sample0 >>  0;
	data[1] = sample0 >>  8;
	data[2] = sample1 >>  0;
	data[3] = sample1 >>  8;

      data += 4; //bytes << 1;
    }
  }
  else {  /* mono */
    while (nsamples--)
	{
      sample0 = linear_dither(16, *left++, &left_err, clipped,clipping);

		data[1] = sample0 >>  8;
		data[0] = sample0 >>  0;
		data += 2;
    }
  }

  return data - start;
}


// This is an example of an exported function.
void Init_MP3(int Equalizer,char* eq)
{
	if (begin)
	{
	  mad_stream_init(&stream);
	  mad_frame_init(&frame);
	 mad_synth_init(&synth);
	 currentpos=0;
	 framepos=0;
		begin=1;
		seeking=1;
	}
	left_err=0;
	right_err=0;
	clipped=0;

}
// This is an example of an exported function.
void Exit_MP3(void)
{
  mad_frame_finish(&frame);

  mad_stream_finish(&stream);
  mad_synth_finish(&synth);
	begin=1;
}

int GetHeaderInfo_MP3(unsigned char * inbuff, int insize, int* Freq, int* ch, int* BitRate)
{
   	  mad_stream_buffer(&stream, (const unsigned char *)inbuff, insize);
	  while (mad_frame_decode(&frame, &stream) == -1)
	  {
		if (stream.error == MAD_ERROR_BUFLEN)
		{

			framepos=(int)(stream.this_frame-stream.buffer);
			return stream.bufend - stream.this_frame+1;
		}
	  }
  	*Freq=frame.header.samplerate;
	  *BitRate=frame.header.bitrate;
	  *ch=(frame.header.mode > 0) ? 2 : 1;
		Exit_MP3();
		Init_MP3(0,0);
	  return 0;
}

int DecompressAudio_MP3(unsigned char * inbuff, int insize, char *outmemory, int outmemsize, int *done, int* inputpos)
{

	int retval=0;
    mad_fixed_t const *ch1, *ch2;
	int resolution=16;

	if (inbuff)
	{
		mad_stream_buffer(&stream, (const unsigned char *)inbuff, insize);
		currentpos+=framepos;
		begin=0;
	}
	else if (begin)
	{
		*done=0;
		return 1;
	}
	int nch;
	mad_fixed_t clipping=0;
	int err=mad_frame_decode(&frame, &stream);
	if (err==-1)
	{
		if (stream.error == MAD_ERROR_BUFLEN)
		{

			framepos=(int)(stream.this_frame-stream.buffer);
			*inputpos=currentpos+framepos;
			*done=0;
					//printf("Stream error leftover = %d\r\n",stream.bufend - stream.this_frame+1);
			return stream.bufend - stream.this_frame+1;
//				retval=stream.bufend - stream.this_frame+1;			
		}
		else //if((stream.error ==MAD_ERROR_LOSTSYNC||(stream.error==MAD_ERROR_BADDATAPTR)))
		{
			stream.sync=0;
			framepos=(int)(stream.this_frame-stream.buffer);
			*inputpos=currentpos+framepos;
			*done=0;
			return 0;
		}
	}
	seeking=0;
	framepos=(int)(stream.this_frame-stream.buffer);
	*inputpos=currentpos+framepos;
	mad_synth_frame(&synth,&frame);
	nch= synth.pcm.channels;
	ch1 = synth.pcm.samples[0];
	ch2 = synth.pcm.samples[1];
	if (nch == 1)
		ch2 = 0;
	*done=pack_pcm(((unsigned char *)outmemory),
		 synth.pcm.length, ch1, ch2, resolution, &clipped,&clipping);
	return retval;    
}

int (*DecompressAudio)(unsigned char * inbuff, int insize, char *outmemory, int outmemsize, int *done, int* inputpos);
void (*Init) (int Equalizer,char* eq);
void (*Exit)(void);
int (*GetHeaderInfo)(unsigned char * inbuff, int insize, int* Freq, int* ch, int* BitRate);

long decBegin(int First)
{
	DEC_PARAM dec_param;
	dec_param.x_dim = g_nWidth;
	dec_param.y_dim = g_nHeight;
	dec_param.color_depth = 0;
	dec_param.dither=0;
	dec_param.output_format=RGB565;

	if (First==0)
		decode((long) NULL, DEC_OPT_INIT, &dec_param, NULL);

	//decore_setoutput((long) NULL, dec_param.output_format);
	convert_yuv = yuv2rgb_565;
	
	return 0;
}

int written=0;
int writepos=0;
int readpos=0;
char pbBuffer[1510720];
int synchpos[1200];
char abuff[68000];
int DoneDecode=0;
int Finished=0;


void StopFastForward()
{
	FastForward=!FastForward;
	tot=vidinfo->video_frames-vidinfo->video_pos;
	//nCurrentFrame=0;
	if (totAudio)
	{
		AVPicture avpict;
		int t=vidinfo->audio_bytes;
		totAudio=RD.READER_ReSeekAudio();
		t-=totAudio;
		written=0;
		readpos=0;
		writepos=0;
		basesynchpos=tbasesynchpos=0;
		nextsynchpos=tnextsynchpos=0;
		nCurrentFrame=1;
		nShouldbe=(int64)((int64)t*(int64)vidinfo->video_frames/vidinfo->audio_bytes)+1;
		printf("shouldbe=%d %d\r\n",nShouldbe,vidinfo->video_pos);
		while(vidinfo->video_pos<nShouldbe)
		{
			RD.READER_NextVideoFrame(vid_temp,0);
			decode_frame((unsigned char*)vid_temp, 0,( unsigned char *)&avpict, 1, 0);
		}
		Exit();
		Init(0,0);
		//while (written<131072*ch*2) thd_sleep(100);
//		stream_pause();
		stream_init(get_data);
		while (written<131072*ch*2) thd_sleep(100);
		stream_start(Freq, ch-1);
		printf("resume %d %d\r\n",vidinfo->video_pos,totAudio);
	}
	else
	{
		nCurrentFrame=1;
		baseTick=jiffies*10;
		totMsTime=((double)(tot*1000))/vidinfo->fps;
	}

}

void PlayFastForward()
{
	AVPicture avpict;
	int render=1;
	if (!tot) return;
	if (!bPause&&(totAudio))
	{
		stream_pause();
		//bPause=!bPause;
		//thd_sleep(1000);
	}
	nShouldbe=0;
	RD.READER_NextVideoFrame(vid_temp,0);
	decode_frame((unsigned char*)vid_temp, 0,( unsigned char *)&avpict, 1, 0);
	RD.READER_NextVideoFrame(vid_temp,0);
	decode_frame((unsigned char*)vid_temp, 0,( unsigned char *)&avpict, 1, 0);
	RD.READER_NextVideoFrame(vid_temp,0);
	render=decode_frame((unsigned char*)vid_temp, 0,( unsigned char *)&avpict, 1, 0);
	if (totAudio &&(vidinfo->currentchunk.pos<(vidinfo->currentframe.pos-800000)) )
	{
		printf("keepup %d %d %d\r\n", vidinfo->currentchunk.pos,vidinfo->currentframe.pos, vidinfo->video_pos);
		RD.READER_ReadAudio((char *)mp3_buffer,MP3_BUFFER_SIZE);
		//pos+=MP3_BUFFER_SIZE;
		//printf("keepup %d %d\r\n", vidinfo->currentchunk.pos,vidinfo->currentframe.pos);
	}
	if (render)
	{
		convert_yuv(avpict.data[0], avpict.linesize[0],
		avpict.data[1], avpict.data[2], avpict.linesize[1],
		(unsigned char*)outputbuffer, g_nWidth,g_nHeight,512,0);
		outputbuffer=(char*)txset();
	}
	nCurrentFrame+=3;
}

void check_event()
{
	static uint8 mcont = 0;
	static int pressed=0;
	cont_cond_t cond;
	if (!mcont) {

		mcont = maple_first_controller();
		if (!mcont) { return; }
	}
	if (cont_get_cond(mcont, &cond)) 
	{
		printf("here\r\n");
		return; 
	}
	if (!(cond.buttons & CONT_Y)) 
	{
		bEnd=-2;
	}
	if ((!(cond.buttons & CONT_A))&&(!pressed)) 
	{
		int i=0;
		if (bPause&&(!totAudio))
		{
			baseTick=jiffies*10;
			tot=tot-nCurrentFrame;
			nCurrentFrame=1;
			totMsTime=((double)(tot*1000))/vidinfo->fps;
			bPause=false;
		}
		else if (!totAudio)
		{
			bPause=true;
			baseTick=0;
		}
		else
		{
			stream_pause();
		}
		//bPause=!bPause;
		pressed=1;
	}
	if (((cond.buttons & CONT_A))&&(pressed)) 
	{
		pressed=0;
	}
	if ((!(cond.buttons & CONT_B)))
	{
		bEnd=2;
	}
	if ((!(cond.buttons & CONT_X)))
	{
		bEnd=3;
	}
	if ((!(cond.buttons & CONT_DPAD_RIGHT))&&(vidinfo->video_frames>0))
	{
		if (!FastForward)
		{
			if (totAudio)
			{
				stream_stop();
				stream_shutdown();
			}
			//printf("FFWD\r\n");
			FastForward=1;
		}
		else
		{
			StopFastForward();
			//bPause=!bPause;
		}
	}

}
float GetPos();





void PlayFrame( )
{
	AVPicture avpict;
	int temp;
	if (nCurrentFrame>=tot)
	{
		return;
	}
	if (totAudio)
	{
		float fpos=GetPos();
		int diff=(nextsynchpos-basesynchpos);
		temp=(int) (fpos*(float)diff);
		temp+=basesynchpos;
		temp+=diff>>1;
		nShouldbe=(int64)((int64)temp*(int64)tot/totAudio)+1;
	}
	else
	{
		if (!bPause)
		{
			nShouldbe=(((int64)tot*(int64)(jiffies*10-baseTick))/totMsTime)+1;
			//nShouldbe=nCurrentFrame;
		}
		else
		{
			nShouldbe=0;
		}

	}
	int FrameDiff=nCurrentFrame-nShouldbe;
	if (FrameDiff<2&&(FrameDiff>-30)) 
	{
		RD.READER_NextVideoFrame(vid_temp,0);
		if (decode_frame((unsigned char*)vid_temp, 0,( unsigned char *)&avpict, 1, 0))
		{
			convert_yuv(avpict.data[0], avpict.linesize[0],
				avpict.data[1], avpict.data[2], avpict.linesize[1],
				(unsigned char*)outputbuffer, g_nWidth,g_nHeight,512,0);
			outputbuffer=(char*)txset();
		}
		nCurrentFrame++;
	}
	else if (FrameDiff<0)
	{
		RD.READER_NextVideoFrame(vid_temp,0);
		decode_frame((unsigned char*)vid_temp, 0,( unsigned char *)&avpict, 1, 0);
		RD.READER_NextVideoFrame(vid_temp,0);
		decode_frame((unsigned char*)vid_temp, 0,( unsigned char *)&avpict, 1, 0);
		Drop+=2;
		nCurrentFrame+=2;
	}
	else
	{
		if (FastForward) PlayFastForward();
		ReadAhead();
		//printf("waiting %d %d\r\n",nShouldbe,pos);
	}

}

static void* get_data(int req_size,int curbuffer) 
{
	void* temp;
	while(written<req_size&&(!DoneDecode))
	{
		thd_pass();
	}
	if (written<req_size)
	{
		if (!written)
		{
			return NULL;
		}
		memset(abuff,0,req_size);
		req_size=written;
	}
	if (!curbuffer)
	{
		if (readpos>req_size) tbasesynchpos=synchpos[(readpos-req_size)/BUFF_SIZE];
		else tbasesynchpos=synchpos[(1310720-req_size)/BUFF_SIZE];
		if (readpos+req_size>1310720) tnextsynchpos=synchpos[0];
		else tnextsynchpos=synchpos[(readpos+req_size)/BUFF_SIZE];
	}

	written-=req_size;
	temp= (void*)pbBuffer+readpos;
	if (readpos+req_size>=1310720)
	{
		memcpy(abuff,temp,1310720-readpos);
		memcpy(abuff+(1310720-readpos),pbBuffer,req_size-(1310720-readpos));
		readpos=0;
		if (readpos==1310720) readpos=0;
		return abuff;
	}
	else
	{
		readpos+=req_size;
		if (written==req_size)
		{
			memcpy(abuff,temp,req_size);
			if (readpos==1310720) readpos=0;
			return abuff;
		}
		else
		{
			if (readpos==1310720) readpos=0;
			return temp;
		}
	}
}


void audio(void *v)
{
	while (written<131072*ch*2) thd_sleep(100);
	stream_start(Freq, ch-1);
	while(stream_poll()>=0 &&(!bEnd))
	{
		check_event();
		//thd_pass();
		thd_sleep_jiffies(10);
	}
	Finished=1;
}
void Decode(void *v)
{
	char buffer[4800];
	int temp;
	int totalwritten=0;
	do 
	{	
		//printf("leftover %d bytes\r\n", status);
		if (status==32768) status=0;
		if (status>0) 
		{
			status--;
			memmove(mp3_buffer,mp3_buffer+(MP3_BUFFER_SIZE-status),status);
		}
		if (written>131072*ch) PlayFrame();
		ReadAhead();
		int nb_read=0;
		//printf("before %d\r\n",vidinfo->currentchunk.pos);
		nb_read=RD.READER_ReadAudio((char *)mp3_buffer+status,MP3_BUFFER_SIZE-status);
		//printf("after %d\r\n",vidinfo->currentchunk.pos);
		//printf("read one\r\n");
		if (!nb_read)
		{
			printf("Done\r\n");
			status=-1;
			continue;
		}
		do{
			if (written>131072*ch||(FastForward)) PlayFrame();
			if (bEnd) 
			{
				DoneDecode=1;
				return;
			}
		}while(written>=1000000||(FastForward));


//		printf("Status in=%d\r\n",status);
		status=DecompressAudio((unsigned char*) mp3_buffer,nb_read+status,buffer,4800,&played,&pos) ; 
//		printf("Status=%d\r\n",status);
		synchpos[writepos/BUFF_SIZE]=pos;
		if (written>131072*ch) PlayFrame();

		totalwritten+=played;
		if (writepos+played>=1310720)
		{
			memcpy((char*)pbBuffer+writepos,buffer,1310720-writepos);
			memcpy((char*)pbBuffer,buffer+(1310720-writepos),played-(1310720-writepos));
			writepos+=played-(1310720-writepos);
			writepos=0;
		}
		else
		{
			memcpy((char*)pbBuffer+writepos,buffer,played);
			writepos+=played;
		}
		if (writepos==1310720) writepos=0;
		written+=played;

		if (!status)
		{
			do 
			{	
				if (!bPause)
				{

					do{
						if (written>131072*ch) PlayFrame();
						if (bEnd) 
						{
							DoneDecode=1;
							return;
						}
					}while(written>=1000000);
					status=DecompressAudio(NULL,0,buffer,4800,&played,&pos) ; 
					//if (status>MP3_BUFFER_SIZE) printf("Level 2 failed");
					synchpos[writepos/BUFF_SIZE]=pos;
					if (written>131072*ch) PlayFrame();
					totalwritten+=played;
					if (writepos+played>=1310720)
					{
						memcpy((char*)pbBuffer+writepos,buffer,1310720-writepos);
						memcpy((char*)pbBuffer,buffer+(1310720-writepos),played-(1310720-writepos));
						writepos+=played-(1310720-writepos);
						writepos=0;
					}
					else
					{
						memcpy((char*)pbBuffer+writepos,buffer,played);
						writepos+=played;
					}
					if (writepos==1310720) writepos=0;
					written+=played;
				}
				else
				{
					if (FastForward) PlayFrame();
					status = 0;
				}
			}while ((!status)&&(!bEnd));
		}
    } while ((status > 0)&&(!bEnd));
	DoneDecode=1;
	printf("done audio\r\n");
	while (!Finished&&(!bEnd))
	{
		PlayFrame();
	}
	printf("done all\r\n");
}

int decore_release();

void decEnd()
{
	decore_release();
}

int play_divx(char *filename)
{
pos=0;	
 basesynchpos=tbasesynchpos=0;
 nextsynchpos=tnextsynchpos=0;
 int i;
if (!mp3_buffer) mp3_buffer=malloc(MP3_BUFFER_SIZE);
for (i=0;i<1200;i++)
{
	synchpos[i]=0;
}

	printf("starting %s\r\n",filename);
	if (strstr(filename,".mp3")||(strstr(filename,".MP3")))
	{
		InitializeReaderMP3(&RD);
		DecompressAudio=DecompressAudio_MP3;
		Init=Init_MP3;
		Exit=Exit_MP3;
		GetHeaderInfo=GetHeaderInfo_MP3;
	}
	else if (strstr(filename,".ogg")||(strstr(filename,".OGG")))
	{
		InitializeReaderMP3(&RD);
		DecompressAudio=DecompressAudio_OGG;
		Init=Init_OGG;
		Exit=Exit_OGG;
		GetHeaderInfo=GetHeaderInfo_OGG;
	}
	else
	{
		InitializeReaderAVI(&RD);
		DecompressAudio=DecompressAudio_MP3;
		Init=Init_MP3;
		Exit=Exit_MP3;
		GetHeaderInfo=GetHeaderInfo_MP3;
	}
	if (!RD.READER_Open(filename, 1,16,&vidinfo))
	{
		printf("read file failed %s\r\n",filename);
		return 0;
	}
	
	g_nWidth=vidinfo->width;
	g_nHeight=vidinfo->height;
	tot=vidinfo->video_frames;
	printf("total frames: %d \r\n", tot);
	printf("width: %d \r\n", g_nWidth);
	printf("height: %d \r\n", g_nHeight);
	printf("codec:");
	printf(vidinfo->compressor);
	printf("\r\n");
	totAudio=vidinfo->audio_bytes; //Freq*ch*2*(tot/vidinfo->fps);//vidinfo->audio_bytes;
	totMsTime=((double)(tot*1000))/vidinfo->fps;
	baseTick=jiffies*10;
	printf("total audio bytes=%d",totAudio);
	decode=decore;
	decode_frame=decore_frame;
	if (strstr(vidinfo->compressor,"div3")||(strstr(vidinfo->compressor,"DIV3"))||(strstr(vidinfo->compressor,"DIV4"))||(strstr(vidinfo->compressor,"div4"))||(strstr(vidinfo->compressor,"mp43"))||(strstr(vidinfo->compressor,"MP43")))
	{
		decode=decore_Div3;
		decode_frame=decore_frame_Div3;
	}
	else if (strstr(vidinfo->compressor,"vp31")||(strstr(vidinfo->compressor,"vp31")))
	{
		decode=decore_vp3;
		decode_frame=decore_frame_vp3;
	}
	written=0;
	writepos=0;
	readpos=0;
	Drop=0;
	decBegin(0);
	nCurrentFrame=1;
	if (!outputbuffer)
		outputbuffer=(char*)txinit();
	txclear(g_nWidth,g_nHeight);
	
	bEnd=0;
	Init(0, (char*)0);
	ReadAhead();
	RD.READER_ReadAudio((char *)mp3_buffer,MP3_BUFFER_SIZE-1);
	Freq=0;
	printf("before header\r\n");
	while((status=GetHeaderInfo((unsigned char*)mp3_buffer,MP3_BUFFER_SIZE-1,&Freq,&ch,&br))!=0)
	{
		if (status==32768) status=0;
		if (status>0) 
		{
			status--;
			memmove(mp3_buffer,mp3_buffer+(MP3_BUFFER_SIZE-status),status);
		}
		RD.READER_ReadAudio((char *)mp3_buffer+status,MP3_BUFFER_SIZE-status);
	}
	printf("got past header\r\n");
	//GetHeaderInfo((unsigned char*)mp3_buffer,MP3_BUFFER_SIZE-1,&Freq,&ch,&br);
	memmove(mp3_buffer+1,mp3_buffer,MP3_BUFFER_SIZE-1);
	status=MP3_BUFFER_SIZE;
	BUFF_SIZE=Freq>22050?2304*ch:1152*ch;

	Finished=0;
	DoneDecode=0;
//	totAudio=0;
//	vidinfo->audio_bytes=0;
	if (totAudio)
	{
		stream_init(get_data);
		thd_create(audio,NULL);
		Decode(0);
		stream_stop();
		stream_shutdown();

		timer_spin_sleep(1000);
		Exit();
	}
	else
	{
		while (!bEnd&&(vidinfo->video_pos<(vidinfo->video_frames-1)))
		{
			PlayFrame();
			if ((!(jiffies % 5))&&(nCurrentFrame>100)) check_event();
		}
	}
	printf("played %d frames decode complete drop=%d\r\n",nCurrentFrame,Drop);
	decEnd();
	RD.READER_Close();
	malloc_stats();
	return bEnd;
}

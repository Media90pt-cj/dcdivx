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
 **************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif 

#ifndef AVI_DECAPS_H
#define AVI_DECAPS_H

/*
 * includes
 */
#include <malloc.h>
#include <stdlib.h>

/*
 * SOME USEFUL MACROS
 */

//#define AVI_MAX_LEN 2000000000
//#define HEADERBYTES 2048
#define PAD_EVEN(x) ( ((x)+1) & ~1 )
#define AVIIF_KEYFRAME	0x00000010L

/*
 * INDEX TYPES
 */

typedef struct
{
  long pos;
  long len;
  char flags;

} video_index_entry;

typedef struct
{
   long pos;
   long len;
   long tot;

} audio_index_entry;

//enum {
//
//	INPUT_TYPE_FILE,INPUT_TYPE_CACHEFILE,
//	INPUT_TYPE_HTTP
//};

/*
 * File mode enum
 */

//enum {

//	INPUT_OPEN_ASCII,
//	INPUT_OPEN_BINARY
//};

/*
 * Seek Enum
 */

enum {

	INPUT_SEEK_SET,
	INPUT_SEEK_CUR,
	INPUT_SEEK_END
};

/*
 * Main Class
 */

typedef struct _videoinfo
{
    long               width;             
    long               height;            
    double             fps;               
    char               compressor[8];     
    long               video_strn;        
    long               video_frames;      
    char               video_tag[4];      
    long               video_pos;         
    long               a_fmt;             
//    long               a_chans;           
//    long               a_rate;            
//    long               a_bits;            

    long               audio_strn;        
    long               audio_bytes;       
    long               audio_chunks;      
    char               audio_tag[4];      
    long               audio_posc;        
    long               audio_posb;        
    long               pos;               
    long               n_idx;             
    long               max_idx;           
    char               (*idx)[16]; 
    video_index_entry *video_index;
    audio_index_entry *audio_index;
    long               last_pos;          
    long               last_len;          
    long               movi_start;

	//HANDLE             hIOMutex;
	char				m_lpFilename[200];
	int					m_type;
	int					threshhold;
	video_index_entry	currentframe;
	audio_index_entry	currentchunk;

} videoinfo;

typedef int (*pREADER_Open)(char* lpFilename, int type, int maxsize,videoinfo** info);

typedef int (*pREADER_ReadAudio)(char *audbuf, int bytes);

typedef int (*pREADER_NextVideoFrame)(char *buffer, int drop);

typedef int (*pREADER_Seek)(int percent);

typedef int (*pREADER_ReSeekAudio)();

typedef double (*pREADER_GetProgress)();

typedef int (*pREADER_Close)();






typedef struct _reader
{
	pREADER_Open READER_Open;
	pREADER_ReadAudio READER_ReadAudio;
	pREADER_NextVideoFrame READER_NextVideoFrame;
	pREADER_Seek READER_Seek;
	pREADER_ReSeekAudio READER_ReSeekAudio;
	pREADER_GetProgress READER_GetProgress;
	pREADER_Close READER_Close;

}reader;


    

	int AVIDecaps_IsAVI();
	int AVIDecaps_FillHeader(int getIndex);
	int AVIDecaps_AddIndexEntry(char *tag, 
			          long flags, 
			          long pos, 
			          long len);
	int AVIDecaps_isKeyframe(long frame);
	int AVIDecaps_ReadAudio(char *audbuf, int bytes);


	int               AVIDecaps_Open(char* lpFilename, int Cache, int CacheSize,videoinfo** vidinfo);
	double            AVIDecaps_FrameRate();
	int               AVIDecaps_NextVideoFrame(char *buffer, int drop);

	int               AVIDecaps_VideoSeek(long frame);
	int               AVIDecaps_AudioSeek(long bytes);

	int               AVIDecaps_NextKeyFrame();
	int				  AVIDecaps_PreviousKeyFrame();

	int               AVIDecaps_Seek(int percent);
	int               AVIDecaps_ReSeekAudio();

//	int               AVIDecaps_SampleSize();

	int AVIDecaps_Rewind();

	double AVIDecaps_GetProgress();

	int AVIDecaps_Close();
void InitializeReaderAVI(reader* rd);
void InitializeReaderMP3(reader* rd);

#endif
#ifdef __cplusplus
}
#endif 

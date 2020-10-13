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
#include "kos.h"
#include "AVIDecaps.h"
#include "inputmedia.h"

videoinfo thisvid;
char           data[256];

//#include "inputmedia.h"
/*
 * Some useful functions
 */

/*
 * copy n into dst as a 4 byte, 
 * little endian number.
 * should also work on 
 * big endian machines 
 *
 */
int nMalloc=0;
int abytes_read=0;
static void long2str(char *dst, int n)
{
	dst[0] = (n      ) & 0xff;
	dst[1] = (n >>  8) & 0xff;
	dst[2] = (n >> 16) & 0xff;
	dst[3] = (n >> 24) & 0xff;
}

/* Convert a string of 4 
 * or 2 bytes to a number,
 * also working on big 
 * endian machines 
 *
 */

unsigned long str2ulong(char *str)
{
    unsigned long result;

	result = ( ((unsigned long) str[0] & 0xFF) | ( ((unsigned long)str[1] &0xFF) << 8) |
			  (((unsigned long) str[2] & 0xFF) << 16) | ( ((unsigned long) str[3] & 0xFF) << 24) );

	return result;
}

static unsigned long str2ushort(char *str)
{
	return ( str[0] | (str[1]<<8) );
}

/*
 * Reads the first 12 bytes of
 * the file, and returns TRUE
 * if the file is an AVI.
 *
 */

int AVIDecaps_IsAVI()
{
	//char data[24];
	
//	if(thisvid.input != NULL) {
		
		if( InputMediaRead(data, 12) != 12 ) {
			
			//MessageBox(NULL, "Error Reading", "Info", MB_OK);
			return 0;
		}

		
		if( strncmp(data, "RIFF", 4) !=0 || 
			strncmp(data + 8, "AVI ", 4) !=0 ) {
			return 0;
		}
		
		return 1;
	
	return 0;
}



/*
 * Fill the class with info from headers
 * and reconstruct an index if wanted.
 *
 */

int AVIDecaps_FillHeader(int getIndex)
{
	unsigned long  i, n, rate; 
	char          *hdrl_data;
	long           hdrl_len = 0;
	int            scale,lasttag = 0;
	int            vids_strh_seen = 0;
	int            vids_strf_seen = 0;
	int            auds_strh_seen = 0;
	int            auds_strf_seen = 0;
	int            num_stream = 0;
//	char           data[256];
	
	/* go through the AVI file and 
    * extract the header list,
    * the start position of the 'movi' 
    * list and an optionally
    * present idx1 tag 
    *
    */
	
	hdrl_data = 0;

	while(1)
    {
		if( InputMediaRead(data, 8) != 8 ) 
			break; 
	
			/*
			 * we assume it's EOF 
			 *
		     */
		
		n = str2ulong(data + 4);
		n = PAD_EVEN(n);
		
		if(strncmp(data, "LIST", 4) == 0)
		{
			if( InputMediaRead(data, 4) != 4 ) {

				return 0;
			}
			
			n -= 4;
			
			if(strncmp(data, "hdrl", 4) == 0)
			{
				hdrl_len = n;
				if (hdrl_data) free(hdrl_data);
				hdrl_data = (char *) malloc(n);
				
				if(hdrl_data == 0) { 
					
					return 0;
				}
				
				if( InputMediaRead(hdrl_data, n) != n ) {
					
					return 0;
				}
			}
			else if(strncmp(data, "movi", 4) == 0)
			{
				thisvid.movi_start = InputMediaSeek(0, INPUT_SEEK_CUR);
				//break;
				InputMediaSeek(n, INPUT_SEEK_CUR);
			}
			else
			{
				InputMediaSeek(n, INPUT_SEEK_CUR);
				//break;
			}
		}
		else if(strncmp(data,"idx1",4) == 0)
		{
			thisvid.audio_bytes=0;
			unsigned long read;
			thisvid.n_idx = thisvid.max_idx = n/16;
			for (i=0;i<(thisvid.n_idx/1000)+1;i++)
			{
				char temp[16000];
				int toread=16000;
				int i2;
				if (i==(thisvid.n_idx/1000)) 
					toread=(thisvid.n_idx%1000)*16;
				if( (read = InputMediaRead((char *) temp, toread)) != toread) {
					
					break;
				}
				for (i2=0;i2<read/16;i2++)
				{
					if(strncmp(temp+(16*i2)+2, "wb", 2) == 0)
					{
						thisvid.audio_bytes += str2ulong(temp+(16*i2)+12);												
					}
				}
			}

		}
		else {
		  
			InputMediaSeek(n, INPUT_SEEK_CUR);
		}
	}
	
	if(!hdrl_data) {
		
		return 0;
	}
	
	if(!thisvid.movi_start) {
		
		return 0;
	}
	
	/* 
    * interpret the header list 
    *
    */
	
	for(i=0; i < hdrl_len; )
	{
	/* 
	* list tags are completly ignored 
	*
		*/
		
		if(strncmp(hdrl_data + i, "LIST", 4)==0) 
		{ 
			i+= 12; 
			continue; 
		}
		
		n = str2ulong(hdrl_data+i+4);
		n = PAD_EVEN(n);
		
		/* 
		* interpret the tag and its args 
		*
		*/
		
		if(strncmp(hdrl_data + i, "strh", 4)==0)
		{
			i += 8;
			if(strncmp(hdrl_data + i, "vids", 4) == 0 && !vids_strh_seen)
			{
				memcpy(thisvid.compressor,hdrl_data+i+4,4);
				
				thisvid.compressor[4] = 0;
				
				scale = str2ulong(hdrl_data+i+20);
				rate  = str2ulong(hdrl_data+i+24);
				
				if(scale!=0) 
					thisvid.fps = (double)rate/(double)scale;
				
				thisvid.video_frames = str2ulong(hdrl_data+i+32);
				thisvid.video_strn = num_stream;
				
				vids_strh_seen = 1;
				lasttag = 1; 
			}
			else if (strncmp (hdrl_data + i, "auds", 4) == 0 && ! auds_strh_seen)
			{
				//thisvid.audio_bytes = str2ulong(hdrl_data + i + 32);//AVIDecaps_SampleSize();
				thisvid.audio_strn = num_stream;
				
				auds_strh_seen = 1;
				lasttag = 2; 
			}
			else
				lasttag = 0;
			num_stream++;
		}
		else if(strncmp(hdrl_data + i, "strf", 4)==0)
		{
			i += 8;
			if(lasttag == 1)
			{
				
			/*
			* keep a copy of 
			* the bitmapinfoheader
			*
				*/
				
				
				thisvid.width  = str2ulong(hdrl_data+i+4);
				thisvid.height = str2ulong(hdrl_data+i+8);
				vids_strf_seen = 1;
			}
			else if(lasttag == 2)
			{
				thisvid.a_fmt   = str2ushort(hdrl_data + i  );
			}
			lasttag = 0;
		}
		else
		{
			i += 8;
			lasttag = 0;
		}
		
		i += n;
	}
	
	if (hdrl_data) free(hdrl_data);
	hdrl_data=0;
	if(!vids_strh_seen || !vids_strf_seen || thisvid.video_frames==0) { 
		
		return 0;
	}
	
	thisvid.video_tag[0] = thisvid.video_strn/10 + '0';
	thisvid.video_tag[1] = thisvid.video_strn%10 + '0';
	thisvid.video_tag[2] = 'd';
	thisvid.video_tag[3] = 'b';
	
	/* 
    * audio tag is set to "99wb" 
    * if no audio present 
    *
    */
	
	thisvid.audio_tag[0] = thisvid.audio_strn/10 + '0';
	thisvid.audio_tag[1] = thisvid.audio_strn%10 + '0';
	thisvid.audio_tag[2] = 'w';
	thisvid.audio_tag[3] = 'b';
	
	//InputMediaSeek(thisvid.movi_start, INPUT_SEEK_SET);
	thisvid.currentchunk.pos=thisvid.movi_start;
	thisvid.currentchunk.len=0;
	thisvid.currentframe.pos=thisvid.movi_start;
	thisvid.currentframe.len=0;
	/* 
     * reposition the file 
     *
     */
	//InputMediaSeek(0, 
	//	INPUT_SEEK_SET);

	InputMediaClose();
	//InputMediaOpen(thisvid.m_lpFilename,INPUT_OPEN_BINARY,thisvid.m_type,2000000);
	InputMediaOpen(thisvid.m_lpFilename, 0, 0,2000000,1);
	InputMediaSeek(thisvid.movi_start, 
		INPUT_SEEK_SET);
	
	thisvid.video_pos = 0;
	return 1;
}



/*
 * Tries to open an AVI
 * with and without an index
 */

int AVIDecaps_Open(char* lpFilename, int type, int size,videoinfo** vid)
{
	int ret=0;

	strcpy(thisvid.m_lpFilename,lpFilename);
	thisvid.m_type=type;
	if (!InputMediaOpen(lpFilename, 0, type,2000000,0))
	{
		return 0;
	}

	thisvid.video_pos  = 0;
	thisvid.audio_posc = 0;
	thisvid.audio_posb = 0;

	thisvid.idx         = NULL;
	thisvid.video_index = NULL;
	thisvid.audio_index = NULL;

	if(AVIDecaps_IsAVI()) {

		
		if(AVIDecaps_FillHeader(1)) {
		
			ret= 1;
		}
		else {

			InputMediaSeek(0, INPUT_SEEK_SET);

			AVIDecaps_IsAVI();
			
			if(AVIDecaps_FillHeader(0)) {
		
				ret=1;
			}
		}
	}
	*vid=&::thisvid;
	return ret;
}

void GetNextChunkInfo()
{
	unsigned long  n; 
	long           hdrl_len = 0;
	int            lasttag = 0;
	int            vids_strh_seen = 0;
	int            vids_strf_seen = 0;
	int            auds_strh_seen = 0;
	int            auds_strf_seen = 0;
	int            num_stream = 0;
	InputMediaSeek(PAD_EVEN(thisvid.currentchunk.pos+thisvid.currentchunk.len),INPUT_SEEK_SET);
	while(1)
	{
		
		if( InputMediaRead(data, 8) != 8 ) 
		{
			thisvid.currentchunk.pos+=thisvid.currentchunk.len;
			thisvid.currentchunk.len=0;
			break;
		}
		
		n = str2ulong(data + 4);
		
		/* 
		* the movi list may contain sub-lists, 
		* ignore them 
		*
		*/
		
		if(strncmp(data,"LIST", 4) == 0)
		{
			InputMediaSeek(4, INPUT_SEEK_CUR);
			continue;
		}
		
		/* 
		* check if we got a tag ##db, 
		* ##dc or ##wb 
		* 
		*/
		
		if( ( (data[2]=='d' || data[2]=='D') &&
			(data[3]=='b' || data[3]=='B' || data[3]=='c' || data[3]=='C') ) || 
			( (data[2]=='w' || data[2]=='W') &&
			(data[3]=='b' || data[3]=='B') ) )
		{
			if(strncmp(data, thisvid.audio_tag, 4) == 0)
			{
				thisvid.currentchunk.pos = InputMediaSeek(0, INPUT_SEEK_CUR);
				thisvid.currentchunk.len = n;
				return;				
			}
		}
		
		InputMediaSeek(PAD_EVEN(n),
			INPUT_SEEK_CUR);
	}
	
	
}

void GetNextFrameInfo()
{
	unsigned long  n; 
	long           hdrl_len = 0;
	int            lasttag = 0;
	int            vids_strh_seen = 0;
	int            vids_strf_seen = 0;
	int            auds_strh_seen = 0;
	int            auds_strf_seen = 0;
	int            num_stream = 0;
	char           data[256];
	InputMediaSeek(PAD_EVEN(thisvid.currentframe.pos+thisvid.currentframe.len),INPUT_SEEK_SET);
	while(1)
	{
		
		if( InputMediaRead(data, 8) != 8 ) 
			break;
		
		n = str2ulong(data + 4);
		
		/* 
		* the movi list may contain sub-lists, 
		* ignore them 
		*
		*/
		
		if(strncmp(data,"LIST", 4) == 0)
		{
			InputMediaSeek(4, INPUT_SEEK_CUR);
			continue;
		}
		
		/* 
		* check if we got a tag ##db, 
		* ##dc or ##wb 
		* 
		*/
		
		if( ( (data[2]=='d' || data[2]=='D') &&
			(data[3]=='b' || data[3]=='B' || data[3]=='c' || data[3]=='C') ) || 
			( (data[2]=='w' || data[2]=='W') &&
			(data[3]=='b' || data[3]=='B') ) )
		{
			if(strncmp(data, thisvid.video_tag, 3) == 0)
			{
				thisvid.currentframe.flags = 0;//str2ulong(thisvid.idx[i]+ 4);
				thisvid.currentframe.pos   = InputMediaSeek(0, INPUT_SEEK_CUR);
				thisvid.currentframe.len   = n;
				return;				
			}
		}
		
		InputMediaSeek(PAD_EVEN(n),
			INPUT_SEEK_CUR);
	}
	
	
}
/*
 * Returns the wavefromatex
 * associated with the first audio 
 * stream.
 */
	
/*
 * Reads the next video Frame into
 * buffer, return the actual size of
 * the frame.
 *
 */

int AVIDecaps_NextVideoFrame(char *buffer, int drop)
{
   //unsigned int n;

   //if(thisvid.video_pos >= thisvid.video_frames) {
   //  
//	   return -2;
   //}
   /*
    * Request the mutex 
    * for reading the file;
    */
   GetNextFrameInfo();
   //n = thisvid.video_index[thisvid.video_pos].len;
   InputMediaSeek(thisvid.currentframe.pos, INPUT_SEEK_SET);

   InputMediaRead(buffer, thisvid.currentframe.len);

   thisvid.video_pos++;

   /*
    * Release the Mutex.
    */

   return thisvid.currentframe.len;
}

/*
 * Reads any amount of audio
 * data. FIXME : should return
 * the actual number read.
 */

int AVIDecaps_ReadAudio(char *audbuf, int bytes)
{
	int nr, left = 0, todo;


   nr = 0; 

   /*
    * Request the read Mutex 
    */

   /*
    * We loop until we parsed enough
	* chunks for the amount we want
    *
    */

   while(bytes > 0)
   {
      left = thisvid.currentchunk.len - thisvid.audio_posb;

      if(!left)
      {
         //if(thisvid.audio_posc>=thisvid.audio_chunks-1) 
		 //{
		//	ReleaseMutex(thisvid.hIOMutex);
		//	return nr;
		// }
         //thisvid.audio_posc++;
         thisvid.audio_posb = 0;
		 GetNextChunkInfo();
		 if (!thisvid.currentchunk.len)
		 {
			return nr;
		 }
         continue;
      }
      if(bytes<left)
         todo = bytes;
      else
         todo = left;
      
	  InputMediaSeek(thisvid.currentchunk.pos + thisvid.audio_posb, INPUT_SEEK_SET);

      InputMediaRead(audbuf + nr, todo);

      bytes -= todo;
      nr    += todo;
      thisvid.audio_posb += todo;
   }

   /*
    * And release the Mutex.
    */

	abytes_read+=nr;
   return nr;
}

/*
 * Return the actual framerate
 * FIXME : should be a double...
 *
 */

double AVIDecaps_FrameRate()
{
	/*
	 * Fix for some trailers
	 */

	if(thisvid.fps == 0)
		thisvid.fps = 25;

	if(thisvid.fps == 23)
		thisvid.fps = 25;

	return (double) thisvid.fps;
}


/*
 * Seek to a particular 
 * video frame.
 */


int AVIDecaps_Seek(int percent)
{
	return thisvid.audio_bytes;
}

int AVIDecaps_ReSeekAudio()
{
	double ratio;
	char temp[512];
	int nShouldbe;
//	  nShouldbe=0;
	  nShouldbe=(abytes_read*(int64)thisvid.video_frames)/thisvid.audio_bytes+1;
	  while(nShouldbe<thisvid.video_pos)
	  {
		  AVIDecaps_ReadAudio(temp,512);
		  nShouldbe=(abytes_read*(int64)thisvid.video_frames)/thisvid.audio_bytes+1;
	  }


    return thisvid.audio_bytes-abytes_read;
}


double AVIDecaps_GetProgress()
{
	return (double) ((double)(thisvid.video_pos))*100.0/((double)thisvid.video_frames);
}


int AVIDecaps_Close()
{
	InputMediaClose();
	return 1;
}

void InitializeReaderAVI(reader* rd)
{
	rd->READER_Open=AVIDecaps_Open;
	rd->READER_ReadAudio=AVIDecaps_ReadAudio;
	rd->READER_NextVideoFrame=AVIDecaps_NextVideoFrame;
	rd->READER_Seek=AVIDecaps_Seek;
	rd->READER_ReSeekAudio=AVIDecaps_ReSeekAudio;
	rd->READER_GetProgress=AVIDecaps_GetProgress;
	rd->READER_Close=AVIDecaps_Close;

}


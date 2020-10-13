/**************************************************************************************
 *                                                                                    *
 * This application contains code from OpenDivX and is released as a "Larger Work"    *
 * under that license. Consistant with that license, this application is released     *
 * under the GNU General Public License.                                              *
 *                                                                                    *
 * The OpenDivX license can be found at: http://www.projectmayo.com/opendivx/docs.php *
 * The GPL can be found at: http://www.gnu.org/copyleft/gpl.html                      *
 *                                                                                    *
 *                                                                                    *
 * Authors: Damien Chavarria                                                          *
 *          DivX Advanced Research Center <darc at projectmayo.com>                   *
 *          Marc Dukette                                                              *
 **************************************************************************************/


#include "AviDecaps.h"
#include "inputmedia.h"
int CacheSize;
videoinfo thisvid;

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
	char data[24];
	
		
		if( InputMediaRead(data, 12) != 12 ) {
			
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
	long           scale, idx_type;
	char          *hdrl_data;
	long           hdrl_len = 0;
	long           nvi=0, nai=0, ioff;
	long           tot=0;
	int            lasttag = 0;
	int            vids_strh_seen = 0;
	int            auds_strh_seen = 0;
	int            num_stream = 0;
	char           data[256];

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
				
				InputMediaSeek(n, INPUT_SEEK_CUR);
			}
			else
				InputMediaSeek(n, INPUT_SEEK_CUR);
		}
		else if(strncmp(data,"idx1",4) == 0)
		{

			thisvid.n_idx = thisvid.max_idx = n/16;
			if (thisvid.idx) free(thisvid.idx);
			thisvid.video_index = (video_index_entry *) malloc(100000*sizeof(video_index_entry));
			if (!thisvid.video_index)
			{
				return 0;
			}
			thisvid.audio_index = (audio_index_entry *) malloc(100000*sizeof(audio_index_entry));
			if (!thisvid.audio_index)
			{
				free(thisvid.video_index);
				thisvid.video_index=0;
				return 0;
			}
			tot=0;
			unsigned long read;
			ioff = thisvid.movi_start+4;
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
					if(strncmp(temp+(16*i2)+2, "db", 1) == 0)
					{
						if(nvi%100000==0)
						{
							void* ptr = realloc((void *)thisvid.video_index,
								(nvi+100000)*sizeof(video_index_entry));
							
							if (!ptr)
							{
								free(thisvid.video_index);
								thisvid.video_index=0;
								free(thisvid.audio_index);
								thisvid.audio_index=0;
								return 0;
							}
							
							thisvid.video_index = (video_index_entry*)ptr;
						}
						thisvid.video_index[nvi].flags = (char) (str2ulong(temp+(16*i2) +4) & AVIIF_KEYFRAME)>1;
						thisvid.video_index[nvi].pos   = str2ulong(temp+(16*i2)+ 8)+ioff;
						thisvid.video_index[nvi].len   = str2ulong(temp+(16*i2)+12);
						
						nvi++;
					}
					else if(strncmp(temp+(16*i2)+2, "wb", 2) == 0)
					{
						if(nai%100000==0)
						{
							void* ptr = realloc((void *)thisvid.audio_index,
								(nai+100000)*sizeof(audio_index_entry));
							
							if (!ptr)
							{
								free(thisvid.video_index);
								thisvid.video_index=0;
								free(thisvid.audio_index);
								thisvid.audio_index=0;
								return 0;
							}
							
							thisvid.audio_index = (audio_index_entry*)ptr;
						}
						thisvid.audio_index[nai].pos = str2ulong(temp+(16*i2)+ 8)+ioff;
						thisvid.audio_index[nai].len = str2ulong(temp+(16*i2)+12);
						thisvid.audio_index[nai].tot = tot;
						
						tot += thisvid.audio_index[nai].len;
						
						nai++;
					}
				}
			}

			void* ptr = realloc((void *)thisvid.video_index,
				(nvi)*sizeof(video_index_entry));
						
			thisvid.video_index = (video_index_entry*)ptr;

			ptr = realloc((void *)thisvid.audio_index,
							(nai)*sizeof(audio_index_entry));
			thisvid.audio_index = (audio_index_entry*)ptr;
		}
		else {
		  
			InputMediaSeek(n, INPUT_SEEK_CUR);
		}
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
				thisvid.audio_bytes = str2ulong(hdrl_data + i + 32); //*AVIDecaps_SampleSize();
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
								
				thisvid.width  = str2ulong(hdrl_data+i+4);
				thisvid.height = str2ulong(hdrl_data+i+8);
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
	
	if (hdrl_data) 
		free(hdrl_data);
	hdrl_data=0;
	
	thisvid.video_tag[0] = thisvid.video_strn/10 + '0';
	thisvid.video_tag[1] = thisvid.video_strn%10 + '0';
	thisvid.video_tag[2] = 'd';
	thisvid.video_tag[3] = 'b';
	
	thisvid.audio_tag[0] = thisvid.audio_strn/10 + '0';
	thisvid.audio_tag[1] = thisvid.audio_strn%10 + '0';
	thisvid.audio_tag[2] = 'w';
	thisvid.audio_tag[3] = 'b';
	
	//InputMediaSeek(thisvid.movi_start, INPUT_SEEK_SET);
	
	
	idx_type = 0;
	thisvid.audio_chunks = nai;
	
	thisvid.idx=0;
	thisvid.audio_bytes = tot;
	/* 
     * reposition the file 
     *
     */
	InputMediaClose();
	int fsize=InputMediaOpen(thisvid.m_lpFilename,0,0,2000000,CacheSize);
	if (!fsize) return 0;
	//int time=(((double)thisvid.video_frames)/((double)thisvid.fps));
	//int abytes=(int)(time*32000)/8;
	//thisvid.threshhold=((double)((fsize-abytes)/time)/thisvid.fps)*(4);
	//Sleep(1000);
	InputMediaSeek(thisvid.movi_start, 
		INPUT_SEEK_SET);
	
	thisvid.video_pos = 0;
	return 1;
}


/*
 * Reading Functions
 *
 */

/*
 * Tries to open an AVI
 * with and without an index
 */

int AVIDecaps_Open(char* lpFilename, int Cache, int CacheSize,videoinfo** vidinfo)
{
	int ret=0;

	strcpy(thisvid.m_lpFilename,lpFilename);
	thisvid.m_type=Cache;
	::CacheSize=CacheSize;
	if (!InputMediaOpen(lpFilename, 0, 0,-1,1000000))
	{
		return 0;
	}
	//thisvid.hIOMutex = CreateMutex (NULL, false, NULL);

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
	}
	*vidinfo=&::thisvid;
	return ret;
}


/*
 * Reads the next video Frame into
 * buffer, return the actual size of
 * the frame.
 *
 */

int AVIDecaps_NextVideoFrame(char *buffer, int drop)
{
   unsigned int n;

//	printf("this=%d\r\n",thisvid.video_pos);

   n = thisvid.video_index[thisvid.video_pos].len;
  // WaitForSingleObject(thisvid.hIOMutex, INFINITE);

   InputMediaSeek(thisvid.video_index[thisvid.video_pos].pos, INPUT_SEEK_SET);

   InputMediaRead(buffer, n);
   //ReleaseMutex(thisvid.hIOMutex);

   thisvid.video_pos++;

//	printf("next=%d\r\n",thisvid.video_pos);
   return n;
}

/*
 * Reads any amount of audio
 * data. FIXME : should return
 * the actual number read.
 */

int AVIDecaps_ReadAudio(char *audbuf, int bytes)
{
	int nr, left = 0, todo;
	int tempb=bytes;

   nr = 0; 


   /*
    * We loop until we parsed enough
	* chunks for the amount we want
    *
    */

   while(bytes > 0)
   {
      left = thisvid.audio_index[thisvid.audio_posc].len - thisvid.audio_posb;

      if(!left)
      {
         if(thisvid.audio_posc>=thisvid.audio_chunks-1) 
		 {
			//ReleaseMutex(thisvid.hIOMutex);
			return nr;
		 }
         thisvid.audio_posc++;
         thisvid.audio_posb = 0;
         continue;
      }
      if(bytes<left)
         todo = bytes;
      else
         todo = left;
      
	  //WaitForSingleObject(thisvid.hIOMutex, INFINITE);
	  InputMediaSeek(thisvid.audio_index[thisvid.audio_posc].pos + thisvid.audio_posb, INPUT_SEEK_SET);

      InputMediaRead(audbuf + nr, todo);
	 // ReleaseMutex(thisvid.hIOMutex);

      bytes -= todo;
      nr    += todo;
      thisvid.audio_posb += todo;
   }


	if (!nr) printf("read failed pos=%d bytes=%d\r\n",InputMediaSeek(0,INPUT_SEEK_CUR),tempb);
   return nr;
}


double AVIDecaps_FrameRate()
{

	return (double) thisvid.fps;
}


int AVIDecaps_AudioSeek(long bytes)
{
   long n0, n1, n;

   if(!thisvid.audio_index) { 
       return -1; 
   }

   if(bytes < 0) 
	   bytes = 0;

   n0 = 0;
   n1 = thisvid.audio_chunks;

   while(n0 < n1 - 1)
   {
      n = (n0 + n1) / 2;
      if(thisvid.audio_index[n].tot > bytes)
         n1 = n;
      else
         n0 = n;
   }

   thisvid.audio_posc = n0;

   if(thisvid.audio_index[n0].len > 1000) {
     thisvid.audio_posb = bytes - thisvid.audio_index[n0].tot;
   }
   else {
     thisvid.audio_posb = 0;
   }

   return 0;
}

int AVIDecaps_isKeyframe(long frame)
{
  return thisvid.video_index[frame].flags; // & AVIIF_KEYFRAME;
}

int AVIDecaps_Seek(int percent)
{
	long frame;
	double ratio;
	long audio_bytes;

	//WaitForSingleObject(thisvid.hIOMutex, INFINITE);

	/*
     * compute the desired 
     * frame number
     *
     */

    frame = (long) (percent * thisvid.video_frames / 100);

    /*
     * and go to the next 
     * keyframe.
     *
     */
  
    while(!AVIDecaps_isKeyframe(frame)) {
      frame++;
    }

    /*
     * now set video 
     * position.
     *
     */
    
	//AVIDecaps_VideoSeek(frame);
	thisvid.video_pos = frame;
    /*
     * calculate what ratio 
     * it corresponds to
     *
     */
    if(thisvid.audio_strn > 0) {
      
      ratio = (double) ((double) frame / (double) thisvid.video_frames);
      
      /*
       * and set audio 
       * position
       *
       */
      audio_bytes  = (long) (ratio * thisvid.audio_bytes);
      audio_bytes  += audio_bytes % 4;

      AVIDecaps_AudioSeek(audio_bytes);

      //ReleaseMutex(thisvid.hIOMutex);

      return thisvid.audio_bytes-audio_bytes;
	}

	return 1;
}

int AVIDecaps_ReSeekAudio()
{
	double ratio;
	long audio_bytes;

    if(thisvid.audio_strn > 0) {

	  //WaitForSingleObject(thisvid.hIOMutex, INFINITE);

      ratio = (double) ((double) thisvid.video_pos / (double) thisvid.video_frames);
      
      audio_bytes  = (long) (ratio * thisvid.audio_bytes);
      audio_bytes  += audio_bytes % 4;

      AVIDecaps_AudioSeek(audio_bytes);

      //ReleaseMutex(thisvid.hIOMutex);

	}

    return thisvid.audio_bytes-audio_bytes;
}

double AVIDecaps_GetProgress()
{
	return (double) ((double)(thisvid.video_pos))*100.0/((double)thisvid.video_frames);
}


int AVIDecaps_Close()
{
	printf("Close\r\n");
	InputMediaClose();
	if (thisvid.video_index) realloc(thisvid.video_index,0);
	//printf("index2\r\n");
	if (thisvid.audio_index) realloc(thisvid.audio_index,0);
	//printf("index3\r\n");
	if (thisvid.idx) free(thisvid.idx);
	thisvid.video_index=0;
	thisvid.audio_index=0;
	thisvid.idx=0;
	printf("Closed reader\r\n");
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



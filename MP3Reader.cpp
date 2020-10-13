// MP3Reader.cpp : Defines the entry point for the DLL application.
//

#include "kos.h"
#include "AVIDecaps.h"
#include "inputmedia.h"

videoinfo mpsvid;


/*
 * Reads the first 12 bytes of
 * the file, and returns TRUE
 * if the file is an AVI.
 *
 */

int MP3Reader_IsAVI()
{
	
	return 0;
}

/*
 * Returns the sample size of
 * the first audio stream...
 *
 */

int MP3Reader_SampleSize()
{
	
	return 0;
}


/*
 * Fill the class with info from headers
 * and reconstruct an index if wanted.
 *
 */

int MP3Reader_FillHeader(int getIndex)
{
	InputMediaSeek(0,INPUT_SEEK_SET);
	mpsvid.width=0;
	mpsvid.height=0;
	mpsvid.video_frames=0;
	mpsvid.video_pos = 0;
	strcpy(mpsvid.compressor,"divx");
	mpsvid.a_fmt=85;
//    mpsvid.a_rate=44100;             
//	mpsvid.a_chans=2;           
 //   mpsvid.a_bits=16;            
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

int MP3Reader_Open(char* lpFilename, int type, int maxsize,videoinfo** vid)
{

	mpsvid.audio_bytes=InputMediaOpen(lpFilename, 0, type,1000000, 0);
	if (!mpsvid.audio_bytes) 
	{
		return 0;
	}

	mpsvid.video_pos  = 0;
	mpsvid.audio_posc = 0;
	mpsvid.audio_posb = 0;

	mpsvid.idx         = NULL;
	mpsvid.video_index = NULL;
	mpsvid.audio_index = NULL;
	*vid=&::mpsvid;
	if(MP3Reader_FillHeader(1)) {
	
		return 1;
	}
	return 0;
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

int MP3Reader_NextVideoFrame(char *buffer, int drop)
{
   return 0;
}

/*
 * Reads any amount of audio
 * data. FIXME : should return
 * the actual number read.
 */

int MP3Reader_ReadAudio(char *audbuf, int bytes)
{
	int nr;


   nr = 0; 



   nr=InputMediaRead(audbuf, bytes);



   return nr;
}

/*
 * Return the actual framerate
 * FIXME : should be a double...
 *
 */

double MP3Reader_FrameRate()
{

	return (double) mpsvid.fps;
}


/*
 * Seek to a particular 
 * video frame.
 */

int MP3Reader_VideoSeek(long frame)
{
   return 1;
}

int MP3Reader_AudioSeek(long bytes)
{
	InputMediaSeek(bytes,INPUT_SEEK_SET);
   return 0;
}

bool MP3Reader_isKeyframe(long frame)
{
      return 1; 
}

int MP3Reader_NextKeyFrame()
{
	return 0;
}

int	MP3Reader_PreviousKeyFrame()
{
	return 1;
}


int MP3Reader_Seek(int percent)
{
	  int64 audio_bytes;


      audio_bytes  = (int64) (percent * mpsvid.audio_bytes)/100;
      audio_bytes  += audio_bytes % 4;

      MP3Reader_AudioSeek(audio_bytes);


      return mpsvid.audio_bytes-audio_bytes;

}

int MP3Reader_ReSeekAudio()
{

    return mpsvid.audio_bytes;
}

double MP3Reader_GetProgress()
{
	
	return (double) ((double)(InputMediaSeek(0,INPUT_SEEK_CUR)))*100.0/((double)mpsvid.audio_bytes);
}


int MP3Reader_Close()
{
	InputMediaClose();
	return 1;
}

void InitializeReaderMP3(reader* rd)
{
	rd->READER_Open=MP3Reader_Open;
	rd->READER_ReadAudio=MP3Reader_ReadAudio;
	rd->READER_NextVideoFrame=MP3Reader_NextVideoFrame;
	rd->READER_Seek=MP3Reader_Seek;
	rd->READER_ReSeekAudio=MP3Reader_ReSeekAudio;
	rd->READER_GetProgress=MP3Reader_GetProgress;
	rd->READER_Close=MP3Reader_Close;

}



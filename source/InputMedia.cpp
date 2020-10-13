 /*This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The GPL can be found at: http://www.gnu.org/copyleft/gpl.html						*
 *																						*
 *	Authors:
 *			Marc Dukette
 **************************************************************************************/
#include "kos.h"
#include "InputMedia.h"


/*
 * Seek Enum
 */

enum {

	INPUT_SEEK_SET,
	INPUT_SEEK_CUR,
	INPUT_SEEK_END
};

//	DWORD mode;

	file_t file;
//	int filesize;
	/*
 	 * HTTP/FTP stuff 
	 */


//	HANDLE    ioMutex;
	long     lastReadPos;

	int     currentBuffer;
int maxread=100000;
int fSize;
#define CACHE_SIZE 10000000
//int Done=0;
char*			  ReadAheadBuff=0;
int BufferStart;
int BufferStartNext;
int CurrentBuffer;
int nCacheSize=1000000;
int WritePos;
/*
 * The main class
 */
void Restart(int pos)
{

	int n;
//	WaitForSingleObject(eventReadDone,INFINITE);
//	SetFilePointer(::file, pos,NULL, FILE_BEGIN);
	printf("restart read pos=%d WritePos=%d\r\n",pos,WritePos);	
	fs_seek(::file,pos,SEEK_SET);
	BufferStart=pos;
	BufferStartNext=pos;
	WritePos=0;
	maxread=200000;
	while(WritePos<CACHE_SIZE-1100000&&(WritePos+BufferStartNext<fSize)) ReadAhead();
	maxread=30000;
}

void ReadAhead()
{
	int size;
	char t[100];
	if (fSize&&(BufferStartNext+WritePos<fSize))
	{
		int n;
		int readamt;
		if (!file) 
		{
			return;
		}
		if (lastReadPos-(WritePos+BufferStart)<1000000&&(lastReadPos>(WritePos+BufferStart))&&(BufferStartNext>BufferStart))
		{
			return;
		}
		if ((WritePos-(lastReadPos-BufferStart))>CACHE_SIZE-1000000)
		{
			return;
		}
		//printf("buffer=%d\r\n",(WritePos-(lastReadPos-BufferStart)));
		readamt=maxread;
		if (CACHE_SIZE-(WritePos+maxread)<0)
		{
			readamt=CACHE_SIZE-WritePos;
		}
		size=fs_read(file,(void *) (ReadAheadBuff+WritePos),readamt);
		WritePos+=size;
		//printf("Write=%d Size=%d\r\n",WritePos,size);
		if (size<readamt)
		{
			return;
		}
		if (WritePos==CACHE_SIZE)
		{
			WritePos=0;
			BufferStartNext=BufferStart+CACHE_SIZE;
		}
	}
	return;
}

/*
 * The main class
 */

int InputMediaOpen(char* lpFilename, int mode, int type, int reservesize, int maxsize) 
{
	if(lpFilename) {
		int n;
		char temp[100];
//		::file       = NULL;
//		::mode       = -1;
		lastReadPos=0;
//		Done=1;
//		ReadAheadBuff=0;

		::file =fs_open((char*)lpFilename,O_RDONLY);
		//::mode = INPUT_TYPE_FILE;

		fSize=fs_total(::file); //GetFileSize(::file,NULL);
		if (!fSize ) return 0;
		if (!ReadAheadBuff) 
			ReadAheadBuff=(char*)malloc(CACHE_SIZE);
		BufferStart=0;
		BufferStartNext=0;
		WritePos=0;
		maxread=200000;
		if (maxsize)
		{
			//
			while(WritePos<CACHE_SIZE-1100000&&(WritePos<fSize)) ReadAhead();
		}
		else
		{
			//CACHE_SIZE-1100000
			while(WritePos<400000&&(WritePos<fSize)) ReadAhead();
		}
		maxread=30000;


	}
	return fSize;
}

void InputMediaClose() 
{
	int n;
	if(fSize) {
		
		fs_close(::file);
	}
	//free(ReadAheadBuff);
	//::file=NULL;
	fSize=0;

}


int InputMediaRead(char *data, unsigned int size)
{
	int bytesleft;
	if (size+lastReadPos>fSize)
		size=fSize-lastReadPos;
	bytesleft=size;


	while(bytesleft&&(lastReadPos<fSize))
	{
		if ((lastReadPos+bytesleft)>(WritePos+BufferStartNext+20000))
		{
			//printf("BufferStart=%d BufferNext=%d WriteBos=%d \r\n",BufferStart, BufferStartNext,WritePos);
			Restart(lastReadPos-100000);
		}

		while((lastReadPos+bytesleft)>(WritePos+BufferStartNext))
		{
		//	printf("caught up\r\n");
			ReadAhead();
		}
		
		if (lastReadPos+bytesleft>(BufferStart+CACHE_SIZE))
		{
			if (lastReadPos<(BufferStart+CACHE_SIZE))
			{
				memcpy((void*)data,(void*)((char*)ReadAheadBuff+(lastReadPos-BufferStart)),(BufferStart+CACHE_SIZE)-lastReadPos);
				bytesleft-=((BufferStart+CACHE_SIZE)-lastReadPos);
				data+=((BufferStart+CACHE_SIZE)-lastReadPos);
				::lastReadPos=BufferStart+CACHE_SIZE; //+=((BufferStart+CACHE_SIZE)-lastReadPos);
			}
			//printf("cycle=%d\r\n",lastReadPos);
			BufferStart=BufferStartNext;

		}
		else if (lastReadPos<BufferStart-CACHE_SIZE+WritePos)
		{
			//printf("BufferStart=%d BufferNext=%d WriteBos=%d \r\n",BufferStart, BufferStartNext,WritePos);
			Restart(lastReadPos-100000);
		}
		else if (lastReadPos<BufferStart)
		{
			if (lastReadPos+bytesleft>(BufferStart))
			{
				memcpy((void*)data,(void*)((char*)ReadAheadBuff+(lastReadPos-(BufferStart-CACHE_SIZE))),BufferStart-lastReadPos);
				bytesleft-=(BufferStart-lastReadPos);
				data+=(BufferStart-lastReadPos);
				::lastReadPos=BufferStart; //+=(BufferStart-lastReadPos);
			}
			else
			{
				memcpy((void*)data,(void*)((char*)ReadAheadBuff+(lastReadPos-(BufferStart-CACHE_SIZE))),bytesleft);
				::lastReadPos+=bytesleft;

				bytesleft=0;
			}
		}
		else
		{
			memcpy((void*)data,(void*)((char*)ReadAheadBuff+(lastReadPos-BufferStart)),bytesleft);
			::lastReadPos+=bytesleft;

			bytesleft=0;
		}

	}
	size-=bytesleft;
	return size;
}


int InputMediaSeek(int size, unsigned int method)
{
	switch(method)
	{
	case INPUT_SEEK_SET:
		::lastReadPos = size;
//		if (size<BufferStart-400000||(size<lastReadPos-500000))

		if (size-BufferStart<WritePos+100000&& (BufferStartNext!=BufferStart))
		{
			printf("BufferStart=%d BufferNext=%d WriteBos=%d \r\n",BufferStart, BufferStartNext,WritePos);
			Restart(size);

		}
		return 0;
		break;

	
	case INPUT_SEEK_CUR:

		if(!size) {
			return ::lastReadPos;
		}
		else 
		{
			int holdpos=lastReadPos;
			::lastReadPos += size;

			if (lastReadPos>(BufferStartNext+CACHE_SIZE))
			{
				//printf("BufferStart=%d BufferNext=%d WriteBos=%d respos=%d\r\n",BufferStart, BufferStartNext,WritePos,holdpos);
				Restart(lastReadPos-100000);
			}

			return 0;
		}
		break;

	}
	return 0;
}



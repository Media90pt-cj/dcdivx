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
 * Authors:																			  *
 *          Marc Dukette                                                              *
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
spinlock_t mutex;
/*
 * The main class
 */
void Restart(int pos)
{

	int n;
//	WaitForSingleObject(eventReadDone,INFINITE);
//	SetFilePointer(::file, pos,NULL, FILE_BEGIN);
	printf("restart read pos=%d WritePos=%d\r\n",pos,WritePos);	
	spinlock_lock(&mutex);
	fs_seek(::file,pos,SEEK_SET);
	BufferStart=pos;
	BufferStartNext=pos;
	WritePos=0;
	maxread=200000;
//	while(WritePos<CACHE_SIZE-600000&&(WritePos+BufferStartNext<fSize)) ReadAhead();
	spinlock_unlock(&mutex);
	while(WritePos<CACHE_SIZE-600000&&(WritePos<fSize)) thd_pass();
	maxread=100000;
//	ReleaseMutex(eventReadDone);	
}

void ReadAhead(void*)
{
	int size;
	char t[100];
//	Done=0;
	while (fSize&&(BufferStartNext+WritePos<fSize))
	{
		int n;
		int readamt;
//		WaitForSingleObject(eventReadDone,INFINITE);
		if (!file) 
		{
			return;
		}
		spinlock_lock(&mutex);
		if (lastReadPos-(WritePos+BufferStart)<500000&&(lastReadPos>(WritePos+BufferStart))&&(BufferStartNext>BufferStart))
		{
			spinlock_unlock(&mutex);
			thd_pass();
			continue;
		}
		if ((WritePos-(lastReadPos-BufferStart))>CACHE_SIZE-500000)
		{
			spinlock_unlock(&mutex);
			thd_pass();
			continue;
		}

		readamt=maxread;
		if (CACHE_SIZE-(WritePos+maxread)<0)
		{
			readamt=CACHE_SIZE-WritePos;
		}
		size=fs_read(file,(void *) (ReadAheadBuff+WritePos),readamt);
		WritePos+=size;
		if (size<readamt)
		{
			//ReleaseMutex(eventReadDone);	
			spinlock_unlock(&mutex);
			continue;
		}
		if (WritePos==CACHE_SIZE)
		{
			WritePos=0;
			BufferStartNext=BufferStart+CACHE_SIZE;
		}
		spinlock_unlock(&mutex);
//		ReleaseMutex(eventReadDone);	
		//Sleep(1);
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
		spinlock_init(&mutex);
//		Done=0;
//		eventReadDone=CreateMutex(NULL,false,_T("ReadDone"));
//		rhandle=CreateThread(NULL,NULL,ReadAhead,(LPVOID)0,0,&rtid);
		maxread=200000;
//		while(WritePos<CACHE_SIZE-600000&&(WritePos<fSize)) ReadAhead();
		thd_create(ReadAhead,NULL);
		while(WritePos<CACHE_SIZE-600000&&(WritePos<fSize)) thd_pass();
		maxread=100000;


	}
	return fSize;
}

void InputMediaClose() 
{
	int n;
	//TerminateThread(rhandle,0);
	if(fSize) {
		
		fs_close(::file);
	}
	//free(ReadAheadBuff);
	//::file=NULL;
	fSize=0;
	//while(!Done) Sleep(100);
	//CloseHandle(eventReadDone);

}


int InputMediaRead(char *data, unsigned int size)
{
	int bytesleft;
	if (size+lastReadPos>fSize)
		size=fSize-lastReadPos;
	bytesleft=size;


	while(bytesleft&&(lastReadPos<fSize))
	{
		spinlock_lock(&mutex);
		if ((lastReadPos+bytesleft)>(WritePos+BufferStartNext+20000))
		{
			spinlock_unlock(&mutex);
			Restart(lastReadPos-100000);
			spinlock_lock(&mutex);
		}

		while((lastReadPos+bytesleft)>(WritePos+BufferStartNext))
		{
			//ReadAhead();
			spinlock_unlock(&mutex);
			thd_pass();
			spinlock_lock(&mutex);
		}
		
		if (lastReadPos+bytesleft>(BufferStart+CACHE_SIZE))
		{
			if (lastReadPos<(BufferStart+CACHE_SIZE))
			{
				memcpy((void*)data,(void*)((char*)ReadAheadBuff+(lastReadPos-BufferStart)),(BufferStart+CACHE_SIZE)-lastReadPos);
				bytesleft-=((BufferStart+CACHE_SIZE)-lastReadPos);
				::lastReadPos+=((BufferStart+CACHE_SIZE)-lastReadPos);
				data+=((BufferStart+CACHE_SIZE)-lastReadPos);
			}
			BufferStart=BufferStartNext;

		}
		else if (lastReadPos<BufferStart)
		{
			if (lastReadPos+bytesleft>(BufferStart+CACHE_SIZE))
			{
				memcpy((void*)data,(void*)((char*)ReadAheadBuff+(lastReadPos-(BufferStart-CACHE_SIZE))),BufferStart-lastReadPos);
				bytesleft-=(BufferStart-lastReadPos);
				::lastReadPos+=(BufferStart-lastReadPos);
				data+=(BufferStart-lastReadPos);
			}
			else
			{
				memcpy((void*)data,(void*)((char*)ReadAheadBuff+(lastReadPos-(BufferStart-CACHE_SIZE))),bytesleft);
				::lastReadPos+=bytesleft;

				bytesleft=0;
			}
		}
		else if (lastReadPos+bytesleft>BufferStart+CACHE_SIZE)
		{
			bytesleft-=CACHE_SIZE-(lastReadPos-BufferStart);
		}
		else
		{
			memcpy((void*)data,(void*)((char*)ReadAheadBuff+(lastReadPos-BufferStart)),bytesleft);
			::lastReadPos+=bytesleft;

			bytesleft=0;
		}
		spinlock_unlock(&mutex);

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
		spinlock_lock(&mutex);

		if (size-BufferStart<WritePos+100000&& (BufferStartNext!=BufferStart))
		{
			spinlock_unlock(&mutex);
			Restart(size);
			spinlock_lock(&mutex);

		}
		spinlock_unlock(&mutex);
		return 0;
		break;

	
	case INPUT_SEEK_CUR:

		if(!size) {
			return ::lastReadPos;
		}
		else 
		{
			spinlock_lock(&mutex);
			::lastReadPos += size;

			if (lastReadPos>(BufferStartNext+CACHE_SIZE))
			{
				spinlock_unlock(&mutex);
				Restart(lastReadPos-100000);
				spinlock_lock(&mutex);
			}

			spinlock_unlock(&mutex);
			return 0;
		}
		break;

	}
	return 0;
}



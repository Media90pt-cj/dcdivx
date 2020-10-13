
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the OGGLIB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OGGLIB_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef OGGLIB_EXPORTS
#define OGGLIB_API __declspec(dllexport)
#else
#define OGGLIB_API __declspec(dllimport)
#endif

// This class is exported from the OGGLib.dll
class OGGLIB_API COGGLib {
public:
	COGGLib(void);
	// TODO: add your methods here.
};

OGGLIB_API void Init(int,char*);
OGGLIB_API void Exit(void);

OGGLIB_API int DecompressMp3(char * inbuff, int insize, char *outmemory, int outmemsize, int *done, int* inputpos);
OGGLIB_API int GetHeaderInfo(unsigned char * inbuff, int insize, int* Freq, int* ch, int* br);

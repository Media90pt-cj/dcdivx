// VP3_PPC.cpp : Defines the entry point for the DLL application.
//

#include "VP3_PPC.h"
#define PBDLL

#include "vp3d.h"
#include "vfw_pb_interface.h"
#include "pbdll.h"
/**/
#include "decore.h"
int x_size;
int y_size;
PB_INSTANCE* myPBI;

int render;
extern unsigned char *decore_stream;
//extern int Dither;
int decore_init(int hor_size, int ver_size, unsigned long color_depth, int output_format);
int decore_release();
//void decore_frame(unsigned char *stream, int length, unsigned char *bmp, int render_flag, int Extra);
//int decore_setoutput(unsigned long color_depth, int output_format);
void VPInitLibrary(void);
void VPDeInitLibrary(void);

/**/

static int flag_firstpicture = 1;

/**/
int decore_vp3(unsigned long handle, unsigned long dec_opt,
	void *param1, void *param2)
{
	switch (dec_opt)
	{
		case DEC_OPT_INIT:
			{DEC_PARAM *dec_param = (DEC_PARAM *) param1;
 			x_size = dec_param->x_dim;
 			y_size = dec_param->y_dim;
			VPInitLibrary();
		    StartDecoder( &myPBI, x_size, y_size );
			return DEC_OK;
			}
			break; 
		case DEC_OPT_RELEASE:
			StopDecoder(&myPBI);
			VPDeInitLibrary();
			return DEC_OK;
			break;
		default:
			return DEC_OK;
			break;
	}
}

/**/


int decore_frame_vp3(unsigned char *stream, int length, unsigned char *bmp, int render_flag)
{
	AVPicture * p=(AVPicture *) bmp;
	int retVal;
//	GetPbParam( myPBI, 0, &PostProcess);
//	SetPbParam( myPBI,0,1);
	render=render_flag;
    retVal= DecodeFrameToYUV(myPBI,
        (char *)stream, length, x_size, y_size);
    if(retVal != 0 )
    {
        return 0;
    }
#ifdef MIPS
	p->data[0] = &myPBI->LastFrameRecon[myPBI->ReconYDataOffset];
	p->data[1] = &myPBI->LastFrameRecon[myPBI->ReconUDataOffset];
	p->data[2] = &myPBI->LastFrameRecon[myPBI->ReconVDataOffset];
	p->linesize[0]=myPBI->Configuration.YStride;
	p->linesize[1]=myPBI->Configuration.UVStride;
	p->linesize[2]=myPBI->Configuration.UVStride;
#else
	p->data[0] = &myPBI->LastFrameRecon[myPBI->ReconYDataOffset+(y_size-1)*myPBI->Configuration.YStride];
	p->data[1] = &myPBI->LastFrameRecon[myPBI->ReconUDataOffset+(y_size/2-1)*myPBI->Configuration.UVStride];
	p->data[2] = &myPBI->LastFrameRecon[myPBI->ReconVDataOffset+(y_size/2-1)*myPBI->Configuration.UVStride];
	p->linesize[0]=myPBI->Configuration.YStride*-1;
	p->linesize[1]=myPBI->Configuration.UVStride*-1;
	p->linesize[2]=myPBI->Configuration.UVStride*-1;
#ifdef ARM
	p->last_picture[0]=p->data[0]; //&myPBI->ThisFrameRecon[myPBI->ReconYDataOffset+(y_size-1)*myPBI->Configuration.YStride];
	p->last_picture[1]=p->data[1];//&myPBI->ThisFrameRecon[myPBI->ReconUDataOffset+(y_size/2-1)*myPBI->Configuration.UVStride];
	p->last_picture[2]=p->data[2];//&myPBI->ThisFrameRecon[myPBI->ReconVDataOffset+(y_size/2-1)*myPBI->Configuration.UVStride];
#endif
#endif
	return 1;
}





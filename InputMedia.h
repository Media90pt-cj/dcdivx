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
//#include "winbase.h"
#ifdef __cplusplus
extern "C" {
#endif 

#ifndef INPUTMEDIA_H
#define INPUTMEDIA_H

/*
 * includes
 */
#include <malloc.h>
#include <stdlib.h>


int InputMediaRead(char *data, unsigned int size);
int InputMediaSeek(int   size, unsigned int method);
void ReadAhead();

	int InputMediaOpen(char* lpFilename, int mode, int type, int cachesize,int maxsize) ;
	void InputMediaClose();
#endif
#ifdef __cplusplus
}
#endif 

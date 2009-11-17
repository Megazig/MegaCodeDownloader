#ifndef MAIN_H
#define MAIN_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <network.h>
#include <stdint.h>
#include <string>

extern "C" {
#include <elm.h>
#include <usync.h>
}

#include "common.h"
#include "netconsole.h"
#include "hexdump.h"

/*
#include "FreeTypeGX.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "input.h"
#include "filelist.h"
#include "demo.h"
*/

#define		DEBUG			0

#define		ENETINIT		-2
#define		EELMMOUNT		-3

#define		MAX_GAME_COUNT	250
#define		MAX_NAME_LENGTH	70

using std::bad_alloc;
using std::string;

int stuff( unsigned int * output ); 

struct httpresponse{
	float  version;
	int    response_code;
	char * text;
	char * date;
	char * modified;
	char * server;
	//size_t content_length;
	int    content_length;
	char * content_type;
	char * charset;
};

#endif


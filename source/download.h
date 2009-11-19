#ifndef DOWNLOAD_H
#define DOWNLOAD_H

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
#include <sys/dir.h>

extern "C" {
#include <elm.h>
#include <usync.h>
}

#include "common.h"
#include "netconsole.h"
#include "hexdump.h"

#define		DEBUG			0

#define		ENETINIT		-2
#define		EELMMOUNT		-3

#define		MAX_GAME_COUNT	250
#define		MAX_NAME_LENGTH	70

using std::bad_alloc;
using std::string;

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

struct game_info{
	char type;
	char region;
	int  selection;
	char * name;
};

enum
{
	TYPE_WII = 1,
	TYPE_WIIWARE,
	TYPE_VC,
	TYPE_WII_CHANNELS,
	TYPE_NES,
	TYPE_N64,
	TYPE_SMS,
	TYPE_GENESIS,
	TYPE_NEOGEO,
	TYPE_COMMODORE,
	TYPE_MSX,
	TYPE_TGFX16,
	TYPE_TGFXCD
};

enum
{
	REGION_NTSCU = 1,
	REGION_PAL,
	REGION_NTSCJ,
	REGION_OTHER
};

char * GetGameName( char * body );
char * GetGameLink( char * body , char regionLetter );
char GetGameTypeChar( int category );
char GetGameRegionChar( int region );
int DownloadCodes( int game , int type , char * GameList );
int GetGameList( int category , int region , char * GameList );

#endif

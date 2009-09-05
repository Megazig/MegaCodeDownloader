#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <network.h>
#include <string.h>
#include <fat.h>

#define		EFATINIT		-1
#define		ENETINIT		-2

#define		MAX_GAME_COUNT	250
#define		MAX_NAME_LENGTH	70

// Colors
#define		BLACK_FG		30
#define		BLACK_BG		40
#define		RED_FG			31
#define		RED_BG			41
#define		GREEN_FG		32
#define		GREEN_BG		42
#define		YELLOW_FG		33
#define		YELLOW_BG		43
#define		BLUE_FG			34
#define		BLUE_BG			44
#define		MAGENTA_FG		35
#define		MAGENTA_BG		45
#define		CYAN_FG			36
#define		CYAN_BG			46
#define		WHITE_FG		37
#define		WHILTE_BG		47

// Text Styles
#define		RESET_TEXT		0
#define		BRIGHT_TEXT		1
#define		DIM_TEXT		2
#define		UNDERSCORE_TEXT	3
#define		BLINKING_TEXT	4
#define		REVERSE_TEXT	5
#define		HIDDEN_TEXT		6

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

#include <stdint.h>
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

//--------------------------------------------------------------------------------
void SetTextInfo( int color_fg , int color_bg , int attribute ) {
//--------------------------------------------------------------------------------
//
//	Params:		color_fg			Foreground Color For Text
//				color_bg			Background Color For Text
//				attribute			Text Style
//	Returns:	None
//

	printf( "\x1b[%dm" , attribute );
	printf( "\x1b[%dm" , color_fg );
	printf( "\x1b[%dm" , color_bg );
}

//--------------------------------------------------------------------------------
void ClearText( ) {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Returns:	None
//

	printf( "\x1b[2J" );
}

//--------------------------------------------------------------------------------
void PrintPositioned( int y , int x , const char * text ) {
//--------------------------------------------------------------------------------
//
//	Params:		y			Y Position for Text
//				x			X Position for Text
//				string		Strig to Print
//	Returns:	None
//

	printf("\x1b[%d;%dH", y , x );
	printf("%s" , text );
}

//--------------------------------------------------------------------------------
void ExitToLoader( int return_val ) {
//--------------------------------------------------------------------------------
//
//	Params:		return_val	Value to Pass to Exit
//	Return:		None		exits
//

	printf("\x1b[25;5HThanks for using MegaDownloader\n");
	exit( return_val );
}

//--------------------------------------------------------------------------------
void WaitForButtonPress() {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Return:		None
//

	while(1) {

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application when home is pressed
		if ( pressed & WPAD_BUTTON_HOME ) ExitToLoader(0);
		// We break the waiting loop with any other buttons
		if ( pressed & WPAD_BUTTON_A ) break;
		if ( pressed & WPAD_BUTTON_B ) break;
		if ( pressed & WPAD_BUTTON_1 ) break;
		if ( pressed & WPAD_BUTTON_2 ) break;
		if ( pressed & WPAD_BUTTON_PLUS ) break;
		if ( pressed & WPAD_BUTTON_MINUS ) break;
		if ( pressed & WPAD_BUTTON_UP ) break;
		if ( pressed & WPAD_BUTTON_DOWN ) break;
		if ( pressed & WPAD_BUTTON_LEFT ) break;
		if ( pressed & WPAD_BUTTON_RIGHT ) break;

		// Add check for Reset to return to loader

		// Wait for the next frame
		VIDEO_WaitVSync();
	}
}

//--------------------------------------------------------------------------------
void ShowProgramInfo() {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Returns:	None
//

	printf("\x1b[2;0H");
	printf("MegaCodeDownloader\n");
	printf("coded by\n");
	SetTextInfo( BLINKING_TEXT , GREEN_FG , BLACK_BG );
	printf("megazig\n");
	SetTextInfo( RESET_TEXT , WHITE_FG , BLACK_BG );
}


//--------------------------------------------------------------------------------
void Init() {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Returns:	None
//

	// Initialise the video system
	VIDEO_Init();
	
	// This function initialises the attached controllers
	WPAD_Init();
	
	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	
	// Initialise the console, required for printf
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);
	
	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);
	
	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	printf("\x1b[2J");
	printf("\x1b[2;0H");
	ShowProgramInfo();

	PrintPositioned( 15 , 0 , "Initializing FAT File System..........." );
	if ( !fatInitDefault() ) {
		PrintPositioned( 15 , 45 , "fatInitDefault failed :( \n" );
		WaitForButtonPress();
		ExitToLoader( EFATINIT );
	}
	PrintPositioned( 15 , 45 , "COMPLETE\n" );

	PrintPositioned( 16 , 0 , "Initializing Network..................." );
	char * myIpAddy = (char*)malloc(16 * sizeof(char));
	if (if_config(myIpAddy, NULL, NULL, true)){
		PrintPositioned( 16 , 45 , "Failed to initialize network :( \n" );
		WaitForButtonPress();
		ExitToLoader( ENETINIT );
	}
	PrintPositioned( 16 , 45 , "COMPLETE\n" );

	//printf("IP Address: %s\n", myIpAddy);

	free(myIpAddy);
}

//--------------------------------------------------------------------------------
int GetStringLengthTerminated( char * text , char terminator ) {
//--------------------------------------------------------------------------------
//
//	Params:		text		String to get length of
//				terminator	Character that ends the string
//	Returns:	None
//

	char * end = strchr( text , terminator );
	int length = end - text;
	return length;
}

//--------------------------------------------------------------------------------
void PrintCharTerminated( char * text , char terminator ) {
//--------------------------------------------------------------------------------
//
//	Params:		text		String to print
//				terminator	Character that ends the string
//	Returns:	None
//

	char * end = strchr( text , terminator );
	int length = end - text;
	int i;
	for ( i = 0 ; i < length ; i++ )
		printf("%.1s", text + i );
	printf("\n");
}

//--------------------------------------------------------------------------------
void PrintResponse( struct httpresponse response ) {
//--------------------------------------------------------------------------------
//
//	Params:		response	HTTP Response
//	Returns:	None
//

	printf("\tHTTP/%1.1f %d %s\n", response.version, response.response_code, response.text);
	printf("\t%d bytes long\tModified: %s\n", response.content_length, response.modified);
	printf("\tContent Type: %s; charset %s\n\n", response.content_type, response.charset);
}

//--------------------------------------------------------------------------------
char * GetGameName( char * body ) {
//--------------------------------------------------------------------------------
//
//	Params:		body		Webpage data as char*
//	Returns:	name		Name of Games as char*
//

	// sets body to the name immediately following the >
	return strchr( body , '>' ) + 1;
}

//--------------------------------------------------------------------------------
char * GetGameLink( char * body , char regionLetter ) {
//--------------------------------------------------------------------------------
//
//	Params:		body		Webpage data as char*
//				region		Region as char
//	Returns:	link		Link of Games as char*
//


	char div[12] = { 'd','i','v',' ','c','l','a','s','s','=','@','\0' };
	div[10] = regionLetter;
	while( strncmp( body+1 , div , 11 ) != 0 ) {
		body = strchr( body+1 , '<' );
		if ( ( body == NULL ) || ( !strncmp( body+1 , "/HTML" , 5 ) ) )
			return NULL;
	}
	while( strncmp( body+1 , "a href=\"./index.php?c=" , 19 ) != 0 ) {
		body = strchr( body+1 , '<' );
		if ( ( body == NULL ) || ( !strncmp( body+1 , "/HTML" , 5 ) ) )
			return NULL;
	}
	body = body + 23;	// places body at GAMEID
	return body;
}

//--------------------------------------------------------------------------------
char GetGameTypeChar( int category ) {
//--------------------------------------------------------------------------------
//
//	Params:		category	Game Category as Integer
//	Returns:	type		Game Type as Char
//

	char chidLetter;
	switch ( category ){
		case 1:					// Wii
			chidLetter = 'R';
			break;
		case 2:					// WiiWare
			chidLetter = 'W';
			break;
		case 3:					// VC Arcade
			chidLetter = 'D';
			break;
		case 4:					// Wii Channels
			chidLetter = 'H';
			break;
		case 5:					// NES / Famicom
			chidLetter = 'F';
			break;
		case 6:					// N64
			chidLetter = 'N';
			break;
		case 7:					// Sega Master System
			chidLetter = 'L';
			break;
		case 8:					// Genesis / MegaDrive
			chidLetter = 'M';
			break;
		case 9:					// NeoGeo
			chidLetter = 'E';
			break;
		case 10:				// Commodore
			chidLetter = 'C';
			break;
		case 11:				// MSX
			chidLetter = 'X';
			break;
		case 12:				// TurboGraFx-16
			chidLetter = 'P';
			break;
		case 13:				// TurboGraFx-CD
			chidLetter = 'Q';
			break;
		default:
			chidLetter = 'R';
			break;
	}

	return chidLetter;

}

//--------------------------------------------------------------------------------
char GetGameRegionChar( int region ) {
//--------------------------------------------------------------------------------
//
//	Params:		region	Game Region as Integer
//	Returns:	reg		Game Region as Char
//

	char regionLetter;
	switch ( region ) {
		case 1:					// NTSC-U
			regionLetter = 'E';
			break;
		case 2:					// PAL
			regionLetter = 'P';
			break;
		case 3:					// NTSC-J
			regionLetter = 'J';
			break;
		case 4:					// other small regions
			regionLetter = 'z';
			break;
		default:
			regionLetter = 'E';
			break;
	}

	return regionLetter;
}

//--------------------------------------------------------------------------------
int DownloadCodes( int game , int type , char * GameList ) {
//--------------------------------------------------------------------------------
//
//	Params:		game		Game to Download Codes For		as Integer
//				type		Game Type of game				as Integer
//				GameList	Game List to return for Showing
//	Returns:	0 for success , negative for failure		as Integer
//

	PrintPositioned( 26 , 10 , "Downloading Codes\n");

	char * codes = (char*)malloc( 0xFFFFF * sizeof(char) );
	//memset( codes , 0 , 0xFFFFF );
	
	char chid = GetGameTypeChar( type );

	struct httpresponse response;
	char * url      = (char*)malloc( 40 * sizeof(char) );
	strncpy( url , "geckocodes.org" , 16 );
	char * link = (char*)malloc( 30 * sizeof(char) );
	sprintf( link , "/codes/%c/%s.txt" , chid , &GameList[ game * 2 * MAX_NAME_LENGTH ] );
	char * filepath = link;
	char * hostname = url;
	ClearText();
	PrintPositioned( 2 , 0 , "" );
	//printf( "hostname is : %s and filepath is : %s\n" , hostname , filepath );

	struct hostent *host = net_gethostbyname(hostname);				// gets host info
	struct sockaddr_in server;
	int socket = net_socket( AF_INET , SOCK_STREAM , IPPROTO_IP );	// creates socket
	if ( host == NULL ) {
		printf("host not found at %s :(\n", hostname);
		free(url);
		free(link);
		free(codes);
		WaitForButtonPress();
		return -1;
	}
	memset( &server , 0 , sizeof(server) );							// reset server var
	server.sin_family	= AF_INET;									// IPv4
	server.sin_port		= htons(80);								// Port 80
	memcpy( &server.sin_addr , host->h_addr_list[0] , host->h_length );
	if ( net_connect( socket , (struct sockaddr*)&server , sizeof(server) ) ) {
		printf("failed to connect :(\n");
		free(url);
		free(link);
		free(codes);
		WaitForButtonPress();
		return -1;
	} else {
		//printf("connected :)\n");
		//WaitForButtonPress();
	}

	char * getstring = (char*)malloc( strlen("GET HTTP/1.0\r\n\r\n") + strlen(filepath) );
	sprintf( getstring , "GET %s HTTP/1.0\r\n\r\n" , filepath );
	int len  = strlen( getstring );
	int sent = net_write( socket , getstring , len );
	free(getstring);
	if ( sent < len )
		printf("sent %d of %d bytes\n", sent, len);
	int bufferlen = 1025;
	char * buf = (char*)malloc( bufferlen );
	unsigned int received = 0;
	int read = 0;
	response.text = (char*)malloc( sizeof(char) * 32 );
	char * filename = (char*)malloc( 50 * sizeof(char) );
	strncpy( filename , "sd:/txtcodes/" , 14 );
	strncat( filename , &GameList[ ( game * 2 * MAX_NAME_LENGTH ) + MAX_NAME_LENGTH ] , GetStringLengthTerminated( &GameList[ ( game * 2 * MAX_NAME_LENGTH ) + MAX_NAME_LENGTH ] , '[' ) - 1 );
	strncat( filename , ".txt" , 5 );
	FILE * fp = fopen( filename , "w" );
	char * line = (char*)malloc( bufferlen );
	char * linebegin;
	char * lineend;
	char dataStarted = 0;
	int headerlength = 0;

	while( (read = net_read( socket , buf , bufferlen - 1 ) ) > 0 ) {
		buf[read] = '\0';		// NULL TERMINATE AMOUNT READ
		linebegin = buf;
		while( (lineend = strchr( linebegin , '\n' ) ) != NULL ) {
			memset( line , '\0' , 1025 );
			strncpy( line , linebegin , lineend - linebegin );
			if ( !dataStarted ) {
				if ( !strncmp( line , "HTTP/" , 5 ) ) {
					sscanf( line , "HTTP/%f %d %s\n" , &(response.version) , &(response.response_code) , response.text );
				} else if ( !strncmp( line , "Content-Length" , 14 ) ) {
					sscanf( line , "Content-Length: %d", &(response.content_length) );
				} else if ( !strncmp( line , "Content-Type" , 12 ) ) {
					char * space = strchr( line  , ' ' );
					char * sc    = strchr( space , ';' );
					char * eq    = strchr( sc    , '=' );
					response.content_type = strndup( ( space + 1 ) , sc - space - 1 );
					response.charset      = strndup( ( eq + 1 ) ,  lineend - eq - 1 );
				} else if ( !strncmp( line , "Last-Modified" , 13 ) ) {
					char * space = strchr( line , ' ' );
					response.modified = strndup( space , lineend - space );
				} else if ( !strcmp( line , "\r" ) ) {
					//printf("end of http header\n");
					dataStarted = 1;
					headerlength = lineend - buf + 1;
				}
			} else {
				fprintf( fp , "%s\n" , line );
				strncat( codes , line , 0xfff );
			}
			linebegin = lineend + 1;
		}
		received += read;
	}
	received -= headerlength;

	free(url);
	free(link);
	free(line);
	free(buf);

	//PrintResponse( response );
	free(response.text);
	free(response.charset);
	free(response.modified);
	free(response.content_type);

	fclose( fp );
	//if ( read == 0 )
	//	printf( "Reached EOF\n" );
	if ( read == -1 )
		printf( "Read Error\n" );
	net_close( socket );

	printf("Codes copied to file\n");
	WaitForButtonPress();

	free(codes);

	return 0;
}


//--------------------------------------------------------------------------------
int GetGameList( int category , int region , char * GameList ) {
//--------------------------------------------------------------------------------
//
//	Params:		category	Game Category as Integer
//				region		Game Region	  as Integer
//				GameList	Game List to return for Showing
//	Returns:	number of games success , negative for failure	as Integer
//

	PrintPositioned( 26 , 10 , "Retrieving Game List\n");

	char chidLetter , regionLetter;
	chidLetter		= GetGameTypeChar( category );
	regionLetter	= GetGameRegionChar( region );

	char * page = (char*)malloc( 0xFFFFF * sizeof(char*) );
	struct httpresponse response;
	char * url      = (char*)malloc( 40 * sizeof(char) );
	strncpy( url , "geckocodes.org/index.php?chid=" , 31 );
	char * chid     = (char*)malloc(  2 * sizeof(char) );
	sprintf( chid , "%c" , chidLetter );
	strcat( url , chid );
	free(chid);
	const char * re       = "&r=";
	strcat( url , re );
	char * reg      = (char*)malloc(  2 * sizeof(char) );
	sprintf( reg  , "%c" , regionLetter );
	strcat( url , reg );
	free(reg);
	const char * wild     = "&l=all";
	strcat( url , wild );
	char * filepath =  strchr( url , '/' );
	char * hostname = strndup( url , filepath - url );
	//printf( "hostname is : %s and filepath is : %s\n" , hostname , filepath );

	struct hostent *host = net_gethostbyname(hostname);				// gets host info
	struct sockaddr_in server;
	int socket = net_socket( AF_INET , SOCK_STREAM , IPPROTO_IP );	// creates socket
	if ( host == NULL ) {
		printf("host not found at %s :(\n", hostname);
		free(page);
		free(url);
		free(hostname);
		WaitForButtonPress();
		return -1;
	}
	free(hostname);

	memset( &server , 0 , sizeof(server) );							// reset server var
	server.sin_family	= AF_INET;									// IPv4
	server.sin_port		= htons(80);								// Port 80
	memcpy( &server.sin_addr , host->h_addr_list[0] , host->h_length );
	if ( net_connect( socket , (struct sockaddr*)&server , sizeof(server) ) ) {
		printf("failed to connect :(\n");
		free(page);
		free(url);
		WaitForButtonPress();
		return -1;
	} else {
		//printf("connected :)\n");
	}

	char * getstring = (char*)malloc( strlen("GET HTTP/1.0\r\n\r\n") + strlen(filepath) );
	sprintf( getstring , "GET %s HTTP/1.0\r\n\r\n" , filepath );
	int len  = strlen( getstring );
	int sent = net_write( socket , getstring , len );
	if ( sent < len )
		printf("sent %d of %d bytes\n", sent, len);
	free(getstring);
	int bufferlen = 1025;
	char * buf = (char*)malloc( bufferlen );
	unsigned int received = 0;
	int read = 0;
	response.text = (char*)malloc( sizeof(char) * 32 );
	FILE * fp = fopen( "sd:/temp.txt" , "w" );
	char * line = (char*)malloc( bufferlen );
	char * linebegin;
	char * lineend;
	char dataStarted = 0;
	int headerlength = 0;

	while( (read = net_read( socket , buf , bufferlen - 1 ) ) > 0 ) {
		buf[read] = '\0';		// NULL TERMINATE AMOUNT READ
		linebegin = buf;
		while( (lineend = strchr( linebegin , '\n' ) ) != NULL ) {
			memset( line , '\0' , 1025 );
			strncpy( line , linebegin , lineend - linebegin );
			if ( !dataStarted ) {
				if ( !strncmp( line , "HTTP/" , 5 ) ) {
					sscanf( line , "HTTP/%f %d %s\n" , &(response.version) , &(response.response_code) , response.text );
				} else if ( !strncmp( line , "Content-Length" , 14 ) ) {
					sscanf( line , "Content-Length: %d", &(response.content_length) );
				} else if ( !strncmp( line , "Content-Type" , 12 ) ) {
					char * space = strchr( line  , ' ' );
					char * sc    = strchr( space , ';' );
					char * eq    = strchr( sc    , '=' );
					response.content_type = strndup( ( space + 1 ) , sc - space - 1 );
					response.charset      = strndup( ( eq + 1 ) ,  lineend - eq - 1 );
				} else if ( !strncmp( line , "Last-Modified" , 13 ) ) {
					char * space = strchr( line , ' ' );
					response.modified = strndup( space , lineend - space );
				} else if ( !strcmp( line , "\r" ) ) {
					//printf("end of http header\n");
					dataStarted = 1;
					headerlength = lineend - buf + 1;
				}
			} else {
				//fprintf( fp , "%s\n" , line );
				strncat( page , line , 1025 );
			}
			linebegin = lineend + 1;
		}
		received += read;
	}
	received -= headerlength;

	fclose( fp );
	free(line);
	free(buf);
	free(url);
	//PrintResponse( response );
	free(response.text);
	
	//if ( read == 0 )
	//	printf( "Reached EOF\n" );
	if ( read == -1 )
		printf( "Read Error\n" );
	net_close( socket );

	// Set body as placeholder in page string
	char * body = page;
	// Set body to <BODY> or <body>
	while( ( strncmp( body+1 , "BODY" , 4 ) != 0 ) && ( strncmp( body+1 , "body" , 4 ) != 0 ) ) {
		body = strchr( body+1 , '<' );
		if ( body == NULL ){
			free(page);
			return -1;
		}
	}

	// Loop to get start of titles
	while( strncmp( body+1 , "div class=title" , 15 ) != 0 ) {
		body = strchr( body+1 , '<' );
		if ( ( body == NULL ) || ( strncmp( body + 1 , "/HTML" , 5 ) == 0 ) ){
			free(page);
			return -2;
		}
	}

	int gameCount = 0;
	int terminated = 0;
	// Loop to get Link and Name for the Games List
	while( body != NULL ){
		body = GetGameLink( body , regionLetter );
		if ( body == NULL ) break;
		terminated = GetStringLengthTerminated( body , '"' );
		strncpy( &GameList[ (gameCount * MAX_NAME_LENGTH * 2) + (0 * MAX_NAME_LENGTH) + 0] , body , terminated );
		body = GetGameName( body );
		terminated = GetStringLengthTerminated( body , '<' );
		strncpy( &GameList[ (gameCount * MAX_NAME_LENGTH * 2) + (1 * MAX_NAME_LENGTH) + 0] , body , terminated );
		gameCount++;
	}

	free(page);

	return gameCount;
}

//--------------------------------------------------------------------------------
int ShowGameList( int count , char * GameList) {
//--------------------------------------------------------------------------------
//
//	Params:		count		Number of Games in List
//				GameList	List of Games and Links
//	Returns:	game		Game to Download Codes For
//

	char * game;
	int i;
	int page = 0;
	int totalpages = count / 20;
	int pos = 1;
	int oldpos = 1;
	int max = 20;
	if ( count < 20 ) max = count;

	printf("\x1b[2J");

	while(1){
		oldpos = pos;

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application when home is pressed
		if ( pressed & WPAD_BUTTON_HOME  ) ExitToLoader(0);
		// Increase pos if down   pressed
		if ( pressed & WPAD_BUTTON_DOWN  ) pos++;
		// Increase pos if up     pressed
		if ( pressed & WPAD_BUTTON_UP    ) pos--;
		// Increase page if right pressed
		if ( pressed & WPAD_BUTTON_RIGHT ) { page++; pos = 1; ClearText(); }
		// Increase page if left  pressed
		if ( pressed & WPAD_BUTTON_LEFT  ) { page--; pos = 1; ClearText(); }
		// Return selection if A  pressed
		if ( pressed & WPAD_BUTTON_A     ) return ( 20 * page ) + ( pos - 1 );

		if ( page < 0  ) page = 0;
		if ( page > totalpages ) page = totalpages;

		if ( page == totalpages )
			max = count % 20;
		else
			max = 20;

		if ( pos < 1   ) pos = 1;
		if ( pos > max ) pos = max;

		PrintPositioned( 2 , 0 , "Choose a game:\n\n" );
		for ( i = 0 ; i < max ; i++ ){
			game = &GameList[ ( 20 * page * 2 * MAX_NAME_LENGTH ) + ( i * 2 * MAX_NAME_LENGTH ) + ( 1 * MAX_NAME_LENGTH ) ];
			PrintPositioned( i + 4 , 4 , game );
		}
		PrintPositioned( oldpos + 3 , 1 , " " );
		PrintPositioned( pos    + 3 , 1 , ">" );

		PrintPositioned( 26         , 1 , "Page " );
		printf("%d of %d\n", page + 1 , totalpages + 1 );

	}
}

//--------------------------------------------------------------------------------
int ShowGameRegionMenu() {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Returns:	Region as Integer
//

	int pos = 1;
	int oldpos = 1;

	printf("\x1b[2J");

	while(1){
		oldpos = pos;

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application when home is pressed
		if ( pressed & WPAD_BUTTON_HOME ) ExitToLoader(0);
		// Increase pos if down  pressed
		if ( pressed & WPAD_BUTTON_DOWN ) pos++;
		// Increase pos if up    pressed
		if ( pressed & WPAD_BUTTON_UP   ) pos--;
		// Return selection if A pressed
		if ( pressed & WPAD_BUTTON_A    ) return pos;

		if ( pos < 1 ) pos = 1;
		if ( pos > 4 ) pos = 4;

		PrintPositioned( 2 , 0 , "Choose a game region:\n\n" );
		PrintPositioned( 4 , 4 , "NTSC-U\n");
		PrintPositioned( 5 , 4 , "PAL\n" );
		PrintPositioned( 6 , 4 , "NTSC-J\n" );
		PrintPositioned( 7 , 4 , "other small regions\n" );
		PrintPositioned( oldpos + 3 , 1 , " " );
		PrintPositioned( pos    + 3 , 1 , ">" );

	}
}

//--------------------------------------------------------------------------------
int ShowVCMenu() {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Returns:	Type as Integer
//

	int pos = 1;
	int oldpos = 1;

	printf("\x1b[2J");

	while(1){
		oldpos = pos;

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application when home is pressed
		if ( pressed & WPAD_BUTTON_HOME ) ExitToLoader(0);
		// Increase pos if down  pressed
		if ( pressed & WPAD_BUTTON_DOWN ) pos++;
		// Increase pos if up    pressed
		if ( pressed & WPAD_BUTTON_UP   ) pos--;
		// Return selection if A pressed
		if ( pressed & WPAD_BUTTON_A    ) return pos + 4;

		if ( pos < 1 ) pos =  1;
		if ( pos > 9 ) pos = 9;

		PrintPositioned(  2 , 0 , "Choose a game type:\n\n" );
		PrintPositioned(  4 , 4 , "NES / Famicom\n");
		PrintPositioned(  5 , 4 , "N64\n" );
		PrintPositioned(  6 , 4 , "Sega Master System\n" );
		PrintPositioned(  7 , 4 , "Genesis / MegaDrive\n" );
		PrintPositioned(  8 , 4 , "NeoGeo\n" );
		PrintPositioned(  9 , 4 , "Commodore\n" );
		PrintPositioned( 10 , 4 , "MSX\n" );
		PrintPositioned( 11 , 4 , "TurboGraFx-16\n" );
		PrintPositioned( 12 , 4 , "TurboGraFx-CD\n" );
		PrintPositioned( oldpos + 3 , 1 , " " );
		PrintPositioned( pos    + 3 , 1 , ">" );

	}
}
//--------------------------------------------------------------------------------
int ShowGameTypeMenu() {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Returns:	Type as Integer
//

	int pos = 1;
	int oldpos = 1;

	printf("\x1b[2J");

	while(1){
		oldpos = pos;

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application when home is pressed
		if ( pressed & WPAD_BUTTON_HOME ) ExitToLoader(0);
		// Increase pos if down  pressed
		if ( pressed & WPAD_BUTTON_DOWN ) pos++;
		// Increase pos if up    pressed
		if ( pressed & WPAD_BUTTON_UP   ) pos--;
		// Return selection if A pressed
		if ( pressed & WPAD_BUTTON_A    ) return pos;

		if ( pos < 1 ) pos = 1;
		if ( pos > 4 ) pos = 4;

		PrintPositioned( 2 , 0 , "Choose a game type:\n\n" );
		PrintPositioned( 4 , 4 , "Wii\n");
		PrintPositioned( 5 , 4 , "WiiWare\n" );
		PrintPositioned( 6 , 4 , "VC Arcade\n" );
		PrintPositioned( 7 , 4 , "Wii Channels\n" );
		PrintPositioned( oldpos + 3 , 1 , " " );
		PrintPositioned( pos    + 3 , 1 , ">" );

	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	Init();

	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	//printf("\x1b[2;0H");
	
	//unsigned int array[20];
	//int ret = stuff( array );

	printf("\x1b[20;0H");
	printf("Press HOME to exit or any other button to continue\n");
	WaitForButtonPress();

	int ret;
	int type, region;
	int success = 0;
	//char GameList[MAX_GAME_COUNT][2][MAX_NAME_LENGTH];
	char * GameList = (char*)malloc( MAX_GAME_COUNT * 2 * MAX_NAME_LENGTH * sizeof(char) );

	// Main Game Loop
	while(1){
		success = 0;
		memset( GameList , 0 , MAX_GAME_COUNT * 2 * MAX_NAME_LENGTH );

		while ( success == 0 ) {
			type = ShowGameTypeMenu();
			if ( type == 3 )
				type = ShowVCMenu();
			region = ShowGameRegionMenu();

			ret = GetGameList( type , region , GameList );
			if ( ret == -1 ){
				PrintPositioned( 27 , 10 , "GetGameList failed :(\n" );
				PrintPositioned( 28 , 10 , "No <BODY> found\n" );
				WaitForButtonPress();	
			}
			while ( ret == -2 ){
				//PrintPositioned( 27 , 10 , "GetGameList failed :(\n" );
				//PrintPositioned( 28 , 10 , "No <div class=title> found\n" );
				//WaitForButtonPress();
				ret = GetGameList( type , region , GameList );
			}
			if ( ret == 0 ){
				success = 1;
				PrintPositioned( 27 , 10 , "No games found :(\n" );
				WaitForButtonPress();
			}
			if ( ret > 0 ){
				ret = ShowGameList( ret , GameList );
				success = 1;
				ret = DownloadCodes( ret , type , GameList );
				PrintPositioned( 27 , 10 , "Code Downloaded. Press A to download another.\n" );
				PrintPositioned( 28 , 10 , "Press HOME to exit.\n" );
			}
		}
	}

	free(GameList);

	return 0;
}

#include "download.h"
#include "unistd.h"

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

	//PrintPositioned( 26 , 19 , "Downloading Codes\n");
	dbgprintf("Downloading codes\n");

	char chid = GetGameTypeChar( type );

	struct httpresponse response;

	char * url = NULL;
	try{
		url = new char[80];
	} catch (bad_alloc& ba) {
		printf( "failed to alloc for url in DownloadCodes\n" );
		ExitToLoader( -1 );
	}

	strncpy( url , "geckocodes.org" , 16 );
	char * link = NULL;
	try{
		link = new char[80];
	} catch (bad_alloc& ba) {
		printf( "failed to alloc for link in DownloadCodes\n" );
		ExitToLoader( -1 );
	}

	sprintf( link , "/codes/%c/%s.txt" , chid , &GameList[ game * 2 * MAX_NAME_LENGTH ] );
	strcat( url , link );
	//char * filepath = link;
	char * filepath =  strchr( url , '/' );
	char * hostname = strndup( url , filepath - url );
	//PrintPositioned( 2 , 0 , "" );

	dbgprintf( "url is : %s\n" , url );
	dbgprintf( "hostname is : %s and filepath is : %s\n" , hostname , filepath );

	// Get host info
	struct hostent *host = net_gethostbyname(hostname);
	struct sockaddr_in server;
	dbgprintf("creating a socket for download\n");
	// Create a socket
	int socket = net_socket( AF_INET , SOCK_STREAM , IPPROTO_IP );
	dbgprintf("socket created\n");
	if ( host == NULL ) {
		dbgprintf("host not found at %s :(\n", hostname);
		delete[] url;
		delete[] link;
		WaitForButtonPress();
		return -1;
	}
	dbgprintf("host ok\n");
	memset( &server , 0 , sizeof(server) );							// reset server var
	server.sin_family	= AF_INET;									// IPv4
	server.sin_port		= htons(80);								// Port 80
	memcpy( &server.sin_addr , host->h_addr_list[0] , host->h_length );
	dbgprintf("connecting to server\n");
	if ( net_connect( socket , (struct sockaddr*)&server , sizeof(server) ) ) {
		printf("failed to connect :(\n");
		delete[] url;
		delete[] link;
		net_close(socket);
		WaitForButtonPress();
		return -1;
	} else {
		dbgprintf("connected :)\n");
		//WaitForButtonPress();
	}

	char * getstring = NULL;
	try{
		getstring = new char[ strlen("GET http://HTTP/1.0\r\n\r\n") + strlen(url) + 2 ];
	} catch (bad_alloc& ba) {
		printf( "failed to alloc for getstring in DownloadCodes\n" );
		ExitToLoader( -1 );
	}

	sprintf( getstring , "GET http://%s HTTP/1.0\r\n\r\n" , url );
	dbgprintf("getstring: %s\n", getstring);
	int len  = strlen( getstring );
	int sent = net_write( socket , getstring , len );
	delete[] getstring;
	if ( sent < len )
		printf("sent %d of %d bytes\n", sent, len);
	int bufferlen = 4097;
	char * buf = NULL;
	try{
		buf = new char[ bufferlen ];
	} catch (bad_alloc &ba) {
		printf( "failed to alloc for buf in DownloadCodes\n" );
		ExitToLoader( -1 );
	}

	unsigned int received = 0;
	int read = 0;
	response.text = NULL;
	try{
		response.text = new char[32];
	} catch (bad_alloc &ba) {
		printf( "failed to alloc for response.text in DownloadCodes\n" );
		ExitToLoader( -1 );
	}
	response.charset = NULL;
	response.modified = NULL;
	response.content_type = NULL;
	response.content_length = 0;

	char * filename = NULL;
	try{
		filename = new char[100];
	} catch (bad_alloc& ba) {
		printf( "failed to alloc for filename in DownloadCodes\n" );
		ExitToLoader( -1 );
	}

	if(chdir("sd:/txtcodes") != 0)
		mkdir("sd:/txtcodes", 0);
	strncpy( filename , "sd:/txtcodes/" , 14 );
	strncat( filename , &GameList[ ( game * 2 * MAX_NAME_LENGTH ) + MAX_NAME_LENGTH ] , GetStringLengthTerminated( &GameList[ ( game * 2 * MAX_NAME_LENGTH ) + MAX_NAME_LENGTH ] , '[' ) - 1 );
	strncat( filename , ".txt" , 5 );

	FILE * fp = fopen( filename , "w" );
	if ( fp == NULL ) {
		printf( "Error opening %s\n" , filename );
		int err = ELM_GetError();
		printf( "Error code: %d\n" , err );
		ELM_Unmount();
		WaitForButtonPress();
		ExitToLoader(1);
	}
	delete[] filename;

	char * line = NULL;
	try{
		line = new char[ bufferlen ];
	} catch (bad_alloc& ba) {
		printf( "failed to alloc for line in DownloadCodes\n" );
		ExitToLoader( -1 );
	}

	char * linebegin = NULL;
	char * lineend = NULL;
	char dataStarted = 0;
	int headerlength = 0;

	dbgprintf("buf offset: %p\n", buf);
	dbgprintf("line offset: %p\n", line);

	while( (read = net_read( socket , buf , bufferlen - 1 ) ) > 0 ) {
		buf[read] = '\0';		// NULL TERMINATE AMOUNT READ
		linebegin = buf;
		dbgprintf( "reset linebegin\n" );
		//hexdump( buf , read );
		while( (lineend = strchr( linebegin , '\n' ) ) != NULL ) {
			memset( line , '\0' , 1025 );
			strncpy( line , linebegin , lineend - linebegin );
			//dbgprintf( "line: %s\n" , line );
			//dbgprintf( "buf : %s\n" , buf );
			if ( !dataStarted ) {
				if ( !strncmp( line , "HTTP/" , 5 ) ) {
					dbgprintf( "http: \n" );
					sscanf( line , "HTTP/%f %d %s\n" , &(response.version) , &(response.response_code) , response.text );
				} else if ( !strncmp( line , "Content-Length" , 14 ) ) {
					dbgprintf( "content-length: \n" );
					sscanf( line , "Content-Length: %d", &(response.content_length) );
				} else if ( !strncmp( line , "Content-Type" , 12 ) ) {
					dbgprintf( "content-type: \n" );
					char * space = strchr( line  , ' ' );
					char * sc    = strchr( space , ';' );
					char * eq    = strchr( sc    , '=' );
					response.content_type = strndup( ( space + 1 ) , sc - space - 1 );
					response.charset      = strndup( ( eq + 1 ) ,  lineend - eq - 1 );
				} else if ( !strncmp( line , "Last-Modified" , 13 ) ) {
					dbgprintf( "last-modified: \n" );
					char * space = strchr( line , ' ' );
					response.modified = strndup( space , lineend - space );
				} else if ( !strcmp( line , "\r" ) ) {
					dbgprintf("end of http header\n");
					dbgprintf( "end of http header: \n" );
					dataStarted = 1;
					headerlength = lineend - buf + 1;
				}
			} else {
				dbgprintf( "data: \n" );
				dbgprintf( "%s\n" , line );
				fprintf( fp , "%s\n" , line );
			}
			//dbgprintf( "linebegin: %p lineend: %p\n" , linebegin , lineend );
			//dbgprintf( "linebegin: %.20s lineend: %.20s\n" , linebegin , lineend );
			linebegin = lineend + 1;
		}
		dbgprintf( "memset: buf: %p size: %d\n" , buf , bufferlen );
		memset( buf , 0 , bufferlen );
		received += read;
	}
	received -= headerlength;
	dbgprintf("received bytes: %08x\n", received);
	net_close(socket);

	delete[] url;
	delete[] link;
	delete[] line;
	delete[] buf;

	//PrintResponse( response );
	delete[] response.text;
	dbgprintf( "response charset free: \n" );
	if ( response.charset != NULL )
		free(response.charset);
	dbgprintf( "response modified free: \n" );
	if ( response.modified != NULL )
		free(response.modified);
	dbgprintf( "response content type free: \n" );
	if ( response.content_type != NULL )
		free(response.content_type);

	fclose( fp );
	if ( read == 0 )
		dbgprintf( "Reached EOF\n" );
	else if ( read == -1 )
		printf( "Read Error\n" );
	net_close( socket );

	dbgprintf("Codes copied to file\n");

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

	//PrintPositioned( 26 , 10 , "Retrieving Game List\n");
	dbgprintf( "Retrieving Game List\n" );

	char chidLetter , regionLetter;
	chidLetter		= GetGameTypeChar( category );
	regionLetter	= GetGameRegionChar( region );

	char * page = NULL;
	page = new char[0xFFFFF];
	if ( page == NULL ) {
		dbgprintf( "failed to allocate buffer for page in GetGameList\n" );
		WaitForButtonPress();
		ExitToLoader(-1);
	}

	struct httpresponse response;

	char * url = NULL;
	try{
		url = new char[90];
	} catch (bad_alloc& ba) {
		dbgprintf( "failed to allocate buffer for url in GetGameList\n" );
		ExitToLoader(-1);
	}

	strncpy( url , "geckocodes.org/index.php?chid=" , 31 );
	char * chid = NULL;
	try{
		chid = new char[2];
	} catch (bad_alloc& ba) {
		dbgprintf( "failed to allocate buffer for chid in GetGameList\n" );
		ExitToLoader(-1);
	}

	sprintf( chid , "%c" , chidLetter );
	strcat( url , chid );
	delete[] chid;
	const char * re = "&r=";
	strcat( url , re );
	char * reg = NULL;
	try{
		reg = new char[2];
	} catch (bad_alloc& ba) {
		dbgprintf( "failed to allocate buffer for reg in GetGameList\n" );
		ExitToLoader(-1);
	}

	sprintf( reg  , "%c" , regionLetter );
	strcat( url , reg );
	delete[] reg;
	const char * wild = "&l=all";
	strcat( url , wild );
	char * filepath =  strchr( url , '/' );
	char * hostname = strndup( url , filepath - url );
	dbgprintf( "url is : %s\n" , url );
	dbgprintf( "hostname is : %s and filepath is : %s\n" , hostname , filepath );

	// Get host info
	struct hostent *host = net_gethostbyname(hostname);
	struct sockaddr_in server;
	// Create a socket
	int socket = net_socket( AF_INET , SOCK_STREAM , IPPROTO_IP );
	if ( host == NULL ) {
		dbgprintf("host not found at %s :(\n", hostname);
		delete[] page;
		delete[] url;
		if ( hostname != NULL )
			free(hostname);
		WaitForButtonPress();
		return -1;
	}
	if ( DEBUG )
		dbgprintf( "get game list hostname free\n" );
	if ( hostname != NULL )
		free(hostname);

	memset( &server , 0 , sizeof(server) );							// reset server var
	server.sin_family	= AF_INET;									// IPv4
	server.sin_port		= htons(80);								// Port 80
	memcpy( &server.sin_addr , host->h_addr_list[0] , host->h_length );
	if ( net_connect( socket , (struct sockaddr*)&server , sizeof(server) ) ) {
		dbgprintf("failed to connect :(\n");
		delete[] page;
		delete[] url;
		net_close(socket);
		WaitForButtonPress();
		return -1;
	} else {
		dbgprintf("connected :)\n");
	}

	char * getstring = NULL;
	try{
		//getstring = new char[ strlen("GET HTTP/1.0\r\n\r\n") + strlen(filepath) + 2 ];
		getstring = new char[ strlen("GET http://HTTP/1.0\r\n\r\n") + strlen(url) + 2 ];
	} catch (bad_alloc& ba) {
		dbgprintf( "failed to allocate buffer for getstring in GetGameList\n" );
		ExitToLoader(-1);
	}

	//sprintf( getstring , "GET %s HTTP/1.0\r\n\r\n" , filepath );
	sprintf( getstring , "GET http://%s HTTP/1.0\r\n\r\n" , url );
	dbgprintf( "request string: %s\n" , getstring );
	int len  = strlen( getstring );
	int sent = net_write( socket , getstring , len );
	if ( sent < len )
		dbgprintf("sent %d of %d bytes\n", sent, len);
	delete[] getstring;
	int bufferlen = 4097;
	char * buf = NULL;
	try{
		buf = new char[ bufferlen ];
	} catch (bad_alloc& ba) {
		dbgprintf( "failed to allocate buffer for buf in GetGameList\n" );
		ExitToLoader( -1 );
	}

	unsigned int received = 0;
	int read = 0;
	response.text = NULL;
	try{
		response.text = new char[32];
	} catch (bad_alloc& ba) {
		dbgprintf( "failed to allocate buffer for response.text in GetGameList\n" );
		ExitToLoader( -1 );
	}
	response.charset = NULL;
	response.modified = NULL;
	response.content_type = NULL;
	response.content_length = 0;

	char * line = NULL;
	try{
		line = new char[ bufferlen ];
	} catch (bad_alloc& ba) {
		dbgprintf( "failed to allocate buffer for line in GetGameList\n" );
		ExitToLoader(-1);
	}
	char * linebegin = NULL;
	char * lineend = NULL;
	char dataStarted = 0;
	int headerlength = 0;

	while( (read = net_read( socket , buf , bufferlen - 1 ) ) > 0 ) {
		buf[read] = '\0';		// NULL TERMINATE AMOUNT READ
		linebegin = buf;
		while( (lineend = strchr( linebegin , '\n' ) ) != NULL ) {
			memset( line , 0 , bufferlen );
			strncpy( line , linebegin , lineend - linebegin );
			if ( !dataStarted ) {
				if ( !strncmp( line , "HTTP/" , 5 ) ) {
					dbgprintf( "%s\n" , line );
					sscanf( line , "HTTP/%f %d %s\n" , &(response.version) , &(response.response_code) , response.text );
				} else if ( !strncmp( line , "Content-Length" , 14 ) ) {
					dbgprintf( "%s\n" , line );
					sscanf( line , "Content-Length: %d", &(response.content_length) );
				} else if ( !strncmp( line , "Content-Type" , 12 ) ) {
					dbgprintf( "%s\n" , line );
					char * space = strchr( line  , ' ' );
					char * sc    = strchr( space , ';' );
					char * eq    = strchr( sc    , '=' );
					response.content_type = strndup( ( space + 1 ) , sc - space - 1 );
					response.charset      = strndup( ( eq + 1 ) ,  lineend - eq - 1 );
				} else if ( !strncmp( line , "Last-Modified" , 13 ) ) {
					dbgprintf( "%s\n" , line );
					char * space = strchr( line , ' ' );
					response.modified = strndup( space , lineend - space );
				} else if ( !strcmp( line , "\r" ) ) {
					dbgprintf("end of http header\n");
					dataStarted = 1;
					headerlength = lineend - buf + 1;
				} else {
					dbgprintf("unknown header: %s\n", line);
				}
			} else {
				//dbgprintf( "DATA: %s\n" , line );
				strncat( page , line , bufferlen );
			}
			linebegin = lineend + 1;
		}
		received += read;
	}
	received -= headerlength;
	net_close(socket);

	delete[] line;
	delete[] buf;
	delete[] url;
	//PrintResponse( response );
	delete[] response.text;
	if ( DEBUG )
		dbgprintf( "response charset free: \n" );
	if ( response.charset != NULL )
		free(response.charset);
	if ( DEBUG )
		dbgprintf( "response modified free: \n" );
	if ( response.modified != NULL )
		free(response.modified);
	if ( DEBUG )
		dbgprintf( "response content type free: \n" );
	if ( response.content_type != NULL )
		free(response.content_type);
	
	if ( read == 0 )
		dbgprintf( "Reached EOF\n" );
	else if ( read == -1 )
		dbgprintf( "Read Error\n" );
	net_close( socket );

	// Set body as placeholder in page string
	char * body = page;
	if ( DEBUG )
	{
		dbgprintf("%s\n", page);
		sleep(5);
	}
	// Set body to <BODY> or <body>
	while( ( strncmp( body+1 , "BODY" , 4 ) != 0 ) && ( strncmp( body+1 , "body" , 4 ) != 0 ) ) {
		body = strchr( body+1 , '<' );
		if ( body == NULL ){
			delete[] page;
			return -1;
		}
	}
	dbgprintf("BODY found\n");

	// Loop to get start of titles
	while( strncmp( body+1 , "div class=title" , 15 ) != 0 ) {
		body = strchr( body+1 , '<' );
		if ( ( body == NULL ) || ( strncmp( body + 1 , "/HTML" , 5 ) == 0 ) ){
			delete[] page;
			return -2;
		}
	}
	dbgprintf("div class=title found\n");

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

	delete[] page;

	dbgprintf( "GameList populated\n" );

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

	char * game = NULL;
	int i;
	int page = 0;
	int totalpages = count / 20;
	int pos = 1;
	int oldpos = 1;
	int max = 20;
	if ( count < 20 ) max = count;

	ClearText( );

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

	ClearText( );

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

	ClearText( );

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

	ClearText( );

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


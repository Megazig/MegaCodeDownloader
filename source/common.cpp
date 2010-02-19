#include "common.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

#define MYIP 0xc0a80092		//192.168.0.146
//#define MYIP 0xc0a801b4		//192.168.1.144

//s32 ExitRequested = 0;
s8 HWButton = -1;
//struct SSettings Settings;
int ExitRequested = 0;

//--------------------------------------------------------------------------------
void WiiResetPressed() {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Returns:	None
//
	HWButton = SYS_RETURNTOMENU;
}

//--------------------------------------------------------------------------------
void WiiPowerPressed() {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Returns:	None
//
	HWButton = SYS_POWEROFF_STANDBY;
}

//--------------------------------------------------------------------------------
void WiimotePowerPressed( int channel ) {
//--------------------------------------------------------------------------------
//
//	Params:		channel		Channel of wiimote whose power button was pressed
//	Returns:	None
//
	HWButton = SYS_POWEROFF_STANDBY;
}

//--------------------------------------------------------------------------------
void SetTextInfo( int color_fg , int color_bg , int attribute ) {
//--------------------------------------------------------------------------------
//
//	Params:		color_fg			Foreground Color For Text
//				color_bg			Background Color For Text
//				attribute			Text Style
//	Returns:	None
//

	dbgprintf( "\x1b[%dm" , attribute );
	dbgprintf( "\x1b[%dm" , color_fg );
	dbgprintf( "\x1b[%dm" , color_bg );
}

//--------------------------------------------------------------------------------
void ClearText( ) {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Returns:	None
//

	dbgprintf( "\x1b[2J" );
}

//--------------------------------------------------------------------------------
void PrintPositioned( int y , int x , const char * text ) {
//--------------------------------------------------------------------------------
//
//	Params:		y			Y Position for Text
//				x			X Position for Text
//				text		String to Print
//	Returns:	None
//

	dbgprintf("\x1b[%d;%dH", y , x );
	dbgprintf("%s" , text );
}

//--------------------------------------------------------------------------------
void PrintPositioned( int y , int x , string text ) {
//--------------------------------------------------------------------------------
//
//	Params:		y			Y Position for Text
//				x			X Position for Text
//				text		String to Print
//	Returns:	None
//

	dbgprintf("\x1b[%d;%dH", y , x );
	dbgprintf("%s" , text.c_str() );
}

//--------------------------------------------------------------------------------
void ExitToLoader( int return_val ) {
//--------------------------------------------------------------------------------
//
//	Params:		return_val	Value to Pass to Exit
//	Return:		None		exits
//

	dbgprintf("\n\n\n\n\n\n\n");
	dbgprintf("\x1b[25;5HThanks for using MegaDownloader\n");
	ShutoffRumble();
	StopGX();
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

		// Add check for Reset or Power button
		if ( HWButton != -1 )
			SYS_ResetSystem( HWButton , 0 , 0 );

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

	dbgprintf("\x1b[2;0H");
	dbgprintf("MegaCodeDownloader\n");
	dbgprintf("coded by\n");
	SetTextInfo( BLINKING_TEXT , GREEN_FG , BLACK_BG );
	dbgprintf("megazig\n");
	SetTextInfo( RESET_TEXT , WHITE_FG , BLACK_BG );
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
	if ( end != NULL ) {
		int length = end - text;
		int i;
		for ( i = 0 ; i < length ; i++ )
			printf("%.1s", text + i );
		dbgprintf("\n");
	} else {
		dbgprintf( "%s\n" , text );
	}
}

//--------------------------------------------------------------------------------
void PrintResponse( struct httpresponse response ) {
//--------------------------------------------------------------------------------
//
//	Params:		response	HTTP Response
//	Returns:	None
//

	if ( response.text != NULL )
		dbgprintf("\tHTTP/%1.1f %d %s\n", response.version, response.response_code, response.text);
	if ( response.modified != NULL )
		dbgprintf("\t%d bytes long\tModified: %s\n", response.content_length, response.modified);
	if ( ( response.content_type != NULL ) && ( response.charset != NULL ) )
		dbgprintf("\tContent Type: %s; charset %s\n\n", response.content_type, response.charset);
}

//--------------------------------------------------------------------------------
void Init() {
//--------------------------------------------------------------------------------
//
//	Params:		None
//	Returns:	None
//

	/*
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
	*/

	// Stuff for LibWiiGUI
	InitVideo();
	SetupPads();
	InitAudio();
	InitFreeType((u8*)font_ttf, font_ttf_size);
	InitGUIThreads();

	SYS_SetResetCallback( WiiResetPressed );
	SYS_SetPowerCallback( WiiPowerPressed );
	WPAD_SetPowerButtonCallback( WiimotePowerPressed );

	/*
	printf("\x1b[2J");
	printf("\x1b[2;0H");
	ShowProgramInfo();
	*/

	//PrintPositioned( 16 , 0 , "Initializing Network..................." );
	char * myIpAddy = NULL;
	myIpAddy = new (std::nothrow) char[16];
	if ( myIpAddy == NULL ) {
		dbgprintf( "failed to alloc for IP Address\n" );
		ExitToLoader( -1 );
	}
	if (if_config(myIpAddy, NULL, NULL, true)){
		//PrintPositioned( 16 , 45 , "Failed to initialize network :( \n" );
		dbgprintf( "Failed to initialize network :( \n" );
		//WaitForButtonPress();
		ExitToLoader( ENETINIT );
	}
	//PrintPositioned( 16 , 45 , "COMPLETE\n" );

	//NCconnect(0xc0a80092); // 192.168.0.146
	NCconnect(MYIP);

	dbgprintf("IP Address: %s\n", myIpAddy);
	delete[] myIpAddy;

	dbgprintf( "Initializing FAT File System.........." );
	uSyncInit();
	if ( ELM_Mount() & 1 ) {
		//PrintPositioned( 15 , 45 , "ELM_Mount failed :( \n" );
		dbgprintf( "ELM_Mount failed :( \n" );
		//WaitForButtonPress();
		ExitToLoader( EELMMOUNT );
	}
	dbgprintf( "COMPLETE\n" );

}


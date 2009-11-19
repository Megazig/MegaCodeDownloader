#ifndef COMMON_H
#define COMMON_H

#include "main.h"

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

using std::string;

void WiiResetPressed();
void WiiPowerPressed();
void WiimotePowerPressed( int channel );
void SetTextInfo( int color_fg , int color_bg , int attribute );
void ClearText( );
void PrintPositioned( int y , int x , const char * text );
void PrintPositioned( int y , int x , string text );
void ExitToLoader( int return_val );
void WaitForButtonPress();
void ShowProgramInfo();
int GetStringLengthTerminated( char * text , char terminator );
void PrintCharTerminated( char * text , char terminator );
void PrintResponse( struct httpresponse response );
void Init();

extern int ExitRequested;

#endif

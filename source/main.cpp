#include "main.h"

struct SSettings Settings;

//---------------------------------------------------------------------------------
void DefaultSettings()
//---------------------------------------------------------------------------------
{
	Settings.LoadMethod = METHOD_AUTO;
	Settings.SaveMethod = METHOD_AUTO;
	sprintf (Settings.Folder1,"libwiigui/first folder");
	sprintf (Settings.Folder2,"libwiigui/second folder");
	sprintf (Settings.Folder3,"libwiigui/third folder");
	Settings.AutoLoad = 1;
	Settings.AutoSave = 1;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	Init();
	DefaultSettings();
	MainMenu(MENU_MAIN);

	/*
	//unsigned int array[20];
	//int ret = stuff( array );

	PrintPositioned( 20 , 0 , "Press HOME to exit or any other button to continue\n");
	WaitForButtonPress();

	int ret;
	int type, region;
	int success = 0;
	int timeout = 0;

	char * GameList = NULL;
	try{
		GameList = new char[ MAX_GAME_COUNT * 2 * MAX_NAME_LENGTH ];
	} catch (bad_alloc& ba) {
		printf( "failed to allocate buffer for GameList in main\n" );
		ExitToLoader(-1);
	}

	// Main Game Loop
	while(1){
		timeout = 0;
		success = 0;
		dbgprintf( "memset: GameList: %p size: %08x\n" , MAX_GAME_COUNT * 2 * MAX_NAME_LENGTH );
		dbgprintf( "MAX_GAME_COUNT: %08x\nMAX_NAME_LENGTH: %08x\n" , MAX_GAME_COUNT , MAX_NAME_LENGTH );
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
				timeout++;
				if ( timeout >= 50 ) {
					PrintPositioned( 27 , 10 , "GetGameList failed :(\n" );
					PrintPositioned( 28 , 10 , "No <div class=title> found\n" );
					WaitForButtonPress();
					ExitToLoader(-1);
				}
				ret = GetGameList( type , region , GameList );
			}
			dbgprintf( "ret != 0\nMAX_GAME_COUNT: %08x\nMAX_NAME_LENGTH: %08x\n" , MAX_GAME_COUNT , MAX_NAME_LENGTH );
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
				WaitForButtonPress();
			}
		}
	}

	delete[] GameList;
	*/

	return 0;
}

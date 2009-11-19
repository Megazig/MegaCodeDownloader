/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.h
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#ifndef _MENU_H_
#define _MENU_H_

#include <ogcsys.h>

void InitGUIThreads();
void MainMenu (int menuitem);

enum
{
	MENU_EXIT = -1,
	MENU_NONE,
	MENU_MAIN,
	MENU_SETTINGS,
	MENU_SETTINGS_FILE,
	MENU_BROWSE_DEVICE,
	MENU_VC_GAME_TYPES,
	MENU_GAME_TYPES,
	MENU_GAME_REGIONS,
	MENU_GAME_LIST,
	MENU_CONVERT_CODE
};

#endif

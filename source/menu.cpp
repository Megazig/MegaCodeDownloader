/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.cpp
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>

#include "libwiigui/gui.h"
#include "menu.h"
#include "input.h"
#include "filelist.h"
#include "filebrowser.h"
#include "common.h"
#include "main.h"
#include "download.h"

#define THREAD_SLEEP 100

static GuiImageData * pointer[4];
static GuiImage * bgImg = NULL;
static GuiSound * bgMusic = NULL;
static GuiWindow * mainWindow = NULL;
static lwp_t guithread = LWP_THREAD_NULL;
static bool guiHalt = true;
static game_info info;

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
static void
ResumeGui()
{
	guiHalt = false;
	LWP_ResumeThread (guithread);
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
static void
HaltGui()
{
	guiHalt = true;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(guithread))
		usleep(THREAD_SLEEP);
}

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice
 ***************************************************************************/
int
WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label)
{
	int choice = -1;

	GuiWindow promptWindow(448,288);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiText titleTxt(title, 26, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,40);
	GuiText msgTxt(msg, 22, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-20);
	msgTxt.SetWrap(true, 400);

	GuiText btn1Txt(btn1Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	GuiImage btn1ImgOver(&btnOutlineOver);
	GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());

	if(btn2Label)
	{
		btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		btn1.SetPosition(20, -25);
	}
	else
	{
		btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
		btn1.SetPosition(0, -25);
	}

	btn1.SetLabel(&btn1Txt);
	btn1.SetImage(&btn1Img);
	btn1.SetImageOver(&btn1ImgOver);
	btn1.SetSoundOver(&btnSoundOver);
	btn1.SetTrigger(&trigA);
	btn1.SetState(STATE_SELECTED);
	btn1.SetEffectGrow();

	GuiText btn2Txt(btn2Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn2Img(&btnOutline);
	GuiImage btn2ImgOver(&btnOutlineOver);
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	btn2.SetPosition(-20, -25);
	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetImageOver(&btn2ImgOver);
	btn2.SetSoundOver(&btnSoundOver);
	btn2.SetTrigger(&trigA);
	btn2.SetEffectGrow();

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
	promptWindow.Append(&btn1);

	if(btn2Label)
		promptWindow.Append(&btn2);

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

	while(choice == -1)
	{
		usleep(THREAD_SLEEP);

		if(btn1.GetState() == STATE_CLICKED)
			choice = 1;
		else if(btn2.GetState() == STATE_CLICKED)
			choice = 0;
	}

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);
	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return choice;
}

/****************************************************************************
 * UpdateGUI
 *
 * Primary thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/

static void *
UpdateGUI (void *arg)
{
	int i;

	while(1)
	{
		if(guiHalt)
		{
			LWP_SuspendThread(guithread);
		}
		else
		{
			UpdatePads();
			mainWindow->Draw();

			#ifdef HW_RVL
			for(i=3; i >= 0; i--) // so that player 1's cursor appears on top!
			{
				if(userInput[i].wpad->ir.valid)
					Menu_DrawImg(userInput[i].wpad->ir.x-48, userInput[i].wpad->ir.y-48,
						96, 96, pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 1, 1, 255);
				DoRumble(i);
			}
			#endif

			Menu_Render();

			for(i=0; i < 4; i++)
				mainWindow->Update(&userInput[i]);

			if(ExitRequested)
			{
				for(i = 0; i < 255; i += 15)
				{
					mainWindow->Draw();
					Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor){0, 0, 0, i},1);
					Menu_Render();
				}
				ExitToLoader(0);
			}
		}
	}
	return NULL;
}

/****************************************************************************
 * InitGUIThread
 *
 * Startup GUI threads
 ***************************************************************************/
void
InitGUIThreads()
{
	LWP_CreateThread (&guithread, UpdateGUI, NULL, NULL, 0, 70);
}

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
static void OnScreenKeyboard(char * var, u16 maxlen)
{
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText okBtnTxt("OK", 22, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetSoundOver(&btnSoundOver);
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 22, (GXColor){0, 0, 0, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetSoundOver(&btnSoundOver);
	cancelBtn.SetTrigger(&trigA);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&keyboard);
	mainWindow->ChangeFocus(&keyboard);
	ResumeGui();

	while(save == -1)
	{
		usleep(THREAD_SLEEP);

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if(save)
	{
		snprintf(var, maxlen, "%s", keyboard.kbtextstr);
	}

	HaltGui();
	mainWindow->Remove(&keyboard);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
}

/****************************************************************************
 * MenuBrowseDevice
 ***************************************************************************/
static int MenuBrowseDevice()
{
	char title[100];
	int i;

	ShutoffRumble();

	// populate initial directory listing
	if(BrowseDevice() <= 0)
	{
		int choice = WindowPrompt(
		"Error",
		"Unable to display files on selected load device.",
		"Retry",
		"Check Settings");

		if(choice)
			return MENU_BROWSE_DEVICE;
		else
			return MENU_SETTINGS;
	}

	int menu = MENU_NONE;

	sprintf(title, "Browse Files");

	GuiText titleTxt(title, 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(100,50);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiFileBrowser fileBrowser(552, 248);
	fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	fileBrowser.SetPosition(0, 100);

	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiText backBtnTxt("Go Back", 24, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(30, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiWindow buttonWindow(screenwidth, screenheight);
	buttonWindow.Append(&backBtn);

	HaltGui();
	mainWindow->Append(&titleTxt);
	mainWindow->Append(&fileBrowser);
	mainWindow->Append(&buttonWindow);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		// update file browser based on arrow buttons
		// set MENU_EXIT if A button pressed on a file
		for(i=0; i < FILE_PAGESIZE; i++)
		{
			if(fileBrowser.fileList[i]->GetState() == STATE_CLICKED)
			{
				fileBrowser.fileList[i]->ResetState();
				// check corresponding browser entry
				if(browserList[browser.selIndex].isdir)
				{
					if(BrowserChangeFolder())
					{
						fileBrowser.ResetState();
						fileBrowser.fileList[0]->SetState(STATE_SELECTED);
						fileBrowser.TriggerUpdate();
					}
					else
					{
						menu = MENU_BROWSE_DEVICE;
						break;
					}
				}
				else
				{
					ShutoffRumble();
					mainWindow->SetState(STATE_DISABLED);
					// load file
					mainWindow->SetState(STATE_DEFAULT);
				}
			}
		}
		if(backBtn.GetState() == STATE_CLICKED)
			menu = MENU_SETTINGS;
	}
	HaltGui();
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(&buttonWindow);
	mainWindow->Remove(&fileBrowser);
	return menu;
}

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
static int MenuSettings()
{
	int menu = MENU_NONE;

	GuiText titleTxt("Settings", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText fileBtnTxt("File Browser", 22, (GXColor){0, 0, 0, 255});
	fileBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage fileBtnImg(&btnLargeOutline);
	GuiImage fileBtnImgOver(&btnLargeOutlineOver);
	GuiButton fileBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	fileBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	fileBtn.SetPosition(50, 120);
	fileBtn.SetLabel(&fileBtnTxt);
	fileBtn.SetImage(&fileBtnImg);
	fileBtn.SetImageOver(&fileBtnImgOver);
	fileBtn.SetSoundOver(&btnSoundOver);
	fileBtn.SetTrigger(&trigA);
	fileBtn.SetEffectGrow();

	GuiText videoBtnTxt("Video", 22, (GXColor){0, 0, 0, 255});
	videoBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage videoBtnImg(&btnLargeOutline);
	GuiImage videoBtnImgOver(&btnLargeOutlineOver);
	GuiButton videoBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	videoBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	videoBtn.SetPosition(0, 120);
	videoBtn.SetLabel(&videoBtnTxt);
	videoBtn.SetImage(&videoBtnImg);
	videoBtn.SetImageOver(&videoBtnImgOver);
	videoBtn.SetSoundOver(&btnSoundOver);
	videoBtn.SetTrigger(&trigA);
	videoBtn.SetEffectGrow();

	GuiText savingBtnTxt1("Saving", 22, (GXColor){0, 0, 0, 255});
	GuiText savingBtnTxt2("&", 18, (GXColor){0, 0, 0, 255});
	GuiText savingBtnTxt3("Loading", 22, (GXColor){0, 0, 0, 255});
	savingBtnTxt1.SetPosition(0, -20);
	savingBtnTxt3.SetPosition(0, +20);
	GuiImage savingBtnImg(&btnLargeOutline);
	GuiImage savingBtnImgOver(&btnLargeOutlineOver);
	GuiButton savingBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	savingBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	savingBtn.SetPosition(-50, 120);
	savingBtn.SetLabel(&savingBtnTxt1, 0);
	savingBtn.SetLabel(&savingBtnTxt2, 1);
	savingBtn.SetLabel(&savingBtnTxt3, 2);
	savingBtn.SetImage(&savingBtnImg);
	savingBtn.SetImageOver(&savingBtnImgOver);
	savingBtn.SetSoundOver(&btnSoundOver);
	savingBtn.SetTrigger(&trigA);
	savingBtn.SetEffectGrow();

	GuiText menuBtnTxt("Menu", 22, (GXColor){0, 0, 0, 255});
	menuBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage menuBtnImg(&btnLargeOutline);
	GuiImage menuBtnImgOver(&btnLargeOutlineOver);
	GuiButton menuBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	menuBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	menuBtn.SetPosition(-125, 250);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetImage(&menuBtnImg);
	menuBtn.SetImageOver(&menuBtnImgOver);
	menuBtn.SetSoundOver(&btnSoundOver);
	menuBtn.SetTrigger(&trigA);
	menuBtn.SetEffectGrow();

	GuiText networkBtnTxt("Network", 22, (GXColor){0, 0, 0, 255});
	networkBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage networkBtnImg(&btnLargeOutline);
	GuiImage networkBtnImgOver(&btnLargeOutlineOver);
	GuiButton networkBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	networkBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	networkBtn.SetPosition(125, 250);
	networkBtn.SetLabel(&networkBtnTxt);
	networkBtn.SetImage(&networkBtnImg);
	networkBtn.SetImageOver(&networkBtnImgOver);
	networkBtn.SetSoundOver(&btnSoundOver);
	networkBtn.SetTrigger(&trigA);
	networkBtn.SetEffectGrow();

	GuiText exitBtnTxt("Exit", 22, (GXColor){0, 0, 0, 255});
	GuiImage exitBtnImg(&btnOutline);
	GuiImage exitBtnImgOver(&btnOutlineOver);
	GuiButton exitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	exitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	exitBtn.SetPosition(100, -35);
	exitBtn.SetLabel(&exitBtnTxt);
	exitBtn.SetImage(&exitBtnImg);
	exitBtn.SetImageOver(&exitBtnImgOver);
	exitBtn.SetSoundOver(&btnSoundOver);
	exitBtn.SetTrigger(&trigA);
	exitBtn.SetTrigger(&trigHome);
	exitBtn.SetEffectGrow();

	GuiText resetBtnTxt("Reset Settings", 22, (GXColor){0, 0, 0, 255});
	GuiImage resetBtnImg(&btnOutline);
	GuiImage resetBtnImgOver(&btnOutlineOver);
	GuiButton resetBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	resetBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	resetBtn.SetPosition(-100, -35);
	resetBtn.SetLabel(&resetBtnTxt);
	resetBtn.SetImage(&resetBtnImg);
	resetBtn.SetImageOver(&resetBtnImgOver);
	resetBtn.SetSoundOver(&btnSoundOver);
	resetBtn.SetTrigger(&trigA);
	resetBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&fileBtn);
	w.Append(&videoBtn);
	w.Append(&savingBtn);
	w.Append(&menuBtn);

#ifdef HW_RVL
	w.Append(&networkBtn);
#endif

	w.Append(&exitBtn);
	w.Append(&resetBtn);

	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(fileBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_BROWSE_DEVICE;
		}
		else if(videoBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_FILE;
		}
		else if(savingBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_FILE;
		}
		else if(menuBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_FILE;
		}
		else if(networkBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_FILE;
		}
		else if(exitBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;
		}
		else if(resetBtn.GetState() == STATE_CLICKED)
		{
			resetBtn.ResetState();

			int choice = WindowPrompt(
				"Reset Settings",
				"Are you sure that you want to reset your settings?",
				"Yes",
				"No");
			if(choice == 1)
			{
				// reset settings
			}
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * MenuConvertCode
 ***************************************************************************/
static int MenuConvertCode()
{
	int menu = MENU_NONE;

	GuiText titleTxt("Convert", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText menuBtnTxt("Main Menu", 22, (GXColor){0, 0, 0, 255});
	GuiImage menuBtnImg(&btnOutline);
	GuiImage menuBtnImgOver(&btnOutlineOver);
	GuiButton menuBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(300, -35);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetImage(&menuBtnImg);
	menuBtn.SetImageOver(&menuBtnImgOver);
	menuBtn.SetSoundOver(&btnSoundOver);
	menuBtn.SetTrigger(&trigA);
	menuBtn.SetTrigger(&trigHome);
	menuBtn.SetEffectGrow();

	GuiText exitBtnTxt("Exit", 22, (GXColor){0, 0, 0, 255});
	GuiImage exitBtnImg(&btnOutline);
	GuiImage exitBtnImgOver(&btnOutlineOver);
	GuiButton exitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	exitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	exitBtn.SetPosition(100, -35);
	exitBtn.SetLabel(&exitBtnTxt);
	exitBtn.SetImage(&exitBtnImg);
	exitBtn.SetImageOver(&exitBtnImgOver);
	exitBtn.SetSoundOver(&btnSoundOver);
	exitBtn.SetTrigger(&trigA);
	exitBtn.SetTrigger(&trigHome);
	exitBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&menuBtn);
	w.Append(&exitBtn);

	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(menuBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_MAIN;
		}
		else if(exitBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * MenuGameList
 ***************************************************************************/
static int MenuGameList()
{
	int menu = MENU_NONE;

	GuiText titleTxt("Download - Game List", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	// GetGameList
	char * GameList = NULL;
	try{
		GameList = new char[ MAX_GAME_COUNT * 2 * MAX_NAME_LENGTH ];
	} catch (bad_alloc& ba) {
		printf( "failed to allocate buffer for GameList in main\n" );
		ExitToLoader(-1);
	}
	memset( GameList , 0 , MAX_GAME_COUNT * 2 * MAX_NAME_LENGTH );

	int count;
	int timeout = 0;
	count = GetGameList( info.type , info.region , GameList );
	if ( count == -1 ){
		dbgprintf( "GetGameList failed :(\n" );
		dbgprintf( "No <BODY> found\n" );
		//ExitToLoader(-1);
	}
	while ( count == -2 ){
		timeout++;
		if ( timeout >= 50 ){
			dbgprintf( "GetGameList failed :(\n" );
			dbgprintf( "No <div class=title> found\n" );
			ExitToLoader(-1);
		}
		count = GetGameList( info.type , info.region , GameList );
	}
	if ( count == 0 ){
		dbgprintf( "No games found :(\n" );
		return menu;
	}

	dbgprintf( "Game count: %d\n" , count );

	int i;
	OptionList options;
	for( i = 0 ; i < count ; i++ ){
		sprintf( options.name[i], &GameList[ ( i * 140 ) + ( 70 ) ] );
		sprintf( options.value[i], "DL" );
	}
	options.length = i;
	dbgprintf( "OptionList populated\n" );

	GuiOptionBrowser optionBrowser(552, 248, &options);
	optionBrowser.SetPosition(0, 108);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	optionBrowser.SetCol2Position(550);
	optionBrowser.SetFocus(true);

	GuiText menuBtnTxt("Main Menu", 22, (GXColor){0, 0, 0, 255});
	GuiImage menuBtnImg(&btnOutline);
	GuiImage menuBtnImgOver(&btnOutlineOver);
	GuiButton menuBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(300, -35);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetImage(&menuBtnImg);
	menuBtn.SetImageOver(&menuBtnImgOver);
	menuBtn.SetSoundOver(&btnSoundOver);
	menuBtn.SetTrigger(&trigA);
	menuBtn.SetTrigger(&trigHome);
	menuBtn.SetEffectGrow();

	GuiText exitBtnTxt("Exit", 22, (GXColor){0, 0, 0, 255});
	GuiImage exitBtnImg(&btnOutline);
	GuiImage exitBtnImgOver(&btnOutlineOver);
	GuiButton exitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	exitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	exitBtn.SetPosition(100, -35);
	exitBtn.SetLabel(&exitBtnTxt);
	exitBtn.SetImage(&exitBtnImg);
	exitBtn.SetImageOver(&exitBtnImgOver);
	exitBtn.SetSoundOver(&btnSoundOver);
	exitBtn.SetTrigger(&trigA);
	exitBtn.SetTrigger(&trigHome);
	exitBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&optionBrowser);
	w.Append(&menuBtn);
	w.Append(&exitBtn);
	dbgprintf( "Buttons and Text appended\n" );

	mainWindow->Append(&w);
	dbgprintf( "window appended\n" );

	ResumeGui();
	dbgprintf( "gui resumed\n" );

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		info.selection = optionBrowser.GetClickedOption();
		if ( info.selection >= 0 ){
			DownloadCodes( info.selection , info.type , GameList );
			dbgprintf( "Codes Downloaded in menu\n" );
			optionBrowser.TriggerUpdate();
			menu = MENU_MAIN;
		}

		if(menuBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_MAIN;
		}
		else if(exitBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;
		}
	}

	delete[] GameList;

	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * MenuSettingsFile
 ***************************************************************************/
static int MenuSettingsFile()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;
	sprintf(options.name[i++], "Load Device");
	sprintf(options.name[i++], "Save Device");
	sprintf(options.name[i++], "Folder 1");
	sprintf(options.name[i++], "Folder 2");
	sprintf(options.name[i++], "Folder 3");
	sprintf(options.name[i++], "Auto Load");
	sprintf(options.name[i++], "Auto Save");
	options.length = i;

	GuiText titleTxt("Settings - Saving & Loading", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(100, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(552, 248, &options);
	optionBrowser.SetPosition(0, 108);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	optionBrowser.SetCol2Position(185);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				Settings.LoadMethod++;
				break;

			case 1:
				Settings.SaveMethod++;
				break;

			case 2:
				OnScreenKeyboard(Settings.Folder1, 256);
				break;

			case 3:
				OnScreenKeyboard(Settings.Folder2, 256);
				break;

			case 4:
				OnScreenKeyboard(Settings.Folder3, 256);
				break;

			case 5:
				Settings.AutoLoad++;
				if (Settings.AutoLoad > 2)
					Settings.AutoLoad = 0;
				break;

			case 6:
				Settings.AutoSave++;
				if (Settings.AutoSave > 3)
					Settings.AutoSave = 0;
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;

			// correct load/save methods out of bounds
			if(Settings.LoadMethod > 4)
				Settings.LoadMethod = 0;
			if(Settings.SaveMethod > 6)
				Settings.SaveMethod = 0;

			if (Settings.LoadMethod == METHOD_AUTO) sprintf (options.value[0],"Auto Detect");
			else if (Settings.LoadMethod == METHOD_SD) sprintf (options.value[0],"SD");
			else if (Settings.LoadMethod == METHOD_USB) sprintf (options.value[0],"USB");
			else if (Settings.LoadMethod == METHOD_DVD) sprintf (options.value[0],"DVD");
			else if (Settings.LoadMethod == METHOD_SMB) sprintf (options.value[0],"Network");

			if (Settings.SaveMethod == METHOD_AUTO) sprintf (options.value[1],"Auto Detect");
			else if (Settings.SaveMethod == METHOD_SD) sprintf (options.value[1],"SD");
			else if (Settings.SaveMethod == METHOD_USB) sprintf (options.value[1],"USB");
			else if (Settings.SaveMethod == METHOD_SMB) sprintf (options.value[1],"Network");
			else if (Settings.SaveMethod == METHOD_MC_SLOTA) sprintf (options.value[1],"MC Slot A");
			else if (Settings.SaveMethod == METHOD_MC_SLOTB) sprintf (options.value[1],"MC Slot B");

			snprintf (options.value[2], 256, "%s", Settings.Folder1);
			snprintf (options.value[3], 256, "%s", Settings.Folder2);
			snprintf (options.value[4], 256, "%s", Settings.Folder3);

			if (Settings.AutoLoad == 0) sprintf (options.value[5],"Off");
			else if (Settings.AutoLoad == 1) sprintf (options.value[5],"Some");
			else if (Settings.AutoLoad == 2) sprintf (options.value[5],"All");

			if (Settings.AutoSave == 0) sprintf (options.value[5],"Off");
			else if (Settings.AutoSave == 1) sprintf (options.value[6],"Some");
			else if (Settings.AutoSave == 2) sprintf (options.value[6],"All");

			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS;
		}
	}
	HaltGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	return menu;
}

/****************************************************************************
 * MenuVCGameTypes
 ***************************************************************************/
static int MenuVCGameTypes()
{
	int menu = MENU_NONE;

	GuiText titleTxt("Download - VC Game Types", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText nesBtnTxt("NES / Famicom", 20, (GXColor){0, 0, 0, 255});
	nesBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage nesBtnImg(&btnLargeOutline);
	GuiImage nesBtnImgOver(&btnLargeOutlineOver);
	GuiButton nesBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	nesBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	nesBtn.SetPosition(50, 100);
	nesBtn.SetLabel(&nesBtnTxt);
	nesBtn.SetImage(&nesBtnImg);
	nesBtn.SetImageOver(&nesBtnImgOver);
	nesBtn.SetSoundOver(&btnSoundOver);
	nesBtn.SetTrigger(&trigA);
	nesBtn.SetEffectGrow();

	GuiText n64BtnTxt("N64", 20, (GXColor){0, 0, 0, 255});
	n64BtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage n64BtnImg(&btnLargeOutline);
	GuiImage n64BtnImgOver(&btnLargeOutlineOver);
	GuiButton n64Btn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	n64Btn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	n64Btn.SetPosition(230, 100);
	n64Btn.SetLabel(&n64BtnTxt);
	n64Btn.SetImage(&n64BtnImg);
	n64Btn.SetImageOver(&n64BtnImgOver);
	n64Btn.SetSoundOver(&btnSoundOver);
	n64Btn.SetTrigger(&trigA);
	n64Btn.SetEffectGrow();

	GuiText smsBtnTxt("Sega Master System", 20, (GXColor){0, 0, 0, 255});
	n64BtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	//smsBtnTxt.SetPosition(0, -20);
	GuiImage smsBtnImg(&btnLargeOutline);
	GuiImage smsBtnImgOver(&btnLargeOutlineOver);
	GuiButton smsBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	smsBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	smsBtn.SetPosition(410, 100);
	smsBtn.SetLabel(&smsBtnTxt);
	smsBtn.SetImage(&smsBtnImg);
	smsBtn.SetImageOver(&smsBtnImgOver);
	smsBtn.SetSoundOver(&btnSoundOver);
	smsBtn.SetTrigger(&trigA);
	smsBtn.SetEffectGrow();

	//50,227
	GuiText genesisBtnTxt("Genesis / MegaDrive", 20, (GXColor){0, 0, 0, 255});
	genesisBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage genesisBtnImg(&btnLargeOutline);
	GuiImage genesisBtnImgOver(&btnLargeOutlineOver);
	GuiButton genesisBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	genesisBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	genesisBtn.SetPosition(50, 195);
	genesisBtn.SetLabel(&genesisBtnTxt);
	genesisBtn.SetImage(&genesisBtnImg);
	genesisBtn.SetImageOver(&genesisBtnImgOver);
	genesisBtn.SetSoundOver(&btnSoundOver);
	genesisBtn.SetTrigger(&trigA);
	genesisBtn.SetEffectGrow();

	GuiText neogeoBtnTxt("NeoGeo", 20, (GXColor){0, 0, 0, 255});
	neogeoBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage neogeoBtnImg(&btnLargeOutline);
	GuiImage neogeoBtnImgOver(&btnLargeOutlineOver);
	GuiButton neogeoBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	neogeoBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	neogeoBtn.SetPosition(230, 195);
	neogeoBtn.SetLabel(&neogeoBtnTxt);
	neogeoBtn.SetImage(&neogeoBtnImg);
	neogeoBtn.SetImageOver(&neogeoBtnImgOver);
	neogeoBtn.SetSoundOver(&btnSoundOver);
	neogeoBtn.SetTrigger(&trigA);
	neogeoBtn.SetEffectGrow();

	GuiText commodoreBtnTxt("Commodore", 20, (GXColor){0, 0, 0, 255});
	commodoreBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage commodoreBtnImg(&btnLargeOutline);
	GuiImage commodoreBtnImgOver(&btnLargeOutlineOver);
	GuiButton commodoreBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	commodoreBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	commodoreBtn.SetPosition(410, 195);
	commodoreBtn.SetLabel(&commodoreBtnTxt);
	commodoreBtn.SetImage(&commodoreBtnImg);
	commodoreBtn.SetImageOver(&commodoreBtnImgOver);
	commodoreBtn.SetSoundOver(&btnSoundOver);
	commodoreBtn.SetTrigger(&trigA);
	commodoreBtn.SetEffectGrow();

	//50,300
	GuiText msxBtnTxt("MSX", 20, (GXColor){0, 0, 0, 255});
	msxBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage msxBtnImg(&btnLargeOutline);
	GuiImage msxBtnImgOver(&btnLargeOutlineOver);
	GuiButton msxBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	msxBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	msxBtn.SetPosition(50, 290);
	msxBtn.SetLabel(&msxBtnTxt);
	msxBtn.SetImage(&msxBtnImg);
	msxBtn.SetImageOver(&msxBtnImgOver);
	msxBtn.SetSoundOver(&btnSoundOver);
	msxBtn.SetTrigger(&trigA);
	msxBtn.SetEffectGrow();

	GuiText tgfx16BtnTxt("TurboGraFx-16", 20, (GXColor){0, 0, 0, 255});
	tgfx16BtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage tgfx16BtnImg(&btnLargeOutline);
	GuiImage tgfx16BtnImgOver(&btnLargeOutlineOver);
	GuiButton tgfx16Btn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	tgfx16Btn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	tgfx16Btn.SetPosition(230, 290);
	tgfx16Btn.SetLabel(&tgfx16BtnTxt);
	tgfx16Btn.SetImage(&tgfx16BtnImg);
	tgfx16Btn.SetImageOver(&tgfx16BtnImgOver);
	tgfx16Btn.SetSoundOver(&btnSoundOver);
	tgfx16Btn.SetTrigger(&trigA);
	tgfx16Btn.SetEffectGrow();

	GuiText tgfxcdBtnTxt("TurboGraFx-CD", 20, (GXColor){0, 0, 0, 255});
	tgfxcdBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage tgfxcdBtnImg(&btnLargeOutline);
	GuiImage tgfxcdBtnImgOver(&btnLargeOutlineOver);
	GuiButton tgfxcdBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	tgfxcdBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	tgfxcdBtn.SetPosition(410, 290);
	tgfxcdBtn.SetLabel(&tgfxcdBtnTxt);
	tgfxcdBtn.SetImage(&tgfxcdBtnImg);
	tgfxcdBtn.SetImageOver(&tgfxcdBtnImgOver);
	tgfxcdBtn.SetSoundOver(&btnSoundOver);
	tgfxcdBtn.SetTrigger(&trigA);
	tgfxcdBtn.SetEffectGrow();

	GuiText menuBtnTxt("Main Menu", 22, (GXColor){0, 0, 0, 255});
	GuiImage menuBtnImg(&btnOutline);
	GuiImage menuBtnImgOver(&btnOutlineOver);
	GuiButton menuBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(300, -35);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetImage(&menuBtnImg);
	menuBtn.SetImageOver(&menuBtnImgOver);
	menuBtn.SetSoundOver(&btnSoundOver);
	menuBtn.SetTrigger(&trigA);
	menuBtn.SetTrigger(&trigHome);
	menuBtn.SetEffectGrow();

	GuiText exitBtnTxt("Exit", 22, (GXColor){0, 0, 0, 255});
	GuiImage exitBtnImg(&btnOutline);
	GuiImage exitBtnImgOver(&btnOutlineOver);
	GuiButton exitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	exitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	exitBtn.SetPosition(100, -35);
	exitBtn.SetLabel(&exitBtnTxt);
	exitBtn.SetImage(&exitBtnImg);
	exitBtn.SetImageOver(&exitBtnImgOver);
	exitBtn.SetSoundOver(&btnSoundOver);
	exitBtn.SetTrigger(&trigA);
	exitBtn.SetTrigger(&trigHome);
	exitBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&nesBtn);
	w.Append(&n64Btn);
	w.Append(&smsBtn);
	w.Append(&genesisBtn);
	w.Append(&neogeoBtn);
	w.Append(&commodoreBtn);
	w.Append(&msxBtn);
	w.Append(&tgfx16Btn);
	w.Append(&tgfxcdBtn);
	w.Append(&menuBtn);
	w.Append(&exitBtn);

	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(nesBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_NES;
			menu = MENU_GAME_REGIONS;
		}
		else if(n64Btn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_N64;
			menu = MENU_GAME_REGIONS;
		}
		else if(smsBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_SMS;
			menu = MENU_GAME_REGIONS;
		}
		else if(genesisBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_GENESIS;
			menu = MENU_GAME_REGIONS;
		}
		else if(neogeoBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_NEOGEO;
			menu = MENU_GAME_REGIONS;
		}
		else if(commodoreBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_COMMODORE;
			menu = MENU_GAME_REGIONS;
		}
		else if(msxBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_MSX;
			menu = MENU_GAME_REGIONS;
		}
		else if(tgfx16Btn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_TGFX16;
			menu = MENU_GAME_REGIONS;
		}
		else if(tgfxcdBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_TGFXCD;
			menu = MENU_GAME_REGIONS;
		}
		else if(menuBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_MAIN;
		}
		else if(exitBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}


/****************************************************************************
 * MenuGameTypes
 ***************************************************************************/
static int MenuGameTypes()
{
	int menu = MENU_NONE;

	GuiText titleTxt("Download - Game Types", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText wiiBtnTxt("Wii", 22, (GXColor){0, 0, 0, 255});
	wiiBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage wiiBtnImg(&btnLargeOutline);
	GuiImage wiiBtnImgOver(&btnLargeOutlineOver);
	GuiButton wiiBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	wiiBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	wiiBtn.SetPosition(50, 120);
	wiiBtn.SetLabel(&wiiBtnTxt);
	wiiBtn.SetImage(&wiiBtnImg);
	wiiBtn.SetImageOver(&wiiBtnImgOver);
	wiiBtn.SetSoundOver(&btnSoundOver);
	wiiBtn.SetTrigger(&trigA);
	wiiBtn.SetEffectGrow();

	GuiText wiiwareBtnTxt("WiiWare", 22, (GXColor){0, 0, 0, 255});
	wiiwareBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage wiiwareBtnImg(&btnLargeOutline);
	GuiImage wiiwareBtnImgOver(&btnLargeOutlineOver);
	GuiButton wiiwareBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	wiiwareBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	wiiwareBtn.SetPosition(0, 120);
	wiiwareBtn.SetLabel(&wiiwareBtnTxt);
	wiiwareBtn.SetImage(&wiiwareBtnImg);
	wiiwareBtn.SetImageOver(&wiiwareBtnImgOver);
	wiiwareBtn.SetSoundOver(&btnSoundOver);
	wiiwareBtn.SetTrigger(&trigA);
	wiiwareBtn.SetEffectGrow();

	GuiText vcBtnTxt("VC Arcade", 22, (GXColor){0, 0, 0, 255});
	vcBtnTxt.SetPosition(0, -20);
	GuiImage vcBtnImg(&btnLargeOutline);
	GuiImage vcBtnImgOver(&btnLargeOutlineOver);
	GuiButton vcBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	vcBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	vcBtn.SetPosition(-50, 120);
	vcBtn.SetLabel(&vcBtnTxt, 0);
	vcBtn.SetImage(&vcBtnImg);
	vcBtn.SetImageOver(&vcBtnImgOver);
	vcBtn.SetSoundOver(&btnSoundOver);
	vcBtn.SetTrigger(&trigA);
	vcBtn.SetEffectGrow();

	GuiText channelsBtnTxt("Wii Channels", 22, (GXColor){0, 0, 0, 255});
	channelsBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage channelsBtnImg(&btnLargeOutline);
	GuiImage channelsBtnImgOver(&btnLargeOutlineOver);
	GuiButton channelsBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	channelsBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	channelsBtn.SetPosition(-125, 250);
	channelsBtn.SetLabel(&channelsBtnTxt);
	channelsBtn.SetImage(&channelsBtnImg);
	channelsBtn.SetImageOver(&channelsBtnImgOver);
	channelsBtn.SetSoundOver(&btnSoundOver);
	channelsBtn.SetTrigger(&trigA);
	channelsBtn.SetEffectGrow();

	GuiText menuBtnTxt("Main Menu", 22, (GXColor){0, 0, 0, 255});
	GuiImage menuBtnImg(&btnOutline);
	GuiImage menuBtnImgOver(&btnOutlineOver);
	GuiButton menuBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(300, -35);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetImage(&menuBtnImg);
	menuBtn.SetImageOver(&menuBtnImgOver);
	menuBtn.SetSoundOver(&btnSoundOver);
	menuBtn.SetTrigger(&trigA);
	menuBtn.SetTrigger(&trigHome);
	menuBtn.SetEffectGrow();

	GuiText exitBtnTxt("Exit", 22, (GXColor){0, 0, 0, 255});
	GuiImage exitBtnImg(&btnOutline);
	GuiImage exitBtnImgOver(&btnOutlineOver);
	GuiButton exitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	exitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	exitBtn.SetPosition(100, -35);
	exitBtn.SetLabel(&exitBtnTxt);
	exitBtn.SetImage(&exitBtnImg);
	exitBtn.SetImageOver(&exitBtnImgOver);
	exitBtn.SetSoundOver(&btnSoundOver);
	exitBtn.SetTrigger(&trigA);
	exitBtn.SetTrigger(&trigHome);
	exitBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&wiiBtn);
	w.Append(&wiiwareBtn);
	w.Append(&vcBtn);
	w.Append(&channelsBtn);
	w.Append(&menuBtn);
	w.Append(&exitBtn);

	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(wiiBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_WII;
			menu = MENU_GAME_REGIONS;
		}
		else if(wiiwareBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_WIIWARE;
			menu = MENU_GAME_REGIONS;
		}
		else if(vcBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_VC;
			menu = MENU_VC_GAME_TYPES;
		}
		else if(channelsBtn.GetState() == STATE_CLICKED)
		{
			info.type = TYPE_WII_CHANNELS;
			menu = MENU_GAME_REGIONS;
		}
		else if(menuBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_MAIN;
		}
		else if(exitBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * MenuGameRegions
 ***************************************************************************/
static int MenuGameRegions()
{
	int menu = MENU_NONE;

	GuiText titleTxt("Download - Game Regions", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText ntscuBtnTxt("NTSC-U", 22, (GXColor){0, 0, 0, 255});
	ntscuBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage ntscuBtnImg(&btnLargeOutline);
	GuiImage ntscuBtnImgOver(&btnLargeOutlineOver);
	GuiButton ntscuBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	ntscuBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	ntscuBtn.SetPosition(50, 120);
	ntscuBtn.SetLabel(&ntscuBtnTxt);
	ntscuBtn.SetImage(&ntscuBtnImg);
	ntscuBtn.SetImageOver(&ntscuBtnImgOver);
	ntscuBtn.SetSoundOver(&btnSoundOver);
	ntscuBtn.SetTrigger(&trigA);
	ntscuBtn.SetEffectGrow();

	GuiText palBtnTxt("PAL", 22, (GXColor){0, 0, 0, 255});
	palBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage palBtnImg(&btnLargeOutline);
	GuiImage palBtnImgOver(&btnLargeOutlineOver);
	GuiButton palBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	palBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	palBtn.SetPosition(0, 120);
	palBtn.SetLabel(&palBtnTxt);
	palBtn.SetImage(&palBtnImg);
	palBtn.SetImageOver(&palBtnImgOver);
	palBtn.SetSoundOver(&btnSoundOver);
	palBtn.SetTrigger(&trigA);
	palBtn.SetEffectGrow();

	GuiText ntscjBtnTxt("NTSC-J", 22, (GXColor){0, 0, 0, 255});
	ntscjBtnTxt.SetPosition(0, -20);
	GuiImage ntscjBtnImg(&btnLargeOutline);
	GuiImage ntscjBtnImgOver(&btnLargeOutlineOver);
	GuiButton ntscjBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	ntscjBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	ntscjBtn.SetPosition(-50, 120);
	ntscjBtn.SetLabel(&ntscjBtnTxt, 0);
	ntscjBtn.SetImage(&ntscjBtnImg);
	ntscjBtn.SetImageOver(&ntscjBtnImgOver);
	ntscjBtn.SetSoundOver(&btnSoundOver);
	ntscjBtn.SetTrigger(&trigA);
	ntscjBtn.SetEffectGrow();

	GuiText otherBtnTxt("other small regions", 22, (GXColor){0, 0, 0, 255});
	otherBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage otherBtnImg(&btnLargeOutline);
	GuiImage otherBtnImgOver(&btnLargeOutlineOver);
	GuiButton otherBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	otherBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	otherBtn.SetPosition(-125, 250);
	otherBtn.SetLabel(&otherBtnTxt);
	otherBtn.SetImage(&otherBtnImg);
	otherBtn.SetImageOver(&otherBtnImgOver);
	otherBtn.SetSoundOver(&btnSoundOver);
	otherBtn.SetTrigger(&trigA);
	otherBtn.SetEffectGrow();

	GuiText menuBtnTxt("Main Menu", 22, (GXColor){0, 0, 0, 255});
	GuiImage menuBtnImg(&btnOutline);
	GuiImage menuBtnImgOver(&btnOutlineOver);
	GuiButton menuBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(300, -35);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetImage(&menuBtnImg);
	menuBtn.SetImageOver(&menuBtnImgOver);
	menuBtn.SetSoundOver(&btnSoundOver);
	menuBtn.SetTrigger(&trigA);
	menuBtn.SetTrigger(&trigHome);
	menuBtn.SetEffectGrow();

	GuiText exitBtnTxt("Exit", 22, (GXColor){0, 0, 0, 255});
	GuiImage exitBtnImg(&btnOutline);
	GuiImage exitBtnImgOver(&btnOutlineOver);
	GuiButton exitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	exitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	exitBtn.SetPosition(100, -35);
	exitBtn.SetLabel(&exitBtnTxt);
	exitBtn.SetImage(&exitBtnImg);
	exitBtn.SetImageOver(&exitBtnImgOver);
	exitBtn.SetSoundOver(&btnSoundOver);
	exitBtn.SetTrigger(&trigA);
	exitBtn.SetTrigger(&trigHome);
	exitBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&ntscuBtn);
	w.Append(&palBtn);
	w.Append(&ntscjBtn);
	w.Append(&otherBtn);
	w.Append(&menuBtn);
	w.Append(&exitBtn);

	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(ntscuBtn.GetState() == STATE_CLICKED)
		{
			info.region = REGION_NTSCU;
			menu = MENU_GAME_LIST;
		}
		else if(palBtn.GetState() == STATE_CLICKED)
		{
			info.region = REGION_PAL;
			menu = MENU_GAME_LIST;
		}
		else if(ntscjBtn.GetState() == STATE_CLICKED)
		{
			info.region = REGION_NTSCJ;
			menu = MENU_GAME_LIST;
		}
		else if(otherBtn.GetState() == STATE_CLICKED)
		{
			info.region = REGION_OTHER;
			menu = MENU_GAME_LIST;
		}
		else if(menuBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_MAIN;
		}
		else if(exitBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
static int MenuMain()
{
	int menu = MENU_NONE;

	GuiText titleTxt("MegaCodeDownloader", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText downloadBtnTxt("Download Codes", 22, (GXColor){0, 0, 0, 255});
	downloadBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage downloadBtnImg(&btnLargeOutline);
	GuiImage downloadBtnImgOver(&btnLargeOutlineOver);
	GuiButton downloadBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	downloadBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	downloadBtn.SetPosition(50, 120);
	downloadBtn.SetLabel(&downloadBtnTxt);
	downloadBtn.SetImage(&downloadBtnImg);
	downloadBtn.SetImageOver(&downloadBtnImgOver);
	downloadBtn.SetSoundOver(&btnSoundOver);
	downloadBtn.SetTrigger(&trigA);
	downloadBtn.SetEffectGrow();

	GuiText convertBtnTxt("Convert Codes", 22, (GXColor){0, 0, 0, 255});
	convertBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage convertBtnImg(&btnLargeOutline);
	GuiImage convertBtnImgOver(&btnLargeOutlineOver);
	GuiButton convertBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	convertBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	convertBtn.SetPosition(0, 120);
	convertBtn.SetLabel(&convertBtnTxt);
	convertBtn.SetImage(&convertBtnImg);
	convertBtn.SetImageOver(&convertBtnImgOver);
	convertBtn.SetSoundOver(&btnSoundOver);
	convertBtn.SetTrigger(&trigA);
	convertBtn.SetEffectGrow();

	GuiText exitBtnTxt("Exit", 22, (GXColor){0, 0, 0, 255});
	GuiImage exitBtnImg(&btnOutline);
	GuiImage exitBtnImgOver(&btnOutlineOver);
	GuiButton exitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	exitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	exitBtn.SetPosition(100, -35);
	exitBtn.SetLabel(&exitBtnTxt);
	exitBtn.SetImage(&exitBtnImg);
	exitBtn.SetImageOver(&exitBtnImgOver);
	exitBtn.SetSoundOver(&btnSoundOver);
	exitBtn.SetTrigger(&trigA);
	exitBtn.SetTrigger(&trigHome);
	exitBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&downloadBtn);
	w.Append(&convertBtn);
	w.Append(&exitBtn);

	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if ( DEBUG )
			if(userInput[0].wpad->ir.valid)
				dbgprintf("Pointer Position: %f %f\n" , userInput[0].wpad->ir.x, userInput[0].wpad->ir.y );

		if(downloadBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAME_TYPES;
		}
		else if(convertBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_CONVERT_CODE;
		}
		else if(exitBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * MainMenuCode
 ***************************************************************************/
void MainMenu(int menu)
{

	info.type = -1;
	info.region = -1;
	info.selection = -1;
	info.name = NULL;

	int currentMenu = menu;

	#ifdef HW_RVL
	pointer[0] = new GuiImageData(player1_point_png);
	pointer[1] = new GuiImageData(player2_point_png);
	pointer[2] = new GuiImageData(player3_point_png);
	pointer[3] = new GuiImageData(player4_point_png);
	#endif

	mainWindow = new GuiWindow(screenwidth, screenheight);

	bgImg = new GuiImage(screenwidth, screenheight, (GXColor){50, 50, 50, 255});
	bgImg->ColorStripe(30);
	mainWindow->Append(bgImg);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	ResumeGui();

	bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG);
	bgMusic->SetVolume(50);
	bgMusic->Play(); // startup music

	while(currentMenu != MENU_EXIT)
	{
		switch (currentMenu)
		{
			case MENU_MAIN:
				currentMenu = MenuMain();
				break;
			/*
			case MENU_SETTINGS:
				currentMenu = MenuSettings();
				break;
			case MENU_SETTINGS_FILE:
				currentMenu = MenuSettingsFile();
				break;
			case MENU_BROWSE_DEVICE:
				currentMenu = MenuBrowseDevice();
				break;
			*/
			case MENU_VC_GAME_TYPES:
				currentMenu = MenuVCGameTypes();
				break;
			case MENU_GAME_TYPES:
				currentMenu = MenuGameTypes();
				break;
			case MENU_GAME_REGIONS:
				currentMenu = MenuGameRegions();
				break;
			case MENU_GAME_LIST:
				currentMenu = MenuGameList();
				break;
			case MENU_CONVERT_CODE:
				currentMenu = MenuConvertCode();
				break;
			default: // unrecognized menu
				currentMenu = MenuMain();
				break;
		}
	}

	ResumeGui();
	ExitRequested = 1;
	while(1) usleep(THREAD_SLEEP);

	HaltGui();

	bgMusic->Stop();
	delete bgMusic;
	delete bgImg;
	delete mainWindow;

	delete pointer[0];
	delete pointer[1];
	delete pointer[2];
	delete pointer[3];

	mainWindow = NULL;
}

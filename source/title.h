#ifndef TITLEH
#define TITLEH

#include <stdlib.h>
#include "ios.h"

class Title{
	public:
		char * name;
		char * cheat_link;
		int    titleid_h;
		int    titleid_l;
		IOS  * ios;

		Title( );
		char * GetName( );
		void   SetName( char * name );
		char * GetLink( );
		void   SetLink( char * link );
		int    GetTitleIDHigh( );
		void   SetTitleIDHigh( int high );
		int    GetTitleIDLow( );
		void   SetTitleIDLow( int low );
		long long GetTitleID( );
		void      SetTitleID( long long titleid );
};

#endif

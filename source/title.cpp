#include "title.h"

Title::Title( ) {
	this->name = NULL;
	this->cheat_link = NULL;
	this->titleid_h = 0;
	this->titleid_l = 0;
	this->ios = NULL;
}

char * Title::GetName( ) {
	return this->name;
}

void   Title::SetName( char * name ) {
	this->name = name;
}

char * Title::GetLink( ) {
	return this->cheat_link;
}

void   Title::SetLink( char * link ) {
	this->cheat_link = link;
}

int    Title::GetTitleIDHigh( ) {
	return this->titleid_h;
}

void   Title::SetTitleIDHigh( int high ) {
	this->titleid_h = high;
}

int    Title::GetTitleIDLow( ) {
	return this->titleid_l;
}

void   Title::SetTitleIDLow( int low ) {
	this->titleid_l = low;
}

long long Title::GetTitleID( ) {
	return ( (long long)this->titleid_h << 32 ) | this->titleid_l;
}

void      Title::SetTitleID( long long titleid ) {
	this->titleid_h = (int)(titleid >> 32);
	this->titleid_l = (int)titleid & 0xffffffff;
}

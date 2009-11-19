/*
 * Application control class
 *
 * Copyright (C) 2006-2008  Alejandro Valenzuela Roca, <lanjoe9 at mexinetica NOSPAMPLEASE dot com>
 *
 * http://mexinetica.com/~lanjoe9
 *
 * This file is part of MotorJ, a free framework for videogame development.
 * <http://wiki.lidsol.net/wiki/index.php?title=MotorJ >
 * <http://motorj.mexinetica.com/ >
 *
 * MotorJ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MotorJ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MotorJ.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APPLICATION_CLASS_H
#define APPLICATION_CLASS_H

#include <gccore.h>

#include "data_structs.h"
#include "musicplayer-class.h"
#include "networkctl-class.h"

//#include "glContexts.h"

class mjApplication
{
	public:
	int video_mode;
#ifdef PC_SDL
	SDL_Surface * screen;
#endif
	int gl_width;
	int gl_height;
	bool shaders_sup;
	bool audio_sup;
	bool network_sup;
	bool font_sup;
	bool video_sup;
	bool fullscreen;
	float fps;
	int argc;
	char ** argv;
	int return_value;

	list_header  extra_data;
	list_header  universes;

#ifndef PC_SDL
	mjNetworkCTL NetworkCTL;
#endif

	Mtx view, perspective; // view and perspective matrices
	Mtx model, modelview[12];
	unsigned wiiCurrentMatrixIndex;
	u32 wiiCurrentFB;
	Mtx * wiiCurrentMtx;
	GXRModeObj * wiiRMode;
	u32 wiiXFBHeight;
	GXColor wiiBgColor;
	void * wiiFIFO;
	f32 wiiYScale;
	bool wiiShowConsole;

	#ifdef GLCONTEXTS_H
	MJglxData glContextData;
	#endif
	private:

};


#endif

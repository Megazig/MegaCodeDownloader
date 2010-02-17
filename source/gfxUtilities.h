#ifndef GFXUTILITIESH
#define GFXUTILITIESH

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <gccore.h>

#include "libpng/pngu/pngu.h"

#define	DEFAULT_FIFO_SIZE	( 1024 * 1024 )

void enableTexture();
void enableDraw();

class GUI
{
	public:
		static void*	frameBuffer[2];
		GXRModeObj*		rmode;
		u32				fb;
		f32				yscale;
		u32				xfbHeight;
		Mtx				perspective;
		Mtx				GXmodelView2D;
		void*			gp_fifo;

		GUI();
		GXRModeObj * VideoInit();
		void GraphicsInit();
		void VideoCleanup();
		void * getCurrentFrameBuffer();
		void * getSpareFrameBuffer();
		void blankScreen();
		void swapFrameBuffers();
		void waitVSync();

		void screenShot(const char* filename);
		void renderToFrameBuffer(void* fb);

		/*
		void drawTile( int x, int y, int width, int height, int image, f32 angle );
		void loadPngTiles(const u8* pngBuffer, int numCols, int numRows);
		*/
};

#endif

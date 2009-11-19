#ifndef IMAGEH
#define IMAGEH

#include <gccore.h>

#include "gfxUtilities.h"

class Image
{
	public:
		u32 width , height;
		GXTexObj * texture;

		Image();
		void DrawImage( f32 xpos , f32 ypos , u16 width , u16 height , u8 data[] , f32 degrees , f32 scaleX , f32 scaleY , u8 alpha , GUI * gui );

};

#endif

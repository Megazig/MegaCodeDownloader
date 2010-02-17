#ifndef PANEH
#define PANEH

#include <ogc/gx.h>

class Pane
{
	public:
		bool visible;
		float xTrans , yTrans , zTrans;
		float xRotate , yRotate , zRotate;
		float xScale , yScale; 
		float width , height;
		GXTexObj * texture;

		Pane();
		void LoadTexture( void * buffer );
		void LoadTexture( GXTexObj * texture );
		void Draw();
};

#endif

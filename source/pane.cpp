#include "pane.h"

Pane::Pane()
{
	visible	= true;
	xTrans	= 0;
	yTrans	= 0;
	zTrans	= 0;
	xRotate	= 0;
	yRotate	= 0;
	zRotate	= 0;
	xScale	= 0;
	yScale	= 0;
	texture = NULL;
}

void Pane::LoadTexture( void * buffer )
{
	return;
}

void Pane::LoadTexture( GXTexObj * texture )
{
	//this->texture = *texture;
}

void Pane::Draw()
{
	if ( visible ) {
		GX_Begin( GX_QUADS , GX_VTXFMT0 , 4 );

		// Vertex 1
		GX_Position3f32( xTrans - ( width / 2 ) , yTrans - ( height / 2 ) , zTrans );
		GX_Color3u8( 0xFF , 0xFF , 0xFF );
		GX_TexCoord2f32( 0 , 0 );

		// Vertex 2
		GX_Position3f32( xTrans + ( width / 2 ) , yTrans - ( height / 2 ) , zTrans );
		GX_Color3u8( 0xFF , 0xFF , 0xFF );
		GX_TexCoord2f32( 1 , 0 );

		// Vertex 3
		GX_Position3f32( xTrans + ( width / 2 ) , yTrans + ( height / 2 ) , zTrans );
		GX_Color3u8( 0xFF , 0xFF , 0xFF );
		GX_TexCoord2f32( 0 , 1 );

		// Vertex 4
		GX_Position3f32( xTrans - ( width / 2 ) , yTrans + ( height / 2 ) , zTrans );
		GX_Color3u8( 0xFF , 0xFF , 0xFF );
		GX_TexCoord2f32( 1 , 1 );

		GX_End();
	}

}

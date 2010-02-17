#include "geometry.h"

void drawRect(int x1, int y1, int width, int height, const GXColor &c, bool fill)
{
	const int x2 = x1 + width;
	const int y2 = y1 + height;
	enableDraw();
	GX_Begin(fill?GX_QUADS:GX_LINESTRIP, GX_VTXFMT0, fill?4:5);	// Draw A Quad
		GX_Position2f32(x1, y1);								// Top Left
		GX_Color4u8(c.r,c.g,c.b,c.a);

		GX_Position2f32(x2, y1);								// Top Right
		GX_Color4u8(c.r,c.g,c.b,c.a);

		GX_Position2f32(x2, y2);								// Bottom Right
		GX_Color4u8(c.r,c.g,c.b,c.a);

		GX_Position2f32(x1, y2);								// Bottom Left
		GX_Color4u8(c.r,c.g,c.b,c.a);
		if(!fill)
		{
			GX_Position2f32(x1, y1);							// Top Left
			GX_Color4u8(c.r,c.g,c.b,c.a);
		}
	GX_End();													// Done Drawing The Quad 	
}

void drawShape(const int points[], int numPoints, const GXColor &c, bool fill)
{
	enableDraw();
	GX_Begin(fill?GX_TRIANGLESTRIP:GX_LINESTRIP, GX_VTXFMT0, fill?numPoints:numPoints+1);	// Draw A Quad
		for(int i=0;i<numPoints;++i)
		{
			GX_Position2f32(points[i*2], points[i*2+1]);									// draw
			GX_Color4u8(c.r,c.g,c.b,c.a);
		}
		if(!fill)
		{
			GX_Position2f32(points[0], points[1]);											// back to first point
			GX_Color4u8(c.r,c.g,c.b,c.a);
		}
	GX_End();																				// Done Drawing The Shape 	
}

void drawTorus( float x , float y , float z )
{
	int i, j;
	float theta, phi, theta1;
	float cosTheta, sinTheta;
	float cosTheta1, sinTheta1;
	float ringDelta, sideDelta;
	float cosPhi, sinPhi, dist;

	Mtx modelView;

	sideDelta = 2 * M_PI / 128;
	ringDelta = 2 * M_PI / 256;

	theta = 0;
	cosTheta = 1;
	sinTheta = 0;

	// Load modelview
	guMtxIdentity(modelView);
	//guMtxTransApply(modelView, modelView, 0, 0, mTorusPosition.z);
	guMtxTransApply(modelView, modelView, 0, 0, z);
	GX_LoadPosMtxImm(modelView, GX_PNMTX0);

	for (i = 127; i > 0; i--)
	{
		theta1 = theta + ringDelta;
		cosTheta1 = cos(theta1);
		sinTheta1 = sin(theta1);

		GX_Begin (GX_TRIANGLESTRIP, GX_VTXFMT0, 512);
		phi = 0;
		for (j = 256; j > 0; j--)
		{
			phi += sideDelta;
			cosPhi = cos(phi);
			sinPhi = sin(phi);
			dist = 1.8 + (0.6 * cosPhi);

          	GX_Normal3f32(cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi);
			GX_Position3f32(cosTheta1 * dist, -sinTheta1 * dist, 0.6 * sinPhi);
			GX_TexCoord2f32(0, 0);

			GX_Normal3f32(cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi);
			GX_Position3f32(cosTheta * dist, -sinTheta * dist, 0.6 * sinPhi);
			GX_TexCoord2f32(0, 0);
		}
		GX_End();
      
		theta = theta1;
		cosTheta = cosTheta1;
		sinTheta = sinTheta1;
	}
}

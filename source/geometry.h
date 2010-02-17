#ifndef GEOMETRYH
#define GEOMETRYH

#include <math.h>

#include <gccore.h>
#include "gfxUtilities.h"

void drawRect(int x1, int y1, int width, int height, const GXColor &c, bool fill = true);
void drawShape(const int points[], int numPoints, const GXColor &c, bool fill = true);
void drawTorus( float x , float y , float z );

#endif

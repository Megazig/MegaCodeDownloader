#include "gfxUtilities.h"

/*
int tilesCols , tilesRows;
*/

void enableTexture()
{
	GX_ClearVtxDesc();
	GX_SetVtxDesc( GX_VA_POS , GX_DIRECT );
	GX_SetVtxDesc( GX_VA_CLR0 , GX_NONE );
	GX_SetVtxDesc( GX_VA_TEX0 , GX_DIRECT );

	GX_SetNumTexGens(1);
	GX_SetTevOp( GX_TEVSTAGE0  , GX_REPLACE );
	GX_SetTevOrder( GX_TEVSTAGE0 , GX_TEXCOORD0 , GX_TEXMAP0 , GX_COLORZERO );
}

void enableDraw()
{
	GX_SetNumTexGens(0);
	GX_InvVtxCache();
	GX_SetTevOp( GX_TEVSTAGE0  , GX_PASSCLR );
	GX_SetTevOrder( GX_TEVSTAGE0 , GX_TEXCOORDNULL , GX_TEXMAP_DISABLE , GX_COLOR0A0 );

	GX_ClearVtxDesc();
	GX_SetVtxDesc( GX_VA_POS , GX_DIRECT );
	GX_SetVtxDesc( GX_VA_TEX0 , GX_NONE );
	GX_SetVtxDesc( GX_VA_CLR0 , GX_DIRECT );
}

GUI::GUI()
{
	this->frameBuffer[0] = NULL;
	this->frameBuffer[1] = NULL;
	this->fb = 0;
}

GXRModeObj * GUI::VideoInit()
{
	VIDEO_Init();
	this->rmode = VIDEO_GetPreferredMode(NULL);
	VIDEO_Configure(this->rmode);

	// allocate 2 framebuffers for double buffering
	this->frameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(this->rmode));
	this->frameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(this->rmode));
	VIDEO_ClearFrameBuffer(this->rmode, this->frameBuffer[this->fb], 0);
	VIDEO_SetNextFramebuffer(this->frameBuffer[this->fb]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();

	//debug
	console_init(this->frameBuffer[0],20,20,this->rmode->fbWidth,this->rmode->xfbHeight,this->rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	VIDEO_WaitVSync();
	if(this->rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	return this->rmode;
}

void GUI::GraphicsInit()
{
	// setup the fifo and then init the flipper
	this->gp_fifo = NULL;
	this->gp_fifo = memalign(32,DEFAULT_FIFO_SIZE);
	memset(this->gp_fifo,0,DEFAULT_FIFO_SIZE);
 
	GX_Init(this->gp_fifo,DEFAULT_FIFO_SIZE);
 
	// clears the bg to color and clears the z buffer
	GXColor background = {0, 0, 0, 0xff};
	GX_SetCopyClear(background, 0x00ffffff);
 
	// other gx setup
	this->yscale = GX_GetYScaleFactor(this->rmode->efbHeight,this->rmode->xfbHeight);
	this->xfbHeight = GX_SetDispCopyYScale(this->yscale);
	GX_SetScissor(0,0,this->rmode->fbWidth,this->rmode->efbHeight);
	GX_SetDispCopySrc(0,0,this->rmode->fbWidth,this->rmode->efbHeight);
	GX_SetDispCopyDst(this->rmode->fbWidth,this->xfbHeight);
	GX_SetCopyFilter(this->rmode->aa,this->rmode->sample_pattern,GX_TRUE,this->rmode->vfilter);
	GX_SetFieldMode(this->rmode->field_rendering,((this->rmode->viHeight==2*this->rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));
	if (this->rmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
	GX_SetDispCopyGamma(GX_GM_1_0);

	// setup the vertex descriptor
	// tells the flipper to expect direct data
	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	
	//  for textures
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_NONE);
	
	// for direct draw
//	GX_InvalidateTexAll();
//	GX_SetVtxDesc(GX_VA_TEX1, GX_NONE);
//	GX_SetVtxDesc(GX_VA_CLR1, GX_DIRECT);

	// used for textures
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_TRUE);

	// used for direct color
//	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS,  GX_POS_XY, GX_F32, 0);
//	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);
//	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0);
	
	GX_SetNumChans(1);
	GX_SetNumTevStages(1);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	//GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	guMtxIdentity(this->GXmodelView2D);
	guMtxTransApply (this->GXmodelView2D, this->GXmodelView2D, 0.0F, 0.0F, -5.0F);
	//guMtxTransApply (this->GXmodelView2D, this->GXmodelView2D, 0.0F, 0.0F, -50.0F);
	GX_LoadPosMtxImm(this->GXmodelView2D,GX_PNMTX0);

	guOrtho(perspective,0,479,0,639,0,300);
	GX_LoadProjectionMtx(this->perspective, GX_ORTHOGRAPHIC);

	GX_SetViewport(0,0,this->rmode->fbWidth,this->rmode->efbHeight,0,1);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);
	
	GX_SetCullMode(GX_CULL_NONE);

	GX_InvalidateTexAll();
}

void GUI::VideoCleanup()
{
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	free(MEM_K1_TO_K0(this->frameBuffer[0]));
	free(MEM_K1_TO_K0(this->frameBuffer[1]));
	this->frameBuffer[0] = NULL;
	this->frameBuffer[1] = NULL;
}

void * GUI::getCurrentFrameBuffer()
{
	return this->frameBuffer[this->fb];
}

void * GUI::getSpareFrameBuffer()
{
	return this->frameBuffer[this->fb?0:1];
}

void GUI::blankScreen()
{
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
}

void GUI::swapFrameBuffers()
{
	this->fb ^= 1;		// flip framebuffer
	VIDEO_SetNextFramebuffer(this->frameBuffer[this->fb]);
	VIDEO_Flush();
}

void GUI::waitVSync()
{
	VIDEO_WaitVSync();
}

void GUI::screenShot(const char* filename)
{
	IMGCTX ctx = PNGU_SelectImageFromDevice(filename);
	PNGU_EncodeFromYCbYCr (ctx, this->rmode->fbWidth, this->rmode->efbHeight, this->getCurrentFrameBuffer(), 0);
	PNGU_ReleaseImageContext(ctx);
}

void GUI::renderToFrameBuffer(void* fb)
{
	GX_DrawDone();
	
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(fb,GX_TRUE);
}

/*
//---------------------------------------------------------------------------------
void drawTile( int x, int y, int width, int height, int image, f32 angle )
{
	GUI::enableTexture();
//---------------------------------------------------------------------------------

	// Set up transform
	Mtx23 transform;
	guMtx23RotDeg(transform, angle);
	guMtx23ScaleApply(transform, transform, width, height);
	guMtx23TransApply(transform, transform, x-width/2.0f, y-width/2.0f);
	
	// set up unity quad
	Mtx24 quad;
	guMtx24Concat(transform, unitQuad, quad);

	int tileX = image % tilesCols;
	int tileY = image / tilesCols;
	float s1 = tileX/(float)tilesCols;
	float s2 = (tileX+1)/(float)tilesCols;
	float t1 = tileY / (float)tilesRows;
	float t2 = (tileY+1)/(float)tilesRows;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);			// Draw A Quad
		GX_Position2f32(quad[0][0], quad[1][0]);	// Top Left
		GX_TexCoord2f32(s1,t1);

		GX_Position2f32(quad[0][1], quad[1][1]);	// Top Right
		GX_TexCoord2f32(s2,t1);

		GX_Position2f32(quad[0][2], quad[1][2]);	// Bottom Right
		GX_TexCoord2f32(s2,t2);

		GX_Position2f32(quad[0][3], quad[1][3]);	// Bottom Left
		GX_TexCoord2f32(s1,t2);
	GX_End();						// Done Drawing The Quad 

}

void loadPngTiles(const u8* pngBuffer, int numCols, int numRows)
{
	IMGCTX ctx = PNGU_SelectImageFromBuffer(pngBuffer);
	PNGUPROP imgProp;
	PNGU_GetImageProperties(ctx, &imgProp);
	u8* tilesTex = reinterpret_cast<u8*>(memalign(32,imgProp.imgWidth*imgProp.imgHeight*4));
	PNGU_DecodeTo4x4RGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, tilesTex, 0xFF);
	PNGU_ReleaseImageContext(ctx);
	DCFlushRange(tilesTex, imgProp.imgWidth*imgProp.imgHeight*4);

	GXTexObj texObj;
	GX_InitTexObj(&texObj, MEM_K0_TO_K1((void*)tilesTex), imgProp.imgWidth, imgProp.imgHeight, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	tilesCols = numCols;
	tilesRows = numRows;
}
*/


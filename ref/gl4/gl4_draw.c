/*
gl_draw.c - orthogonal drawing stuff
Copyright (C) 2010 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "gl4_local.h"

/*
=============
R_GetImageParms
=============
*/
void R_GetTextureParms( int *w, int *h, int texnum )
{
}

/*
=============
R_GetSpriteParms

same as GetImageParms but used
for sprite models
=============
*/
void R_GetSpriteParms( int *frameWidth, int *frameHeight, int *numFrames, int currentFrame, const model_t *pSprite )
{
}

int R_GetSpriteTexture( const model_t *m_pSpriteModel, int frame )
{
}

/*
=============
R_DrawStretchPic
=============
*/
void R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, int texnum )
{
}

/*
=============
R_DrawStretchRaw
=============
*/
void R_DrawStretchRaw( float x, float y, float w, float h, int cols, int rows, const byte *data, qboolean dirty )
{
}

/*
=============
R_UploadStretchRaw
=============
*/
void R_UploadStretchRaw( int texture, int cols, int rows, int width, int height, const byte *data )
{
}

/*
===============
R_Set2DMode
===============
*/
void R_Set2DMode( qboolean enable )
{
}

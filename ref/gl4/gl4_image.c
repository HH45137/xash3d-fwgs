/*
gl_image.c - texture uploading and processing
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

#include <stdarg.h>
#include "gl4_local.h"
#include "crclib.h"

#define TEXTURES_HASH_SIZE	(MAX_TEXTURES >> 2)

static gl_texture_t		gl_textures[MAX_TEXTURES];
static gl_texture_t*	gl_texturesHashTable[TEXTURES_HASH_SIZE];
static uint		gl_numTextures;

static byte    dottexture[8][8] =
{
	  {0,1,1,0,0,0,0,0},
	  {1,1,1,1,0,0,0,0},
	  {1,1,1,1,0,0,0,0},
	  {0,1,1,0,0,0,0,0},
	  {0,0,0,0,0,0,0,0},
	  {0,0,0,0,0,0,0,0},
	  {0,0,0,0,0,0,0,0},
	  {0,0,0,0,0,0,0,0},
};


#define IsLightMap( tex )	( FBitSet(( tex )->flags, TF_ATLAS_PAGE ))
/*
=================
R_GetTexture

acess to array elem
=================
*/
gl_texture_t *R_GetTexture( GLenum texnum )
{
	ASSERT( texnum >= 0 && texnum < MAX_TEXTURES );
	return &gl_textures[texnum];
}

/*
=================
GL_TargetToString
=================
*/
const char *GL_TargetToString( GLenum target )
{
	switch( target )
	{
	case GL_TEXTURE_1D:
		return "1D";
	case GL_TEXTURE_2D:
		return "2D";
	case GL_TEXTURE_2D_MULTISAMPLE:
		return "2D Multisample";
	case GL_TEXTURE_3D:
		return "3D";
	// case GL_TEXTURE_CUBE_MAP_ARB:
	// 	return "Cube";
	// case GL_TEXTURE_2D_ARRAY_EXT:
	// 	return "Array";
	// case GL_TEXTURE_RECTANGLE_EXT:
	// 	return "Rect";
	}
	return "??";
}

/*
=================
GL_Bind
=================
*/
void GL_Bind( GLint tmu, GLenum texnum )
{
	gl_texture_t	*texture;
	GLuint		glTarget;

	// missed or invalid texture?
	if( texnum <= 0 || texnum >= MAX_TEXTURES )
	{
		if( texnum != 0 )
			gEngfuncs.Con_DPrintf( S_ERROR "%s: invalid texturenum %d\n", __func__, texnum );
		texnum = tr.defaultTexture;
	}
	if( tmu != GL_KEEP_UNIT )
		GL_SelectTexture( tmu );
	else tmu = glState.activeTMU;

	texture = &gl_textures[texnum];
	glTarget = texture->target;

	// if( glTarget == GL_TEXTURE_2D_ARRAY_EXT )
	// 	glTarget = GL_TEXTURE_2D;

	if( glState.currentTextureTargets[tmu] != glTarget )
	{
		GL_EnableTextureUnit( tmu, false );
		glState.currentTextureTargets[tmu] = glTarget;
		GL_EnableTextureUnit( tmu, true );
	}

	if( glState.currentTextures[tmu] == texture->texnum )
		return;

	// pglBindTexture( texture->target, texture->texnum );
	glState.currentTextures[tmu] = texture->texnum;
	glState.currentTexturesIndex[tmu] = texnum;
}

qboolean GL_TextureFilteringEnabled( const gl_texture_t *tex )
{
	if( FBitSet( tex->flags, TF_NEAREST ))
		return false;

	if( FBitSet( tex->flags, TF_DEPTHMAP ))
		return true;

	if( FBitSet( tex->flags, TF_NOMIPMAP ) || tex->numMips <= 1 )
	{
		if( FBitSet( tex->flags, TF_ATLAS_PAGE ))
			return gl_lightmap_nearest.value == 0.0f;

		if( FBitSet( tex->flags, TF_ALLOW_NEAREST ))
			return gl_texture_nearest.value == 0.0f;

		return true;
	}

	return gl_texture_nearest.value == 0.0f;
}

/*
=================
GL_ApplyTextureParams
=================
*/
void GL_ApplyTextureParams( gl_texture_t *tex )
{
	vec4_t	border = { 0.0f, 0.0f, 0.0f, 1.0f };
	qboolean nomipmap;

	if( !glw_state.initialized )
		return;

	Assert( tex != NULL );

	// multisample textures does not support any sampling state changing
	if( FBitSet( tex->flags, TF_MULTISAMPLE ))
		return;

	// set texture filter
	nomipmap = tex->numMips <= 1 || FBitSet( tex->flags, TF_NOMIPMAP|TF_DEPTHMAP );
	if( !GL_TextureFilteringEnabled( tex ))
	{
		// pglTexParameteri( tex->target, GL_TEXTURE_MIN_FILTER, nomipmap ? GL_NEAREST : GL_NEAREST_MIPMAP_NEAREST );
		// pglTexParameteri( tex->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}
	else
	{
		// pglTexParameteri( tex->target, GL_TEXTURE_MIN_FILTER, nomipmap ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR );
		// pglTexParameteri( tex->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}

	if( FBitSet( tex->flags, TF_DEPTHMAP ))
	{
		if( !FBitSet( tex->flags, TF_NOCOMPARE ))
		{
			// pglTexParameteri( tex->target, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB );
			// pglTexParameteri( tex->target, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL );
		}

		// if( FBitSet( tex->flags, TF_LUMINANCE ))
		// 	pglTexParameteri( tex->target, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE );
		// else pglTexParameteri( tex->target, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY );

		// allow max anisotropy as 1.0f on depth textures
		// if( GL_Support( GL_ANISOTROPY_EXT ))
		// 	pglTexParameterf( tex->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f );
	}
	else if( !FBitSet( tex->flags, TF_NOMIPMAP ) && tex->numMips > 1 )
	{
		// // set texture anisotropy if available
		// if( GL_Support( GL_ANISOTROPY_EXT ) && ( tex->numMips > 1 ) && !FBitSet( tex->flags, TF_ALPHACONTRAST ))
		// 	pglTexParameterf( tex->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_texture_anisotropy.value );

		// // set texture LOD bias if available
		// if( GL_Support( GL_TEXTURE_LOD_BIAS ) && ( tex->numMips > 1 ))
		// 	pglTexParameterf( tex->target, GL_TEXTURE_LOD_BIAS_EXT, gl_texture_lodbias.value );
	}

	// check if border is not supported
	if( FBitSet( tex->flags, TF_BORDER ) && !GL_Support( GL_CLAMP_TEXBORDER_EXT ))
	{
		ClearBits( tex->flags, TF_BORDER );
		SetBits( tex->flags, TF_CLAMP );
	}

	// only seamless cubemaps allows wrap 'clamp_to_border"
	// if( tex->target == GL_TEXTURE_CUBE_MAP_ARB && !GL_Support( GL_ARB_SEAMLESS_CUBEMAP ) && FBitSet( tex->flags, TF_BORDER ))
	// 	ClearBits( tex->flags, TF_BORDER );

	// set texture wrap
	if( FBitSet( tex->flags, TF_BORDER ))
	{
		// pglTexParameteri( tex->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );

		// if( tex->target != GL_TEXTURE_1D )
		// 	pglTexParameteri( tex->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

		// if( tex->target == GL_TEXTURE_3D || tex->target == GL_TEXTURE_CUBE_MAP_ARB )
		// 	pglTexParameteri( tex->target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );

		// pglTexParameterfv( tex->target, GL_TEXTURE_BORDER_COLOR, border );
	}
	else if( FBitSet( tex->flags, TF_CLAMP ))
	{
		if( GL_Support( GL_CLAMPTOEDGE_EXT ))
		{
			// pglTexParameteri( tex->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );

			// if( tex->target != GL_TEXTURE_1D )
			// 	pglTexParameteri( tex->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

			// if( tex->target == GL_TEXTURE_3D || tex->target == GL_TEXTURE_CUBE_MAP_ARB )
			// 	pglTexParameteri( tex->target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
		}
		else
		{
			// pglTexParameteri( tex->target, GL_TEXTURE_WRAP_S, GL_CLAMP );

			// if( tex->target != GL_TEXTURE_1D )
			// 	pglTexParameteri( tex->target, GL_TEXTURE_WRAP_T, GL_CLAMP );

			// if( tex->target == GL_TEXTURE_3D || tex->target == GL_TEXTURE_CUBE_MAP_ARB )
			// 	pglTexParameteri( tex->target, GL_TEXTURE_WRAP_R, GL_CLAMP );
		}
	}
	else
	{
		// pglTexParameteri( tex->target, GL_TEXTURE_WRAP_S, GL_REPEAT );

		// if( tex->target != GL_TEXTURE_1D )
		// 	pglTexParameteri( tex->target, GL_TEXTURE_WRAP_T, GL_REPEAT );

		// if( tex->target == GL_TEXTURE_3D || tex->target == GL_TEXTURE_CUBE_MAP_ARB )
		// 	pglTexParameteri( tex->target, GL_TEXTURE_WRAP_R, GL_REPEAT );
	}
}

/*
=================
GL_UpdateTextureParams
=================
*/
static void GL_UpdateTextureParams( int iTexture )
{
	gl_texture_t	*tex = &gl_textures[iTexture];
	qboolean nomipmap;

	Assert( tex != NULL );

	if( !tex->texnum ) return; // free slot

	GL_Bind( XASH_TEXTURE0, iTexture );

	// set texture anisotropy if available
	// if( GL_Support( GL_ANISOTROPY_EXT ) && ( tex->numMips > 1 ) && !FBitSet( tex->flags, TF_DEPTHMAP|TF_ALPHACONTRAST ))
	// 	pglTexParameterf( tex->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_texture_anisotropy.value );

	// // set texture LOD bias if available
	// if( GL_Support( GL_TEXTURE_LOD_BIAS ) && ( tex->numMips > 1 ) && !FBitSet( tex->flags, TF_DEPTHMAP ))
	// 	pglTexParameterf( tex->target, GL_TEXTURE_LOD_BIAS_EXT, gl_texture_lodbias.value );

	// nomipmap = tex->numMips <= 1 || FBitSet( tex->flags, TF_NOMIPMAP|TF_DEPTHMAP );

	// if( !GL_TextureFilteringEnabled( tex ))
	// {
	// 	pglTexParameteri( tex->target, GL_TEXTURE_MIN_FILTER, nomipmap ? GL_NEAREST : GL_NEAREST_MIPMAP_NEAREST );
	// 	pglTexParameteri( tex->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	// }
	// else
	// {
	// 	pglTexParameteri( tex->target, GL_TEXTURE_MIN_FILTER, nomipmap ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR );
	// 	pglTexParameteri( tex->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	// }
}

/*
=================
R_SetTextureParameters
=================
*/
void R_SetTextureParameters( void )
{
	int	i;

	if( GL_Support( GL_ANISOTROPY_EXT ))
	{
		if( gl_texture_anisotropy.value > glConfig.max_texture_anisotropy )
			gEngfuncs.Cvar_SetValue( "gl_anisotropy", glConfig.max_texture_anisotropy );
		else if( gl_texture_anisotropy.value < 1.0f )
			gEngfuncs.Cvar_SetValue( "gl_anisotropy", 1.0f );
	}

	if( GL_Support( GL_TEXTURE_LOD_BIAS ))
	{
		if( gl_texture_lodbias.value < -glConfig.max_texture_lod_bias )
			gEngfuncs.Cvar_SetValue( "gl_texture_lodbias", -glConfig.max_texture_lod_bias );
		else if( gl_texture_lodbias.value > glConfig.max_texture_lod_bias )
			gEngfuncs.Cvar_SetValue( "gl_texture_lodbias", glConfig.max_texture_lod_bias );
	}

	ClearBits( gl_texture_anisotropy.flags, FCVAR_CHANGED );
	ClearBits( gl_texture_lodbias.flags, FCVAR_CHANGED );
	ClearBits( gl_texture_nearest.flags, FCVAR_CHANGED );
	ClearBits( gl_lightmap_nearest.flags, FCVAR_CHANGED );

	// change all the existing mipmapped texture objects
	for( i = 0; i < gl_numTextures; i++ )
		GL_UpdateTextureParams( i );
}

/*
================
GL_CalcTextureSamples
================
*/
static int GL_CalcTextureSamples( int flags )
{
	if( FBitSet( flags, IMAGE_HAS_COLOR ))
		return FBitSet( flags, IMAGE_HAS_ALPHA ) ? 4 : 3;
	return FBitSet( flags, IMAGE_HAS_ALPHA ) ? 2 : 1;
}

/*
==================
GL_CalcImageSize
==================
*/
static size_t GL_CalcImageSize( pixformat_t format, int width, int height, int depth )
{
	size_t	size = 0;

	// check the depth error
	depth = Q_max( 1, depth );

	switch( format )
	{
	case PF_LUMINANCE:
		size = width * height * depth;
		break;
	case PF_RGB_24:
	case PF_BGR_24:
		size = width * height * depth * 3;
		break;
	case PF_BGRA_32:
	case PF_RGBA_32:
		size = width * height * depth * 4;
		break;
	case PF_DXT1:
		size = (((width + 3) >> 2) * ((height + 3) >> 2) * 8) * depth;
		break;
	case PF_DXT3:
	case PF_DXT5:
	case PF_BC6H_SIGNED:
	case PF_BC6H_UNSIGNED:
	case PF_BC7_UNORM:
	case PF_BC7_SRGB:
	case PF_ATI2:
		size = (((width + 3) >> 2) * ((height + 3) >> 2) * 16) * depth;
		break;
	}

	return size;
}

/*
==================
GL_CalcTextureSize
==================
*/
static size_t GL_CalcTextureSize( GLenum format, int width, int height, int depth )
{
	size_t	size = 0;

	// check the depth error
	depth = Q_max( 1, depth );

	switch( format )
	{
	// case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	// case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	// 	size = (((width + 3) >> 2) * ((height + 3) >> 2) * 8) * depth;
	// 	break;
	// case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	// case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	// case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
	// case GL_COMPRESSED_LUMINANCE_ALPHA_ARB:
	// case GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI:
	// case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
	// case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
	// case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
	// case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
	// 	size = (((width + 3) >> 2) * ((height + 3) >> 2) * 16) * depth;
	// 	break;
	case GL_RGBA8:
	case GL_RGBA:
		size = width * height * depth * 4;
		break;
	case GL_RGB8:
	case GL_RGB:
		size = width * height * depth * 3;
		break;
	case GL_RGB5:
		size = (width * height * depth * 3) / 2;
		break;
	case GL_RGBA4:
		size = (width * height * depth * 4) / 2;
		break;
	// case GL_INTENSITY:
	// case GL_LUMINANCE:
	// case GL_INTENSITY8:
	// case GL_LUMINANCE8:
	// 	size = (width * height * depth);
	// 	break;
	// case GL_LUMINANCE_ALPHA:
	// case GL_LUMINANCE8_ALPHA8:
	// 	size = width * height * depth * 2;
	// 	break;
	case GL_R8:
		size = width * height * depth;
		break;
	case GL_RG8:
		size = width * height * depth * 2;
		break;
	case GL_R16:
		size = width * height * depth * 2;
		break;
	case GL_RG16:
		size = width * height * depth * 4;
		break;
	case GL_R16F:
	// case GL_LUMINANCE16F_ARB:
	// 	size = width * height * depth * 2;	// half-floats
	// 	break;
	case GL_R32F:
	// case GL_LUMINANCE32F_ARB:
	// 	size = width * height * depth * 4;
	// 	break;
	case GL_RG16F:
	// case GL_LUMINANCE_ALPHA16F_ARB:
	// 	size = width * height * depth * 4;
	// 	break;
	case GL_RG32F:
	// case GL_LUMINANCE_ALPHA32F_ARB:
	// 	size = width * height * depth * 8;
	// 	break;
	// case GL_RGB16F_ARB:
	// 	size = width * height * depth * 6;
	// 	break;
	// case GL_RGBA16F_ARB:
	// 	size = width * height * depth * 8;
	// 	break;
	// case GL_RGB32F_ARB:
	// 	size = width * height * depth * 12;
	// 	break;
	// case GL_RGBA32F_ARB:
	// 	size = width * height * depth * 16;
	// 	break;
	case GL_DEPTH_COMPONENT16:
		size = width * height * depth * 2;
		break;
	case GL_DEPTH_COMPONENT24:
		size = width * height * depth * 3;
		break;
	case GL_DEPTH_COMPONENT32F:
		size = width * height * depth * 4;
		break;
	default:
		gEngfuncs.Host_Error( "%s: bad texture internal format (%u)\n", __func__, format );
		break;
	}

	return size;
}

static int GL_CalcMipmapCount( gl_texture_t *tex, qboolean haveBuffer )
{
	int	width, height;
	int	mipcount;

	Assert( tex != NULL );

	if( !haveBuffer || tex->target == GL_TEXTURE_3D )
		return 1;

	// generate mip-levels by user request
	if( FBitSet( tex->flags, TF_NOMIPMAP ))
		return 1;

	// mip-maps can't exceeds 16
	for( mipcount = 0; mipcount < 16; mipcount++ )
	{
		width = Q_max( 1, ( tex->width >> mipcount ));
		height = Q_max( 1, ( tex->height >> mipcount ));
		if( width == 1 && height == 1 )
			break;
	}

	return mipcount + 1;
}

/*
================
GL_SetTextureDimensions
================
*/
static void GL_SetTextureDimensions( gl_texture_t *tex, int width, int height, int depth )
{
	int	maxTextureSize = 0;
	int	maxDepthSize = 1;

	Assert( tex != NULL );

	switch( tex->target )
	{
	case GL_TEXTURE_1D:
	case GL_TEXTURE_2D:
	case GL_TEXTURE_2D_MULTISAMPLE:
		maxTextureSize = glConfig.max_2d_texture_size;
		break;
	// case GL_TEXTURE_2D_ARRAY_EXT:
	// 	maxDepthSize = glConfig.max_2d_texture_layers;
	// 	maxTextureSize = glConfig.max_2d_texture_size;
	// 	break;
	// case GL_TEXTURE_RECTANGLE_EXT:
	// 	maxTextureSize = glConfig.max_2d_rectangle_size;
	// 	break;
	// case GL_TEXTURE_CUBE_MAP_ARB:
	// 	maxTextureSize = glConfig.max_cubemap_size;
	// 	break;
	case GL_TEXTURE_3D:
		maxDepthSize = glConfig.max_3d_texture_size;
		maxTextureSize = glConfig.max_3d_texture_size;
		break;
	default:
		Assert( false );
	}

	// store original sizes
	tex->srcWidth = width;
	tex->srcHeight = height;

	if( !GL_Support( GL_ARB_TEXTURE_NPOT_EXT ))
	{
		int	step = (int)gl_round_down.value;
		int	scaled_width, scaled_height;

		for( scaled_width = 1; scaled_width < width; scaled_width <<= 1 );

		if( step > 0 && width < scaled_width && ( step == 1 || ( scaled_width - width ) > ( scaled_width >> step )))
			scaled_width >>= 1;

		for( scaled_height = 1; scaled_height < height; scaled_height <<= 1 );

		if( step > 0 && height < scaled_height && ( step == 1 || ( scaled_height - height ) > ( scaled_height >> step )))
			scaled_height >>= 1;

		width = scaled_width;
		height = scaled_height;
	}

	if( width > maxTextureSize || height > maxTextureSize || depth > maxDepthSize )
	{
		if( tex->target == GL_TEXTURE_1D )
		{
			while( width > maxTextureSize )
				width >>= 1;
		}
		// else if( tex->target == GL_TEXTURE_3D || tex->target == GL_TEXTURE_2D_ARRAY_EXT )
		// {
		// 	while( width > maxTextureSize || height > maxTextureSize || depth > maxDepthSize )
		// 	{
		// 		width >>= 1;
		// 		height >>= 1;
		// 		depth >>= 1;
		// 	}
		// }
		else // all remaining cases
		{
			while( width > maxTextureSize || height > maxTextureSize )
			{
				width >>= 1;
				height >>= 1;
			}
		}
	}

	// set the texture dimensions
	tex->width = Q_max( 1, width );
	tex->height = Q_max( 1, height );
	tex->depth = Q_max( 1, depth );
}

/*
===============
GL_SetTextureTarget
===============
*/
static void GL_SetTextureTarget( gl_texture_t *tex, rgbdata_t *pic )
{
}

/*
===============
GL_SetTextureFormat
===============
*/
static void GL_SetTextureFormat( gl_texture_t *tex, pixformat_t format, int channelMask )
{
}

/*
=================
GL_ResampleTexture

Assume input buffer is RGBA
=================
*/
byte *GL_ResampleTexture( const byte *source, int inWidth, int inHeight, int outWidth, int outHeight, qboolean isNormalMap )
{
	uint		frac, fracStep;
	uint		*in = (uint *)source;
	uint		p1[0x1000], p2[0x1000];
	byte		*pix1, *pix2, *pix3, *pix4;
	uint		*out, *inRow1, *inRow2;
	static byte	*scaledImage = NULL;	// pointer to a scaled image
	vec3_t		normal;
	int		i, x, y;

	if( !source ) return NULL;

	scaledImage = Mem_Realloc( r_temppool, scaledImage, outWidth * outHeight * 4 );
	fracStep = inWidth * 0x10000 / outWidth;
	out = (uint *)scaledImage;

	frac = fracStep >> 2;
	for( i = 0; i < outWidth; i++ )
	{
		p1[i] = 4 * (frac >> 16);
		frac += fracStep;
	}

	frac = (fracStep >> 2) * 3;
	for( i = 0; i < outWidth; i++ )
	{
		p2[i] = 4 * (frac >> 16);
		frac += fracStep;
	}

	if( isNormalMap )
	{
		for( y = 0; y < outHeight; y++, out += outWidth )
		{
			inRow1 = in + inWidth * (int)(((float)y + 0.25f) * inHeight / outHeight);
			inRow2 = in + inWidth * (int)(((float)y + 0.75f) * inHeight / outHeight);

			for( x = 0; x < outWidth; x++ )
			{
				pix1 = (byte *)inRow1 + p1[x];
				pix2 = (byte *)inRow1 + p2[x];
				pix3 = (byte *)inRow2 + p1[x];
				pix4 = (byte *)inRow2 + p2[x];

				normal[0] = MAKE_SIGNED( pix1[0] ) + MAKE_SIGNED( pix2[0] ) + MAKE_SIGNED( pix3[0] ) + MAKE_SIGNED( pix4[0] );
				normal[1] = MAKE_SIGNED( pix1[1] ) + MAKE_SIGNED( pix2[1] ) + MAKE_SIGNED( pix3[1] ) + MAKE_SIGNED( pix4[1] );
				normal[2] = MAKE_SIGNED( pix1[2] ) + MAKE_SIGNED( pix2[2] ) + MAKE_SIGNED( pix3[2] ) + MAKE_SIGNED( pix4[2] );

				if( !VectorNormalizeLength( normal ))
					VectorSet( normal, 0.5f, 0.5f, 1.0f );

				((byte *)(out+x))[0] = 128 + (byte)(127.0f * normal[0]);
				((byte *)(out+x))[1] = 128 + (byte)(127.0f * normal[1]);
				((byte *)(out+x))[2] = 128 + (byte)(127.0f * normal[2]);
				((byte *)(out+x))[3] = 255;
			}
		}
	}
	else
	{
		for( y = 0; y < outHeight; y++, out += outWidth )
		{
			inRow1 = in + inWidth * (int)(((float)y + 0.25f) * inHeight / outHeight);
			inRow2 = in + inWidth * (int)(((float)y + 0.75f) * inHeight / outHeight);

			for( x = 0; x < outWidth; x++ )
			{
				pix1 = (byte *)inRow1 + p1[x];
				pix2 = (byte *)inRow1 + p2[x];
				pix3 = (byte *)inRow2 + p1[x];
				pix4 = (byte *)inRow2 + p2[x];

				((byte *)(out+x))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0]) >> 2;
				((byte *)(out+x))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1]) >> 2;
				((byte *)(out+x))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2]) >> 2;
				((byte *)(out+x))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3]) >> 2;
			}
		}
	}

	return scaledImage;
}

/*
=================
GL_BoxFilter3x3

box filter 3x3
=================
*/
static void GL_BoxFilter3x3( byte *out, const byte *in, int w, int h, int x, int y )
{
	int		r = 0, g = 0, b = 0, a = 0;
	int		count = 0, acount = 0;
	int		i, j, u, v;
	const byte	*pixel;

	for( i = 0; i < 3; i++ )
	{
		u = ( i - 1 ) + x;

		for( j = 0; j < 3; j++ )
		{
			v = ( j - 1 ) + y;

			if( u >= 0 && u < w && v >= 0 && v < h )
			{
				pixel = &in[( u + v * w ) * 4];

				if( pixel[3] != 0 )
				{
					r += pixel[0];
					g += pixel[1];
					b += pixel[2];
					a += pixel[3];
					acount++;
				}
			}
		}
	}

	if(  acount == 0 )
		acount = 1;

	out[0] = r / acount;
	out[1] = g / acount;
	out[2] = b / acount;
//	out[3] = (int)( SimpleSpline( ( a / 12.0f ) / 255.0f ) * 255 );
}

/*
=================
GL_ApplyFilter

Apply box-filter to 1-bit alpha
=================
*/
static byte *GL_ApplyFilter( const byte *source, int width, int height )
{
	byte	*in = (byte *)source;
	byte	*out = (byte *)source;
	int	i;

	if( ENGINE_GET_PARM( PARM_QUAKE_COMPATIBLE ) || glConfig.max_multisamples > 1 )
		return in;

	for( i = 0; source && i < width * height; i++, in += 4 )
	{
		if( in[0] == 0 && in[1] == 0 && in[2] == 0 && in[3] == 0 )
			GL_BoxFilter3x3( in, source, width, height, i % width, i / width );
	}

	return out;
}

/*
=================
GL_BuildMipMap

Operates in place, quartering the size of the texture
=================
*/
static void GL_BuildMipMap( byte *in, int srcWidth, int srcHeight, int srcDepth, int flags )
{
	byte	*out = in;
	int	instride = ALIGN( srcWidth * 4, 1 );
	int	mipWidth, mipHeight, outpadding;
	int	row, x, y, z;
	vec3_t	normal;

	if( !in ) return;

	mipWidth = Q_max( 1, ( srcWidth >> 1 ));
	mipHeight = Q_max( 1, ( srcHeight >> 1 ));
	outpadding = ALIGN( mipWidth * 4, 1 ) - mipWidth * 4;
	row = srcWidth << 2;

	if( FBitSet( flags, TF_ALPHACONTRAST ))
	{
		memset( in, mipWidth, mipWidth * mipHeight * 4 );
		return;
	}

	// move through all layers
	for( z = 0; z < srcDepth; z++ )
	{
		if( FBitSet( flags, TF_NORMALMAP ))
		{
			for( y = 0; y < mipHeight; y++, in += instride * 2, out += outpadding )
			{
				byte *next = ((( y << 1 ) + 1 ) < srcHeight ) ? ( in + instride ) : in;
				for( x = 0, row = 0; x < mipWidth; x++, row += 8, out += 4 )
				{
					if((( x << 1 ) + 1 ) < srcWidth )
					{
						normal[0] = MAKE_SIGNED( in[row+0] ) + MAKE_SIGNED( in[row+4] )
						+ MAKE_SIGNED( next[row+0] ) + MAKE_SIGNED( next[row+4] );
						normal[1] = MAKE_SIGNED( in[row+1] ) + MAKE_SIGNED( in[row+5] )
						+ MAKE_SIGNED( next[row+1] ) + MAKE_SIGNED( next[row+5] );
						normal[2] = MAKE_SIGNED( in[row+2] ) + MAKE_SIGNED( in[row+6] )
						+ MAKE_SIGNED( next[row+2] ) + MAKE_SIGNED( next[row+6] );
					}
					else
					{
						normal[0] = MAKE_SIGNED( in[row+0] ) + MAKE_SIGNED( next[row+0] );
						normal[1] = MAKE_SIGNED( in[row+1] ) + MAKE_SIGNED( next[row+1] );
						normal[2] = MAKE_SIGNED( in[row+2] ) + MAKE_SIGNED( next[row+2] );
					}

					if( !VectorNormalizeLength( normal ))
						VectorSet( normal, 0.5f, 0.5f, 1.0f );

					out[0] = 128 + (byte)(127.0f * normal[0]);
					out[1] = 128 + (byte)(127.0f * normal[1]);
					out[2] = 128 + (byte)(127.0f * normal[2]);
					out[3] = 255;
				}
			}
		}
		else
		{
			for( y = 0; y < mipHeight; y++, in += instride * 2, out += outpadding )
			{
				byte *next = ((( y << 1 ) + 1 ) < srcHeight ) ? ( in + instride ) : in;
				for( x = 0, row = 0; x < mipWidth; x++, row += 8, out += 4 )
				{
					if((( x << 1 ) + 1 ) < srcWidth )
					{
						out[0] = (in[row+0] + in[row+4] + next[row+0] + next[row+4]) >> 2;
						out[1] = (in[row+1] + in[row+5] + next[row+1] + next[row+5]) >> 2;
						out[2] = (in[row+2] + in[row+6] + next[row+2] + next[row+6]) >> 2;
						out[3] = (in[row+3] + in[row+7] + next[row+3] + next[row+7]) >> 2;
					}
					else
					{
						out[0] = (in[row+0] + next[row+0]) >> 1;
						out[1] = (in[row+1] + next[row+1]) >> 1;
						out[2] = (in[row+2] + next[row+2]) >> 1;
						out[3] = (in[row+3] + next[row+3]) >> 1;
					}
				}
			}
		}
	}
}

static void GL_TextureImageRAW( gl_texture_t *tex, GLint side, GLint level, GLint width, GLint height, GLint depth, GLint type, const void *data )
{
}

static void GL_TextureImageCompressed( gl_texture_t *tex, GLint side, GLint level, GLint width, GLint height, GLint depth, size_t size, const void *data )
{
}

/*
===============
GL_CheckTexImageError

show GL-errors on load images
===============
*/
static void GL_CheckTexImageError( gl_texture_t *tex )
{
}

/*
===============
GL_UploadTexture

upload texture into video memory
===============
*/
static qboolean GL_UploadTexture( gl_texture_t *tex, rgbdata_t *pic )
{
}

/*
===============
GL_ProcessImage

do specified actions on pixels
===============
*/
static void GL_ProcessImage( gl_texture_t *tex, rgbdata_t *pic )
{
	uint	img_flags = 0;

	// force upload texture as RGB or RGBA (detail textures requires this)
	if( tex->flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;
	if( pic->flags & IMAGE_HAS_ALPHA ) tex->flags |= TF_HAS_ALPHA;

	tex->encode = pic->encode; // share encode method

	if( ImageCompressed( pic->type ))
	{
		if( !pic->numMips )
			tex->flags |= TF_NOMIPMAP; // disable mipmapping by user request

		// clear all the unsupported flags
		tex->flags &= ~TF_KEEP_SOURCE;
	}
	else
	{
		// copy flag about luma pixels
		if( pic->flags & IMAGE_HAS_LUMA )
			tex->flags |= TF_HAS_LUMA;

		if( pic->flags & IMAGE_QUAKEPAL )
			tex->flags |= TF_QUAKEPAL;

		// create luma texture from quake texture
		if( tex->flags & TF_MAKELUMA )
		{
			img_flags |= IMAGE_MAKE_LUMA;
			tex->flags &= ~TF_MAKELUMA;
		}

		if( !FBitSet( tex->flags, TF_IMG_UPLOADED ) && FBitSet( tex->flags, TF_KEEP_SOURCE ))
			tex->original = gEngfuncs.FS_CopyImage( pic ); // because current pic will be expanded to rgba

		// we need to expand image into RGBA buffer
		if( pic->type == PF_INDEXED_24 || pic->type == PF_INDEXED_32 )
			img_flags |= IMAGE_FORCE_RGBA;

		// processing image before uploading (force to rgba, make luma etc)
		if( pic->buffer ) gEngfuncs.Image_Process( &pic, 0, 0, img_flags, 0 );

		if( FBitSet( tex->flags, TF_LUMINANCE ))
			ClearBits( pic->flags, IMAGE_HAS_COLOR );
	}
}

/*
================
GL_CheckTexName
================
*/
static qboolean GL_CheckTexName( const char *name )
{
	int len;

	if( !COM_CheckString( name ))
		return false;

	len = Q_strlen( name );

	// because multi-layered textures can exceed name string
	if( len >= sizeof( gl_textures->name ))
	{
		gEngfuncs.Con_Printf( S_ERROR "LoadTexture: too long name %s (%d)\n", name, len );
		return false;
	}

	return true;
}

/*
================
GL_TextureForName
================
*/
static gl_texture_t *GL_TextureForName( const char *name )
{
	gl_texture_t	*tex;
	uint		hash;

	// find the texture in array
	hash = COM_HashKey( name, TEXTURES_HASH_SIZE );

	for( tex = gl_texturesHashTable[hash]; tex != NULL; tex = tex->nextHash )
	{
		if( !Q_stricmp( tex->name, name ))
			return tex;
	}

	return NULL;
}

/*
================
GL_AllocTexture
================
*/
static gl_texture_t *GL_AllocTexture( const char *name, texFlags_t flags )
{
}

/*
================
GL_DeleteTexture
================
*/
static void GL_DeleteTexture( gl_texture_t *tex )
{
}

/*
================
GL_UpdateTexSize

recalc image room
================
*/
void GL_UpdateTexSize( int texnum, int width, int height, int depth )
{
	int		i, j, texsize;
	int		numSides;
	gl_texture_t	*tex;

	if( texnum <= 0 || texnum >= MAX_TEXTURES )
		return;

	tex = &gl_textures[texnum];
	numSides = FBitSet( tex->flags, TF_CUBEMAP ) ? 6 : 1;
	GL_SetTextureDimensions( tex, width, height, depth );
	tex->size = 0; // recompute now

	for( i = 0; i < numSides; i++ )
	{
		for( j = 0; j < Q_max( 1, tex->numMips ); j++ )
		{
			width = Q_max( 1, ( tex->width >> j ));
			height = Q_max( 1, ( tex->height >> j ));
			texsize = GL_CalcTextureSize( tex->format, width, height, tex->depth );
			tex->size += texsize;
		}
	}
}

/*
================
GL_LoadTexture
================
*/
int GL_LoadTexture( const char *name, const byte *buf, size_t size, int flags )
{
	gl_texture_t	*tex;
	rgbdata_t		*pic;
	uint		picFlags = 0;

	if( !GL_CheckTexName( name ))
		return 0;

	// see if already loaded
	if(( tex = GL_TextureForName( name )))
		return (tex - gl_textures);

	if( FBitSet( flags, TF_NOFLIP_TGA ))
		SetBits( picFlags, IL_DONTFLIP_TGA );

	if( FBitSet( flags, TF_KEEP_SOURCE ) && !FBitSet( flags, TF_EXPAND_SOURCE ))
		SetBits( picFlags, IL_KEEP_8BIT );

	// set some image flags
	gEngfuncs.Image_SetForceFlags( picFlags );

	pic = gEngfuncs.FS_LoadImage( name, buf, size );
	if( !pic ) return 0; // couldn't loading image

	// allocate the new one
	tex = GL_AllocTexture( name, flags );
	GL_ProcessImage( tex, pic );

	if( !GL_UploadTexture( tex, pic ))
	{
		memset( tex, 0, sizeof( gl_texture_t ));
		gEngfuncs.FS_FreeImage( pic ); // release source texture
		return 0;
	}

	GL_ApplyTextureParams( tex ); // update texture filter, wrap etc
	gEngfuncs.FS_FreeImage( pic ); // release source texture

	// NOTE: always return texnum as index in array or engine will stop work !!!
	return tex - gl_textures;
}

/*
================
GL_LoadTextureArray
================
*/
int GL_LoadTextureArray( const char **names, int flags )
{
}

/*
================
GL_LoadTextureFromBuffer
================
*/
int GL_LoadTextureFromBuffer( const char *name, rgbdata_t *pic, texFlags_t flags, qboolean update )
{
	gl_texture_t	*tex;

	if( !GL_CheckTexName( name ))
		return 0;

	// see if already loaded
	if(( tex = GL_TextureForName( name )) && !update )
		return (tex - gl_textures);

	// couldn't loading image
	if( !pic ) return 0;

	if( update )
	{
		if( tex == NULL )
			gEngfuncs.Host_Error( "%s: couldn't find texture %s for update\n", __func__, name );
		SetBits( tex->flags, flags );
	}
	else
	{
		// allocate the new one
		tex = GL_AllocTexture( name, flags );
	}

	GL_ProcessImage( tex, pic );

	if( !GL_UploadTexture( tex, pic ))
	{
		memset( tex, 0, sizeof( gl_texture_t ));
		return 0;
	}

	GL_ApplyTextureParams( tex ); // update texture filter, wrap etc
	return (tex - gl_textures);
}

/*
================
GL_CreateTexture

creates texture from buffer
================
*/
int GL_CreateTexture( const char *name, int width, int height, const void *buffer, texFlags_t flags )
{
	qboolean	update = FBitSet( flags, TF_UPDATE ) ? true : false;
	int	datasize = 1;
	rgbdata_t	r_empty;

	if( FBitSet( flags, TF_ARB_16BIT ))
		datasize = 2;
	else if( FBitSet( flags, TF_ARB_FLOAT ))
		datasize = 4;

	ClearBits( flags, TF_UPDATE );
	memset( &r_empty, 0, sizeof( r_empty ));
	r_empty.width = width;
	r_empty.height = height;
	r_empty.type = PF_RGBA_32;
	r_empty.size = r_empty.width * r_empty.height * datasize * 4;
	r_empty.buffer = (byte *)buffer;

	// clear invalid combinations
	ClearBits( flags, TF_TEXTURE_3D );

	// if image not luminance and not alphacontrast it will have color
	if( !FBitSet( flags, TF_LUMINANCE ) && !FBitSet( flags, TF_ALPHACONTRAST ))
		SetBits( r_empty.flags, IMAGE_HAS_COLOR );

	if( FBitSet( flags, TF_HAS_ALPHA ))
		SetBits( r_empty.flags, IMAGE_HAS_ALPHA );

	if( FBitSet( flags, TF_CUBEMAP ))
	{
		if( !GL_Support( GL_TEXTURE_CUBEMAP_EXT ))
			return 0;
		SetBits( r_empty.flags, IMAGE_CUBEMAP );
		r_empty.size *= 6;
	}

	return GL_LoadTextureFromBuffer( name, &r_empty, flags, update );
}

/*
================
GL_CreateTextureArray

creates texture array from buffer
================
*/
int GL_CreateTextureArray( const char *name, int width, int height, int depth, const void *buffer, texFlags_t flags )
{
	rgbdata_t	r_empty;

	memset( &r_empty, 0, sizeof( r_empty ));
	r_empty.width = Q_max( width, 1 );
	r_empty.height = Q_max( height, 1 );
	r_empty.depth = Q_max( depth, 1 );
	r_empty.type = PF_RGBA_32;
	r_empty.size = r_empty.width * r_empty.height * r_empty.depth * 4;
	r_empty.buffer = (byte *)buffer;

	// clear invalid combinations
	ClearBits( flags, TF_CUBEMAP|TF_SKYSIDE|TF_HAS_LUMA|TF_MAKELUMA|TF_ALPHACONTRAST );

	// if image not luminance it will have color
	if( !FBitSet( flags, TF_LUMINANCE ))
		SetBits( r_empty.flags, IMAGE_HAS_COLOR );

	if( FBitSet( flags, TF_HAS_ALPHA ))
		SetBits( r_empty.flags, IMAGE_HAS_ALPHA );

	if( FBitSet( flags, TF_TEXTURE_3D ))
	{
		if( !GL_Support( GL_TEXTURE_3D_EXT ))
			return 0;
	}
	else
	{
		if( !GL_Support( GL_TEXTURE_ARRAY_EXT ))
			return 0;
		SetBits( r_empty.flags, IMAGE_MULTILAYER );
	}

	return GL_LoadTextureInternal( name, &r_empty, flags );
}

/*
================
GL_FindTexture
================
*/
int GL_FindTexture( const char *name )
{
	gl_texture_t	*tex;

	if( !GL_CheckTexName( name ))
		return 0;

	// see if already loaded
	if(( tex = GL_TextureForName( name )))
		return (tex - gl_textures);

	return 0;
}

/*
================
GL_FreeTexture
================
*/
void GL_FreeTexture( GLenum texnum )
{
	// number 0 it's already freed
	if( texnum <= 0 ) return;

	GL_DeleteTexture( &gl_textures[texnum] );
}

/*
================
GL_ProcessTexture
================
*/
void GL_ProcessTexture( int texnum, float gamma, int topColor, int bottomColor )
{
	gl_texture_t	*image;
	rgbdata_t		*pic;
	int		flags = 0;

	if( texnum <= 0 || texnum >= MAX_TEXTURES )
		return; // missed image
	image = &gl_textures[texnum];

	// select mode
	if( gamma != -1.0f )
	{
		flags = IMAGE_LIGHTGAMMA;
	}
	else if( topColor != -1 && bottomColor != -1 )
	{
		flags = IMAGE_REMAP;
	}
	else
	{
		gEngfuncs.Con_Printf( S_ERROR "%s: bad operation for %s\n", __func__, image->name );
		return;
	}

	if( !image->original )
	{
		gEngfuncs.Con_Printf( S_ERROR "%s: no input data for %s\n", __func__, image->name );
		return;
	}

	if( ImageCompressed( image->original->type ))
	{
		gEngfuncs.Con_Printf( S_ERROR "%s: can't process compressed texture %s\n", __func__, image->name );
		return;
	}

	// all the operations makes over the image copy not an original
	pic = gEngfuncs.FS_CopyImage( image->original );

	// we need to expand image into RGBA buffer
	if( pic->type == PF_INDEXED_24 || pic->type == PF_INDEXED_32 )
		flags |= IMAGE_FORCE_RGBA;

	gEngfuncs.Image_Process( &pic, topColor, bottomColor, flags, 0.0f );

	GL_UploadTexture( image, pic );
	GL_ApplyTextureParams( image ); // update texture filter, wrap etc

	gEngfuncs.FS_FreeImage( pic );
}

/*
================
GL_TexMemory

return size of all uploaded textures
================
*/
int GL_TexMemory( void )
{
	int	i, total = 0;

	for( i = 0; i < gl_numTextures; i++ )
		total += gl_textures[i].size;

	return total;
}

/*
==============================================================================

INTERNAL TEXTURES

==============================================================================
*/
/*
==================
GL_FakeImage
==================
*/
static rgbdata_t *GL_FakeImage( int width, int height, int depth, int flags )
{
	static byte	data2D[1024]; // 16x16x4
	static rgbdata_t	r_image;

	// also use this for bad textures, but without alpha
	r_image.width = Q_max( 1, width );
	r_image.height = Q_max( 1, height );
	r_image.depth = Q_max( 1, depth );
	r_image.flags = flags;
	r_image.type = PF_RGBA_32;
	r_image.size = r_image.width * r_image.height * r_image.depth * 4;
	r_image.buffer = (r_image.size > sizeof( data2D )) ? NULL : data2D;
	r_image.palette = NULL;
	r_image.numMips = 1;
	r_image.encode = 0;

	if( FBitSet( r_image.flags, IMAGE_CUBEMAP ))
		r_image.size *= 6;
	memset( data2D, 0xFF, sizeof( data2D ));

	return &r_image;
}

/*
==================
R_InitDlightTexture
==================
*/
void R_InitDlightTexture( void )
{
	rgbdata_t r_image =
	{
		.width = BLOCK_SIZE,
		.height = BLOCK_SIZE,
		.type = PF_RGBA_32,
		.flags = IMAGE_HAS_COLOR,
		.size = r_image.width * r_image.height * 4
	};
	qboolean update = false;

	if( tr.dlightTexture != 0 )
		update = true;

	tr.dlightTexture = GL_LoadTextureFromBuffer( "*dlight", &r_image, TF_NOMIPMAP|TF_CLAMP|TF_ATLAS_PAGE, update );
}

/*
==================
GL_CreateInternalTextures
==================
*/
static void GL_CreateInternalTextures( void )
{
	int	dx2, dy, d;
	int	x, y;
	rgbdata_t	*pic;

	// emo-texture from quake1
	pic = GL_FakeImage( 16, 16, 1, IMAGE_HAS_COLOR );

	for( y = 0; y < 16; y++ )
	{
		for( x = 0; x < 16; x++ )
		{
			if(( y < 8 ) ^ ( x < 8 ))
				((uint *)pic->buffer)[y*16+x] = 0xFFFF00FF;
			else ((uint *)pic->buffer)[y*16+x] = 0xFF000000;
		}
	}

	tr.defaultTexture = GL_LoadTextureInternal( REF_DEFAULT_TEXTURE, pic, TF_COLORMAP );

	// particle texture from quake1
	pic = GL_FakeImage( 8, 8, 1, IMAGE_HAS_COLOR|IMAGE_HAS_ALPHA );

	for( x = 0; x < 8; x++ )
	{
		for( y = 0; y < 8; y++ )
		{
			if( dottexture[x][y] )
				pic->buffer[( y * 8 + x ) * 4 + 3] = 255;
			else pic->buffer[( y * 8 + x ) * 4 + 3] = 0;
		}
	}

	tr.particleTexture = GL_LoadTextureInternal( REF_PARTICLE_TEXTURE, pic, TF_CLAMP );

	// white texture
	pic = GL_FakeImage( 4, 4, 1, IMAGE_HAS_COLOR );
	for( x = 0; x < 16; x++ )
		((uint *)pic->buffer)[x] = 0xFFFFFFFF;
	tr.whiteTexture = GL_LoadTextureInternal( REF_WHITE_TEXTURE, pic, TF_COLORMAP );

	// gray texture
	pic = GL_FakeImage( 4, 4, 1, IMAGE_HAS_COLOR );
	for( x = 0; x < 16; x++ )
		((uint *)pic->buffer)[x] = 0xFF7F7F7F;
	tr.grayTexture = GL_LoadTextureInternal( REF_GRAY_TEXTURE, pic, TF_COLORMAP );

	// black texture
	pic = GL_FakeImage( 4, 4, 1, IMAGE_HAS_COLOR );
	for( x = 0; x < 16; x++ )
		((uint *)pic->buffer)[x] = 0xFF000000;
	tr.blackTexture = GL_LoadTextureInternal( REF_BLACK_TEXTURE, pic, TF_COLORMAP );

	// cinematic dummy
	pic = GL_FakeImage( 640, 100, 1, IMAGE_HAS_COLOR );
	tr.cinTexture = GL_LoadTextureInternal( "*cintexture", pic, TF_NOMIPMAP|TF_CLAMP );
}

/*
===============
R_TextureList_f
===============
*/
void R_TextureList_f( void )
{
}

/*
===============
R_InitImages
===============
*/
void R_InitImages( void )
{
	memset( gl_textures, 0, sizeof( gl_textures ));
	memset( gl_texturesHashTable, 0, sizeof( gl_texturesHashTable ));
	gl_numTextures = 0;

	// create unused 0-entry
	Q_strncpy( gl_textures->name, "*unused*", sizeof( gl_textures->name ));
	gl_textures->hashValue = COM_HashKey( gl_textures->name, TEXTURES_HASH_SIZE );
	gl_textures->nextHash = gl_texturesHashTable[gl_textures->hashValue];
	gl_texturesHashTable[gl_textures->hashValue] = gl_textures;
	gl_numTextures = 1;

	// validate cvars
	R_SetTextureParameters();
	GL_CreateInternalTextures();

	gEngfuncs.Cmd_AddCommand( "texturelist", R_TextureList_f, "display loaded textures list" );
}

/*
===============
R_ShutdownImages
===============
*/
void R_ShutdownImages( void )
{
	gl_texture_t	*tex;
	int		i;

	gEngfuncs.Cmd_RemoveCommand( "texturelist" );
	GL_CleanupAllTextureUnits();

	for( i = 0, tex = gl_textures; i < gl_numTextures; i++, tex++ )
		GL_DeleteTexture( tex );

	memset( tr.lightmapTextures, 0, sizeof( tr.lightmapTextures ));
	memset( gl_texturesHashTable, 0, sizeof( gl_texturesHashTable ));
	memset( gl_textures, 0, sizeof( gl_textures ));
	gl_numTextures = 0;
}

void R_TextureReplacementReport( const char *modelname, int gl_texturenum, const char *foundpath )
{
	if( host_allow_materials->value != 2.0f )
		return;

	if( gl_texturenum > 0 )
		gEngfuncs.Con_Printf( "Looking for %s tex replacement..." S_GREEN "OK (%s)\n", modelname, foundpath );
	else if( gl_texturenum < 0 )
		gEngfuncs.Con_Printf( "Looking for %s tex replacement..." S_YELLOW "MISS (%s)\n", modelname, foundpath );
	else
		gEngfuncs.Con_Printf( "Looking for %s tex replacement..." S_RED "FAIL (%s)\n", modelname, foundpath );
}

qboolean R_SearchForTextureReplacement( char *out, size_t size, const char *modelname, const char *fmt, ... )
{
	va_list ap;
	int ret;

	va_start( ap, fmt );
	ret = Q_vsnprintf( out,	size, fmt, ap );
	va_end( ap );

	if( ret < 0 )
	{
		R_TextureReplacementReport( modelname, -1, "overflow" );
		return false;
	}

	if( gEngfuncs.fsapi->FileExists( out, false ))
		return true;

	R_TextureReplacementReport( modelname, -1, out );
	return false;
}

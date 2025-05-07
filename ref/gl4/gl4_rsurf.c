/*
gl_rsurf.c - surface-related refresh code
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
#include "xash3d_mathlib.h"
#include "mod_local.h"

typedef struct
{
	int		allocated[BLOCK_SIZE_MAX];
	int		current_lightmap_texture;
	msurface_t	*dynamic_surfaces;
	msurface_t	*lightmap_surfaces[MAX_LIGHTMAPS];
	byte		lightmap_buffer[BLOCK_SIZE_MAX*BLOCK_SIZE_MAX*4];
} gllightmapstate_t;

static vec2_t		world_orthocenter;
static vec2_t		world_orthohalf;
static uint		r_blocklights[BLOCK_SIZE_MAX*BLOCK_SIZE_MAX*3];
static mextrasurf_t		*fullbright_surfaces[MAX_TEXTURES];
static mextrasurf_t		*detail_surfaces[MAX_TEXTURES];
static int		rtable[MOD_FRAMES][MOD_FRAMES];

typedef struct
{
	int first, last;
} separate_pass_t;

static separate_pass_t draw_wateralpha = { 0, -1 };
static separate_pass_t draw_alpha_surfaces = { 0, -1 };
static separate_pass_t draw_fullbrights = { 0, -1 };
static separate_pass_t draw_details = { 0, -1 };
static msurface_t		*skychain = NULL;
static gllightmapstate_t	gl_lms;

static void LM_UploadBlock( qboolean dynamic ) { return; }
static qboolean R_AddSurfToVBO( msurface_t *surf, qboolean buildlightmaps ) { return false; }
static void R_DrawVBO( qboolean drawlightmaps, qboolean drawtextures ) { return; }

static inline void R_AddToSeparatePass( separate_pass_t *sp, int num )
{
	if( sp->first > num )
		sp->first = num;

	if( sp->last < num )
		sp->last = num;
}

static inline void R_ResetSeparatePass( separate_pass_t *sp )
{
	sp->last = -1;
}

static inline qboolean R_SeparatePassActive( const separate_pass_t *sp )
{
	return sp->last >= 0 ? true : false;
}

byte *Mod_GetCurrentVis( void )
{
	if( gEngfuncs.drawFuncs->Mod_GetCurrentVis && tr.fCustomRendering )
		return gEngfuncs.drawFuncs->Mod_GetCurrentVis();
	return RI.visbytes;
}

void Mod_SetOrthoBounds( const float *mins, const float *maxs )
{
	if( gEngfuncs.drawFuncs->GL_OrthoBounds )
	{
		gEngfuncs.drawFuncs->GL_OrthoBounds( mins, maxs );
	}

	Vector2Average( maxs, mins, world_orthocenter );
	Vector2Subtract( maxs, world_orthocenter, world_orthohalf );
}

void R_LightmapCoord( const vec3_t v, const msurface_t *surf, const float sample_size, vec2_t coords )
{
	const mextrasurf_t *info = surf->info;
	float s, t;

	s = DotProduct( v, info->lmvecs[0] ) + info->lmvecs[0][3] - info->lightmapmins[0];
	s += surf->light_s * sample_size;
	s += sample_size * 0.5f;
	s /= BLOCK_SIZE * sample_size; //fa->texinfo->texture->width;

	t = DotProduct( v, info->lmvecs[1] ) + info->lmvecs[1][3] - info->lightmapmins[1];
	t += surf->light_t * sample_size;
	t += sample_size * 0.5f;
	t /= BLOCK_SIZE * sample_size; //fa->texinfo->texture->width;

	Vector2Set( coords, s, t );
}

static void R_TextureCoord( const vec3_t v, const msurface_t *surf, vec2_t coords )
{
	const mtexinfo_t *info = surf->texinfo;
	float s, t;

	s = DotProduct( v, info->vecs[0] );
	t = DotProduct( v, info->vecs[1] );

	if( !FBitSet( surf->flags, SURF_DRAWTURB ))
	{
		s = ( s + info->vecs[0][3] ) / info->texture->width;
		t = ( t + info->vecs[1][3] ) / info->texture->height;
	}

	Vector2Set( coords, s, t );
}

static void R_GetEdgePosition( const model_t *mod, const msurface_t *fa, int i, vec3_t vec )
{
	const int lindex = mod->surfedges[fa->firstedge + i];

	if( FBitSet( mod->flags, MODEL_QBSP2 ))
	{
		const medge32_t *pedges = mod->edges32;

		if( lindex > 0 )
			VectorCopy( mod->vertexes[pedges[lindex].v[0]].position, vec );
		else
			VectorCopy( mod->vertexes[pedges[-lindex].v[1]].position, vec );
	}
	else
	{
		const medge16_t *pedges = mod->edges16;

		if( lindex > 0 )
			VectorCopy( mod->vertexes[pedges[lindex].v[0]].position, vec );
		else
			VectorCopy( mod->vertexes[pedges[-lindex].v[1]].position, vec );
	}
}

static void BoundPoly( int numverts, float *verts, vec3_t mins, vec3_t maxs )
{
	int	i, j;
	float	*v;

	ClearBounds( mins, maxs );

	for( i = 0, v = verts; i < numverts; i++ )
	{
		for( j = 0; j < 3; j++, v++ )
		{
			if( *v < mins[j] ) mins[j] = *v;
			if( *v > maxs[j] ) maxs[j] = *v;
		}
	}
}

static void SubdividePolygon_r( model_t *loadmodel, msurface_t *warpface, int numverts, float *verts )
{
}

/*
===============================
GL_SetupFogColorForSurfaces

every render pass applies new fog layer, resulting in wrong fog color
recalculate fog color for current pass count
===============================
*/
static void GL_SetupFogColorForSurfacesEx( int passes, float density, qboolean blend_lightmaps )
{
}


void GL_SetupFogColorForSurfaces( void )
{
	GL_SetupFogColorForSurfacesEx( r_detailtextures.value ? 3 : 2, 1.0f, false );
}

void GL_ResetFogColor( void )
{
}

/*
================
GL_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent and sky warps
can be done reasonably.
================
*/
void GL_SubdivideSurface( model_t *loadmodel, msurface_t *fa )
{
	vec3_t	verts[SUBDIVIDE_SIZE];
	int	i;

	// convert edges back to a normal polygon
	for( i = 0; i < fa->numedges; i++ )
		R_GetEdgePosition( loadmodel, fa, i, verts[i] );

	SetBits( fa->flags, SURF_DRAWTURB_QUADS ); // predict state

	// do subdivide
	SubdividePolygon_r( loadmodel, fa, fa->numedges, verts[0] );
}

/*
================
GL_BuildPolygonFromSurface
================
*/
static int GL_BuildPolygonFromSurface( model_t *mod, msurface_t *fa )
{
	int		i, lnumverts, nColinElim = 0;
	float		sample_size;
	texture_t		*tex;
	gl_texture_t	*glt;
	glpoly2_t		*poly;

	if( !mod || !fa->texinfo || !fa->texinfo->texture )
		return nColinElim; // bad polygon ?

	if( FBitSet( fa->flags, SURF_CONVEYOR ) && fa->texinfo->texture->gl_texturenum != 0 )
	{
		glt = R_GetTexture( fa->texinfo->texture->gl_texturenum );
		tex = fa->texinfo->texture;
		Assert( glt != NULL && tex != NULL );

		// update conveyor widths for keep properly speed of scrolling
		glt->srcWidth = tex->width;
		glt->srcHeight = tex->height;
	}

	sample_size = gEngfuncs.Mod_SampleSizeForFace( fa );

	// reconstruct the polygon
	lnumverts = fa->numedges;

	// detach if already created, reconstruct again
	poly = fa->polys;
	fa->polys = NULL;

	// quake simple models (healthkits etc) need to be reconstructed their polys because LM coords has changed after the map change
	poly = Mem_Realloc( mod->mempool, poly, sizeof( glpoly2_t ) + lnumverts * VERTEXSIZE * sizeof( float ));
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	for( i = 0; i < lnumverts; i++ )
	{
		R_GetEdgePosition( mod, fa, i, poly->verts[i] );
		R_TextureCoord( poly->verts[i], fa, &poly->verts[i][3] );
		R_LightmapCoord( poly->verts[i], fa, sample_size, &poly->verts[i][5] );
	}

	// remove co-linear points - Ed
	if( !gl_keeptjunctions.value && !FBitSet( fa->flags, SURF_UNDERWATER ))
	{
		for( i = 0; i < lnumverts; i++ )
		{
			vec3_t	v1, v2;
			float	*prev, *this, *next;

			prev = poly->verts[(i + lnumverts - 1) % lnumverts];
			next = poly->verts[(i + 1) % lnumverts];
			this = poly->verts[i];

			VectorSubtract( this, prev, v1 );
			VectorNormalize( v1 );
			VectorSubtract( next, prev, v2 );
			VectorNormalize( v2 );

			// skip co-linear points
			if(( fabs( v1[0] - v2[0] ) <= 0.001f) && (fabs( v1[1] - v2[1] ) <= 0.001f) && (fabs( v1[2] - v2[2] ) <= 0.001f))
			{
				int	j, k;

				for( j = i + 1; j < lnumverts; j++ )
				{
					for( k = 0; k < VERTEXSIZE; k++ )
						poly->verts[j-1][k] = poly->verts[j][k];
				}

				// retry next vertex next time, which is now current vertex
				lnumverts--;
				nColinElim++;
				i--;
			}
		}
	}

	poly->numverts = lnumverts;
	return nColinElim;
}


/*
===============
R_TextureAnim

Returns the proper texture for a given time and base texture, do not process random tiling
===============
*/
static texture_t *R_TextureAnim( texture_t *b )
{
	texture_t *base = b;
	int	count, reletive;

	if( RI.currententity->curstate.frame )
	{
		if( base->alternate_anims )
			base = base->alternate_anims;
	}

	if( !base->anim_total )
		return base;

	if( base->name[0] == '-' )
	{
		return b; // already tiled
	}
	else
	{
		int	speed;

		// Quake1 textures uses 10 frames per second
		if( FBitSet( R_GetTexture( base->gl_texturenum )->flags, TF_QUAKEPAL ))
			speed = 10;
		else speed = 20;

		reletive = (int)(gp_cl->time * speed) % base->anim_total;
	}


	count = 0;

	while( base->anim_min > reletive || base->anim_max <= reletive )
	{
		base = base->anim_next;

		if( !base || ++count > MOD_FRAMES )
			return b;
	}

	return base;
}

/*
===============
R_TextureAnimation

Returns the proper texture for a given time and surface
===============
*/
static texture_t *R_TextureAnimation( msurface_t *s )
{
	texture_t	*base = s->texinfo->texture;
	int	count, reletive;

	if( RI.currententity && RI.currententity->curstate.frame )
	{
		if( base->alternate_anims )
			base = base->alternate_anims;
	}

	if( !base->anim_total )
		return base;

	if( base->name[0] == '-' )
	{
		int	tx = (int)((s->texturemins[0] + (base->width << 16)) / base->width) % MOD_FRAMES;
		int	ty = (int)((s->texturemins[1] + (base->height << 16)) / base->height) % MOD_FRAMES;

		reletive = rtable[tx][ty] % base->anim_total;
	}
	else
	{
		int	speed;

		// Quake1 textures uses 10 frames per second
		if( FBitSet( R_GetTexture( base->gl_texturenum )->flags, TF_QUAKEPAL ))
			speed = 10;
		else speed = 20;

		reletive = (int)(gp_cl->time * speed) % base->anim_total;
	}

	count = 0;

	while( base->anim_min > reletive || base->anim_max <= reletive )
	{
		base = base->anim_next;

		if( !base || ++count > MOD_FRAMES )
			return s->texinfo->texture;
	}

	return base;
}

/*
===============
R_AddDynamicLights
===============
*/
static void R_AddDynamicLights( const msurface_t *surf )
{
	const mextrasurf_t *info = surf->info;
	int lnum, smax, tmax;
	int sample_frac = 1.0;
	float sample_size;
	mtexinfo_t *tex;

	// no dlighted surfaces here
	if( !surf->dlightbits )
		return;

	sample_size = gEngfuncs.Mod_SampleSizeForFace( surf );
	smax = (info->lightextents[0] / sample_size) + 1;
	tmax = (info->lightextents[1] / sample_size) + 1;
	tex = surf->texinfo;

	if( FBitSet( tex->flags, TEX_WORLD_LUXELS ))
	{
		if( surf->texinfo->faceinfo )
			sample_frac = surf->texinfo->faceinfo->texture_step;
		else if( FBitSet( surf->texinfo->flags, TEX_EXTRA_LIGHTMAP ))
			sample_frac = LM_SAMPLE_EXTRASIZE;
		else sample_frac = LM_SAMPLE_SIZE;
	}

	for( lnum = 0; lnum < MAX_DLIGHTS; lnum++ )
	{
		dlight_t *dl;
		vec3_t impact, origin_l;
		float dist, rad, minlight;
		float sl, tl;
		int t;

		if( !FBitSet( surf->dlightbits, BIT( lnum )))
			continue;	// not lit by this light

		dl = &tr.dlights[lnum];

		// transform light origin to local bmodel space
		if( !tr.modelviewIdentity )
			Matrix4x4_VectorITransform( RI.objectMatrix, dl->origin, origin_l );
		else VectorCopy( dl->origin, origin_l );

		rad = dl->radius;
		dist = PlaneDiff( origin_l, surf->plane );
		rad -= fabs( dist );

		// rad is now the highest intensity on the plane
		minlight = dl->minlight;
		if( rad < minlight )
			continue;

		minlight = rad - minlight;

		if( surf->plane->type < 3 )
		{
			VectorCopy( origin_l, impact );
			impact[surf->plane->type] -= dist;
		}
		else VectorMA( origin_l, -dist, surf->plane->normal, impact );

		sl = DotProduct( impact, info->lmvecs[0] ) + info->lmvecs[0][3] - info->lightmapmins[0];
		tl = DotProduct( impact, info->lmvecs[1] ) + info->lmvecs[1][3] - info->lightmapmins[1];

		for( t = 0; t < tmax; t++ )
		{
			int td = (tl - sample_size * t) * sample_frac;
			int s;

			if( td < 0 )
				td = -td;

			for( s = 0; s < smax; s++ )
			{
				int sd = (sl - sample_size * s) * sample_frac;
				float dist;

				if( sd < 0 )
					sd = -sd;

				if( sd > td )
					dist = sd + (td >> 1);
				else
					dist = td + (sd >> 1);

				if( dist < minlight )
				{
					uint *bl = &r_blocklights[(s + (t * smax)) * 3];

					bl[0] += ((int)((rad - dist) * 256) * dl->color.r ) / 256;
					bl[1] += ((int)((rad - dist) * 256) * dl->color.g ) / 256;
					bl[2] += ((int)((rad - dist) * 256) * dl->color.b ) / 256;
				}
			}
		}
	}
}

/*
================
R_SetCacheState
================
*/
static void R_SetCacheState( msurface_t *surf )
{
}

/*
=============================================================================

  LIGHTMAP ALLOCATION

=============================================================================
*/
static void LM_InitBlock( void )
{
}

static int LM_AllocBlock( int w, int h, int *x, int *y )
{
	return true;
}

static void LM_UploadDynamicBlock( void )
{
}

/*
=================
R_BuildLightmap

Combine and scale multiple lightmaps into the floating
format in r_blocklights
=================
*/
static void R_BuildLightMap( const msurface_t *surf, byte *dest, int stride, qboolean dynamic )
{
}

/*
================
DrawGLPoly
================
*/
static void DrawGLPoly( glpoly2_t *p, float xScale, float yScale )
{
}

/*
================
DrawGLPolyChain

Render lightmaps
================
*/
static void DrawGLPolyChain( glpoly2_t *p, float soffset, float toffset )
{
}

static qboolean R_HasLightmap( void )
{
	return true;
}

/*
================
R_BlendLightmaps
================
*/
static void R_BlendLightmaps( void )
{
}

/*
================
R_RenderFullbrights
================
*/
static void R_RenderFullbrights( void )
{
}

/*
================
R_RenderDetails
================
*/
static void R_RenderDetails( int passes )
{
}

static void R_RenderFullbrightForSurface( msurface_t *fa, texture_t *t )
{
}

static void R_RenderDetailsForSurface( msurface_t *fa, texture_t *t )
{
}

static void R_RenderDecalsForSurface( msurface_t *fa, int cull_type )
{
}

static qboolean R_CheckLightMap( msurface_t *fa )
{
	return false; // updated
}

static void R_RenderLightmapForSurface( msurface_t *fa )
{
}

/*
================
R_RenderBrushPoly
================
*/
static void R_RenderBrushPoly( msurface_t *fa, int cull_type )
{
}

/*
================
R_DrawTextureChains
================
*/
static void R_DrawTextureChains( void )
{
}

/*
================
R_DrawAlphaTextureChains
================
*/
void R_DrawAlphaTextureChains( void )
{
}

/*
================
R_DrawWaterSurfaces
================
*/
void R_DrawWaterSurfaces( void )
{
}

/*
=================
R_SurfaceCompare

compare translucent surfaces
=================
*/
static int R_SurfaceCompare( const void *a, const void *b )
{
	msurface_t	*surf1, *surf2;
	vec3_t		org1, org2;
	float		len1, len2;

	surf1 = (msurface_t *)((sortedface_t *)a)->surf;
	surf2 = (msurface_t *)((sortedface_t *)b)->surf;

	VectorAdd( RI.currententity->origin, surf1->info->origin, org1 );
	VectorAdd( RI.currententity->origin, surf2->info->origin, org2 );

	// compare by plane dists
	len1 = DotProduct( org1, RI.vforward ) - RI.viewplanedist;
	len2 = DotProduct( org2, RI.vforward ) - RI.viewplanedist;

	if( len1 > len2 )
		return -1;
	if( len1 < len2 )
		return 1;

	return 0;
}

static void R_SetRenderMode( cl_entity_t *e )
{
}

/*
=================
R_DrawBrushModel
=================
*/
void R_DrawBrushModel( cl_entity_t *e )
{
}


/*
==============================

VBO

==============================
*/
/*
Bulld arrays (vboarray_t) for all map geometry on map load.
Store index base for every surface (vbosurfdata_t) to build index arrays
For each texture build index arrays (vbotexture_t) every frame.
*/
// vertex attribs
//#define NO_TEXTURE_MATRIX // need debug
typedef struct vbovertex_s
{
	vec3_t pos;
	vec2_t gl_tc;
	vec2_t lm_tc;
#ifdef NO_TEXTURE_MATRIX
	vec2_t dt_tc;
#endif
} vbovertex_t;

#ifndef UINT_INDEX
typedef unsigned short vboindex_t;
#define VBOINDEX_MAX USHRT_MAX
#define GL_VBOINDEX_TYPE GL_UNSIGNED_SHORT
#else
typedef unsigned int vboindex_t;
#define VBOINDEX_MAX UINT_MAX
#define GL_VBOINDEX_TYPE GL_UNSIGNED_INT
#endif

// store indexes for each texture
typedef struct vbotexture_s
{
	vboindex_t *indexarray; // index array (generated instead of texture chains)
	uint curindex; // counter for index array
	uint len; // maximum index array length
	struct vbotexture_s *next; // if cannot fit into one array, allocate new one, as every array has own index space
	msurface_t *dlightchain; // list of dlight surfaces
	struct vboarray_s *vboarray; // debug
	uint lightmaptexturenum;
} vbotexture_t;

// array list
typedef struct vboarray_s
{
	uint glindex; // glGenBuffers
	int array_len; // allocation length
	vbovertex_t *array; // vertex attrib array
	struct vboarray_s *next; // split by 65536 vertices
} vboarray_t;

// every surface is linked to vbo texture
typedef struct vbosurfdata_s
{
	vbotexture_t *vbotexture;
	uint texturenum;
	uint startindex;
} vbosurfdata_t;

typedef struct vbodecal_s
{
	int numVerts;
} vbodecal_t;

#define DECAL_VERTS_MAX 32
#define DECAL_VERTS_CUT 8

typedef struct vbodecaldata_s
{
	vbodecal_t decals[MAX_RENDER_DECALS];
	vbovertex_t decalarray[MAX_RENDER_DECALS * DECAL_VERTS_CUT];
	uint decalvbo;
	msurface_t **lm;
} vbodecaldata_t;

// gl_decals.c
extern decal_t	gDecalPool[MAX_RENDER_DECALS];

static struct vbo_static_s
{
	// quickly free all allocations on map change
	poolhandle_t mempool;

	// arays
	vbodecaldata_t *decaldata; // array
	vbotexture_t *textures; // array
	vbosurfdata_t *surfdata; // array
	vboarray_t *arraylist; // linked list

	// separate areay for dlights (build during draw)
	vboindex_t *dlight_index; // array
	vec2_t *dlight_tc; // array
	unsigned int dlight_vbo;
	vbovertex_t decal_dlight[MAX_RENDER_DECALS * DECAL_VERTS_MAX];
	unsigned int decal_dlight_vbo;
	int decal_numverts[MAX_RENDER_DECALS * DECAL_VERTS_MAX];

	// prevent draining cpu on empty cycles
	int minlightmap;
	int maxlightmap;
	int mintexture;
	int maxtexture;

	// never skip array splits
	int minarraysplit_tex;
	int maxarraysplit_tex;
	int minarraysplit_lm;
	int maxarraysplit_lm;

	// cvar state potentially might be changed during frame
	// so only enable VBO at the beginning of frame
	qboolean enabled;
} vbos;

static struct multitexturestate_s
{
	int tmu_gl; // texture tmu
	int tmu_dt; // detail tmu
	int tmu_lm; // lightmap tmu
	qboolean details_enabled; // current texture has details
	int lm; // current lightmap texture
	qboolean skiptexture;
	gl_texture_t *glt; // details scale
} mtst;

enum array_state_e
{
	VBO_ARRAY_NONE,
	VBO_ARRAY_STATIC,
	VBO_ARRAY_DECAL,
	VBO_ARRAY_DLIGHT,
	VBO_ARRAY_DECAL_DLIGHT
};

enum texture_state_e
{
	VBO_TEXTURE_NONE,
	VBO_TEXTURE_MAIN,
	VBO_TEXTURE_DECAL
};

enum lightmap_state_e
{
	VBO_LIGHTMAP_NONE,
	VBO_LIGHTMAP_STATIC,
	VBO_LIGHTMAP_DYNAMIC
};

static struct arraystate_s
{
	enum array_state_e astate;
	enum texture_state_e tstate;
	enum lightmap_state_e lstate;
	int itexture;
	qboolean decal_mode;
} vboarray;

qboolean R_HasGeneratedVBO( void )
{
	return vbos.mempool != 0;
}

void R_EnableVBO( qboolean enable )
{
	vbos.enabled = enable;
}

qboolean R_HasEnabledVBO( void )
{
	return vbos.enabled;
}

/*
===================
R_GenerateVBO

Allocate memory for arrays, fill it with vertex attribs and upload to GPU
===================
*/
void R_GenerateVBO( void )
{
}

/*
==============
R_AddDecalVBO

generate decal mesh and put it to array
==============
*/
void R_AddDecalVBO( decal_t *pdecal, msurface_t *surf )
{
}

/*
=============
R_ClearVBO

free all vbo data
=============
*/
void R_ClearVBO( void )
{
}


/*
===================
R_DisableDetail

disable detail tmu
===================
*/
static void R_DisableDetail( void )
{
}

/*
===================
R_EnableDetail

enable detail tmu if availiable
===================
*/
static void R_EnableDetail( void )
{
}

/*
==============
R_SetLightmap

enable lightmap on current tmu
==============
*/
static void R_SetLightmap( void )
{
}

/*
==============
R_SetDecalMode

When drawing decal, disable or restore bump and details
==============
*/
static void R_SetDecalMode( qboolean enable )
{
}

/*
==============
R_SetupVBOTexture

setup multitexture mode before drawing VBOs
if tex is NULL, load texture by number
==============
*/
static texture_t *R_SetupVBOTexture( texture_t *tex, int number )
{
	return NULL;
}


static void R_SetupVBOArrayStatic( vboarray_t *vbo, qboolean drawlightmap, qboolean drawtextures )
{
}



static void R_SetupVBOArrayDlight( vboarray_t *vbo, texture_t *texture )
{
}

#define SPARSE_DECALS_UPLOAD 0

static void R_SetupVBOArrayDecalDlight( int decalcount )
{
}


/*
===================
R_AdditionalPasses

draw details when not enough tmus
===================
*/
static void R_AdditionalPasses( vboarray_t *vbo, int indexlen, void *indexarray, texture_t *tex, qboolean resetvbo, size_t offset )
{
}


#define MINIMIZE_UPLOAD
#define DISCARD_DLIGHTS


static void R_DrawDlightedDecals( vboarray_t *vbo, msurface_t *newsurf, msurface_t *surf, int decalcount, texture_t *texture )
{
}

static void R_FlushDlights( vboarray_t *vbo, int min_index, int max_index, int dlightindex, vboindex_t *dlightarray )
{
}

static void R_AddSurfaceDecalsDlight( msurface_t *surf, int *pdecalcount )
{
}


static void R_DrawVBODlights( vboarray_t *vbo, vbotexture_t *vbotex, texture_t *texture, int lightmap )
{
}



/*
=====================
R_DrawLightmappedVBO

Draw array for given vbotexture_t. build and draw dynamic lightmaps if present
=====================
*/
static void R_DrawLightmappedVBO( vboarray_t *vbo, vbotexture_t *vbotex, texture_t *texture, int lightmap, qboolean skiplighting )
{
}

static void R_SetupVBOArrayDecal( qboolean drawlightmap )
{
}

static void R_SetupVBOArrayDecalDyn( qboolean drawlightmap, float *v )
{
}

static void R_DrawStaticDecals( vboarray_t *vbo, qboolean drawlightmap, int ilightmap )
{
}

static void R_ClearVBOState( qboolean drawlightmap, qboolean drawtextures )
{
}

/*
=============================================================

	WORLD MODEL

=============================================================
*/
/*
================
R_RecursiveWorldNode
================
*/
static void R_RecursiveWorldNode( mnode_t *node, uint clipflags )
{
}

/*
================
R_CullNodeTopView

cull node by user rectangle (simple scissor)
================
*/
static qboolean R_CullNodeTopView( mnode_t *node )
{
}

/*
================
R_DrawTopViewLeaf
================
*/
static void R_DrawTopViewLeaf( mleaf_t *pleaf, uint clipflags )
{
}

/*
================
R_DrawWorldTopView
================
*/
static void R_DrawWorldTopView( mnode_t *node, uint clipflags )
{
}

/*
=============
R_DrawTriangleOutlines
=============
*/
static void R_DrawTriangleOutlines( void )
{
}

/*
=============
R_DrawWorld
=============
*/
void R_DrawWorld( void )
{
}

/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current leaf
===============
*/
void R_MarkLeaves( void )
{
}

/*
========================
GL_CreateSurfaceLightmap
========================
*/
static void GL_CreateSurfaceLightmap( msurface_t *surf, model_t *loadmodel )
{
}

/*
==================
GL_RebuildLightmaps

Rebuilds the lightmap texture
when gamma is changed
==================
*/
void GL_RebuildLightmaps( void )
{
}

/*
==================
GL_BuildLightmaps

Builds the lightmap texture
with all the surfaces from all brush models
==================
*/
void GL_BuildLightmaps( void )
{
}

void GL_InitRandomTable( void )
{
}

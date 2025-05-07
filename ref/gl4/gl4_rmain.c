/*
gl_rmain.c - renderer main loop
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
#include "library.h"
#include "beamdef.h"
#include "particledef.h"
#include "entity_types.h"


#define IsLiquidContents( cnt )	( cnt == CONTENTS_WATER || cnt == CONTENTS_SLIME || cnt == CONTENTS_LAVA )

float		gldepthmin, gldepthmax;
ref_instance_t	RI;

static int R_RankForRenderMode( int rendermode )
{
	switch( rendermode )
	{
	case kRenderTransTexture:
		return 1;	// draw second
	case kRenderTransAdd:
		return 2;	// draw third
	case kRenderGlow:
		return 3;	// must be last!
	}
	return 0;
}

void R_AllowFog( qboolean allowed )
{
}

/*
===============
R_OpaqueEntity

Opaque entity can be brush or studio model but sprite
===============
*/
qboolean R_OpaqueEntity( cl_entity_t *ent )
{
	return false;
}

/*
===============
R_TransEntityCompare

Sorting translucent entities by rendermode then by distance
===============
*/
static int R_TransEntityCompare( const void *a, const void *b )
{
	cl_entity_t	*ent1, *ent2;
	vec3_t		vecLen, org;
	float		dist1, dist2;
	int		rendermode1;
	int		rendermode2;

	ent1 = *(cl_entity_t **)a;
	ent2 = *(cl_entity_t **)b;
	rendermode1 = R_GetEntityRenderMode( ent1 );
	rendermode2 = R_GetEntityRenderMode( ent2 );

	// sort by distance
	if( ent1->model->type != mod_brush || rendermode1 != kRenderTransAlpha )
	{
		VectorAverage( ent1->model->mins, ent1->model->maxs, org );
		VectorAdd( ent1->origin, org, org );
		VectorSubtract( RI.vieworg, org, vecLen );
		dist1 = DotProduct( vecLen, vecLen );
	}
	else dist1 = 1000000000;

	if( ent2->model->type != mod_brush || rendermode2 != kRenderTransAlpha )
	{
		VectorAverage( ent2->model->mins, ent2->model->maxs, org );
		VectorAdd( ent2->origin, org, org );
		VectorSubtract( RI.vieworg, org, vecLen );
		dist2 = DotProduct( vecLen, vecLen );
	}
	else dist2 = 1000000000;

	if( dist1 > dist2 )
		return -1;
	if( dist1 < dist2 )
		return 1;

	// then sort by rendermode
	if( R_RankForRenderMode( rendermode1 ) > R_RankForRenderMode( rendermode2 ))
		return 1;
	if( R_RankForRenderMode( rendermode1 ) < R_RankForRenderMode( rendermode2 ))
		return -1;

	return 0;
}

/*
===============
R_WorldToScreen

Convert a given point from world into screen space
Returns true if we behind to screen
===============
*/
int R_WorldToScreen( const vec3_t point, vec3_t screen )
{
	matrix4x4	worldToScreen;
	qboolean	behind;
	float	w;

	if( !point || !screen )
		return true;

	Matrix4x4_Copy( worldToScreen, RI.worldviewProjectionMatrix );
	screen[0] = worldToScreen[0][0] * point[0] + worldToScreen[0][1] * point[1] + worldToScreen[0][2] * point[2] + worldToScreen[0][3];
	screen[1] = worldToScreen[1][0] * point[0] + worldToScreen[1][1] * point[1] + worldToScreen[1][2] * point[2] + worldToScreen[1][3];
	w = worldToScreen[3][0] * point[0] + worldToScreen[3][1] * point[1] + worldToScreen[3][2] * point[2] + worldToScreen[3][3];
	screen[2] = 0.0f; // just so we have something valid here

	if( w < 0.001f )
	{
		behind = true;
	}
	else
	{
		float invw = 1.0f / w;
		screen[0] *= invw;
		screen[1] *= invw;
		behind = false;
	}

	return behind;
}

/*
===============
R_ScreenToWorld

Convert a given point from screen into world space
===============
*/
void R_ScreenToWorld( const vec3_t screen, vec3_t point )
{
	matrix4x4	screenToWorld;
	float	w;

	if( !point || !screen )
		return;

	Matrix4x4_Invert_Full( screenToWorld, RI.worldviewProjectionMatrix );

	point[0] = screen[0] * screenToWorld[0][0] + screen[1] * screenToWorld[0][1] + screen[2] * screenToWorld[0][2] + screenToWorld[0][3];
	point[1] = screen[0] * screenToWorld[1][0] + screen[1] * screenToWorld[1][1] + screen[2] * screenToWorld[1][2] + screenToWorld[1][3];
	point[2] = screen[0] * screenToWorld[2][0] + screen[1] * screenToWorld[2][1] + screen[2] * screenToWorld[2][2] + screenToWorld[2][3];
	w = screen[0] * screenToWorld[3][0] + screen[1] * screenToWorld[3][1] + screen[2] * screenToWorld[3][2] + screenToWorld[3][3];
	if( w != 0.0f ) VectorScale( point, ( 1.0f / w ), point );
}

/*
===============
R_PushScene
===============
*/
void R_PushScene( void )
{
}

/*
===============
R_PopScene
===============
*/
void R_PopScene( void )
{
}

/*
===============
R_ClearScene
===============
*/
void R_ClearScene( void )
{
}

/*
===============
R_AddEntity
===============
*/
qboolean R_AddEntity( struct cl_entity_s *clent, int type )
{
	return true;
}

/*
=============
R_Clear
=============
*/
static void R_Clear( int bitMask )
{
}

//=============================================================================
/*
===============
R_GetFarClip
===============
*/
static float R_GetFarClip( void )
{
	return 2048.0f;
}

/*
===============
R_SetupFrustum
===============
*/
void R_SetupFrustum( void )
{
}

/*
=============
R_SetupProjectionMatrix
=============
*/
static void R_SetupProjectionMatrix( matrix4x4 m )
{
	GLfloat	xMin, xMax, yMin, yMax, zNear, zFar;

	if( RI.drawOrtho )
	{
		const ref_overview_t *ov = gEngfuncs.GetOverviewParms();
		Matrix4x4_CreateOrtho( m, ov->xLeft, ov->xRight, ov->yTop, ov->yBottom, ov->zNear, ov->zFar );
		return;
	}

	RI.farClip = R_GetFarClip();

	zNear = 4.0f;
	zFar = Q_max( 256.0f, RI.farClip );

	yMax = zNear * tan( RI.fov_y * M_PI_F / 360.0f );
	yMin = -yMax;

	xMax = zNear * tan( RI.fov_x * M_PI_F / 360.0f );
	xMin = -xMax;

	Matrix4x4_CreateProjection( m, xMax, xMin, yMax, yMin, zNear, zFar );
}

/*
=============
R_SetupModelviewMatrix
=============
*/
static void R_SetupModelviewMatrix( matrix4x4 m )
{
	Matrix4x4_CreateModelview( m );
	Matrix4x4_ConcatRotate( m, -RI.viewangles[2], 1, 0, 0 );
	Matrix4x4_ConcatRotate( m, -RI.viewangles[0], 0, 1, 0 );
	Matrix4x4_ConcatRotate( m, -RI.viewangles[1], 0, 0, 1 );
	Matrix4x4_ConcatTranslate( m, -RI.vieworg[0], -RI.vieworg[1], -RI.vieworg[2] );
}

/*
=============
R_LoadIdentity
=============
*/
void R_LoadIdentity( void )
{
}

/*
=============
R_RotateForEntity
=============
*/
void R_RotateForEntity( cl_entity_t *e )
{
}

/*
=============
R_TranslateForEntity
=============
*/
void R_TranslateForEntity( cl_entity_t *e )
{
}

/*
===============
R_FindViewLeaf
===============
*/
void R_FindViewLeaf( void )
{
}

/*
===============
R_SetupFrame
===============
*/
static void R_SetupFrame( void )
{
}

/*
=============
R_SetupGL
=============
*/
void R_SetupGL( qboolean set_gl_state )
{
}

/*
=============
R_EndGL
=============
*/
static void R_EndGL( void )
{
}

/*
=============
R_RecursiveFindWaterTexture

using to find source waterleaf with
watertexture to grab fog values from it
=============
*/
static gl_texture_t *R_RecursiveFindWaterTexture( const mnode_t *node, const mnode_t *ignore, qboolean down )
{
}

/*
=============
R_CheckFog

check for underwater fog
Using backward recursion to find waterline leaf
from underwater leaf (idea: XaeroX)
=============
*/
static void R_CheckFog( void )
{
}

/*
=============
R_CheckGLFog

special condition for Spirit 1.9
that used direct calls of glFog-functions
=============
*/
static void R_CheckGLFog( void )
{
}

/*
=============
R_DrawFog

=============
*/
void R_DrawFog( void )
{
}

/*
=============
R_DrawEntitiesOnList
=============
*/
static void R_DrawEntitiesOnList( void )
{
}

/*
================
R_RenderScene

R_SetupRefParams must be called right before
================
*/
void R_RenderScene( void )
{
	if( !WORLDMODEL && RI.drawWorld )
		gEngfuncs.Host_Error( "%s: NULL worldmodel\n", __func__ );

	// frametime is valid only for normal pass
	if( RP_NORMALPASS( ))
		tr.frametime = gp_cl->time - gp_cl->oldtime;
	else tr.frametime = 0.0;

	// begin a new frame
	tr.framecount++;

	R_PushDlights();

	R_SetupFrustum();
	R_SetupFrame();
	R_SetupGL( true );
	R_Clear( ~0 );

	R_MarkLeaves();
	R_DrawFog ();
	if( RI.drawWorld )
		R_AnimateRipples();

	R_CheckGLFog();
	R_DrawWorld();
	R_CheckFog();

	gEngfuncs.CL_ExtraUpdate ();	// don't let sound get messed up if going slow

	R_DrawEntitiesOnList();

	R_DrawWaterSurfaces();

	R_EndGL();
}

void R_GammaChanged( qboolean do_reset_gamma )
{
}

static void R_CheckCvars( void )
{
}

/*
===============
R_BeginFrame
===============
*/
void R_BeginFrame( qboolean clearScene )
{
	// gEngfuncs.Con_Printf("Begin Frame\n");

	glConfig.softwareGammaUpdate = false;	// in case of possible fails

	glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	R_CheckCvars();

	R_Set2DMode( true );

	// Todo: Draw buffer

	// Todo: Update texture parameters

	gEngfuncs.CL_ExtraUpdate ();
}

/*
===============
R_SetupRefParams

set initial params for renderer
===============
*/
void R_SetupRefParams( const ref_viewpass_t *rvp )
{
}

/*
===============
R_RenderFrame
===============
*/
void R_RenderFrame( const ref_viewpass_t *rvp )
{
}

/*
===============
R_EndFrame
===============
*/
void R_EndFrame( void )
{
	gEngfuncs.GL_SwapBuffers();
}

/*
===============
R_DrawCubemapView
===============
*/
void R_DrawCubemapView( const vec3_t origin, const vec3_t angles, int size )
{
}

/*
===============
CL_FxBlend
===============
*/
int CL_FxBlend( cl_entity_t *e )
{
}

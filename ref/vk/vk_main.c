/*
 * vk_main.c - Vulkan renderer entry point
 * Copyright (C) 2026 Xash3D contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "vk_local.h"

#if XASH_VULKAN

// ============================================================================
// Global Variables
// ============================================================================

// Engine function table (filled by engine)
ref_api_t gEngfuncs;

// Renderer globals
ref_globals_t gVars;

// Render interface (exported to engine)
ref_interface_t gRenderFuncs;

// Vulkan context and backend
vk_context_t vk;
vk_backend_t vkB;
vk_refdef_t vkR;

// ============================================================================
// Entry Point - GetRefAPI
// ============================================================================

/*
 * VK_RefGetParm - Get renderer parameter
 */
static int VK_RefGetParm( int parm, int arg )
{
	switch( parm )
	{
	case PARM_TEX_WIDTH:
		// TODO: Implement texture width queries
		return 0;
	case PARM_TEX_HEIGHT:
		// TODO: Implement texture height queries
		return 0;
	case PARM_TEX_FILTERING:
		return 0; // filtering disabled
	default:
		return 0;
	}
}

/*
=============
VK_CL_FillRGBA

=============
*/
static void VK_CL_FillRGBA( int rendermode, float _x, float _y, float _w, float _h, byte r, byte g, byte b, byte a )
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

void VK_SimpleStub ( void ) 
{

}

/*
===============
CL_DrawParticlesExternal

allow to draw effects from custom renderer
===============
*/
void VK_CL_DrawParticlesExternal( const ref_viewpass_t *rvp, qboolean trans_pass, float frametime )
{
	
}

const ref_interface_t gReffuncs =
{
	VK_Init,
	VK_Shutdown,
	VK_GetConfigName,
	VK_SetDisplayTransform,

	VK_GL_SetupAttributes,
	VK_GL_InitExtensions,
	VK_GL_ClearExtensions,

	VK_R_GammaChanged,
	VK_R_BeginFrame,
	VK_R_RenderScene,
	VK_R_EndFrame,
	VK_R_PushScene,
	VK_R_PopScene,
	VK_GL_BackendStartFrame,
	VK_GL_BackendEndFrame,

	VK_R_ClearScreen,
	VK_R_AllowFog,
	VK_GL_SetRenderMode,

	VK_R_AddEntity,
	VK_R_ProcessEntData,
	VK_R_Flush,

	VK_R_ShowTextures,

	VK_R_GetTextureOriginalBuffer,
	VK_GL_LoadTextureFromBuffer,
	VK_GL_ProcessTexture,
	VK_R_SetupSky,

	VK_R_Set2DMode,
	VK_R_DrawStretchRaw,
	VK_R_DrawStretchPic,
	VK_CL_FillRGBA,
	VK_WorldToScreen,

	VK_VID_ScreenShot,
	VK_VID_CubemapShot,

	VK_R_LightPoint,

	VK_R_DecalShoot,
	VK_R_DecalRemoveAll,
	VK_R_CreateDecalList,
	VK_R_ClearAllDecals,

	VK_R_StudioEstimateFrame,
	VK_R_StudioLerpMovement,
	VK_CL_InitStudioAPI,

	VK_R_SetSkyCloudsTextures,
	VK_R_SubdivideSurface,
	VK_CL_RunLightStyles,


	VK_Mod_ProcessRenderData,
	VK_Mod_StudioLoadTextures,

	VK_CL_DrawParticles,
	VK_CL_DrawTracers,
	VK_CL_DrawBeams,
	VK_R_BeamCull,

	VK_RefGetParm,
	VK_GetDetailScaleForTexture,
	VK_GetExtraParmsForTexture,
	VK_GetFrameTime,

	VK_R_SetCurrentEntity,
	VK_R_SetCurrentModel,

	VK_GL_FindTexture,
	VK_GL_TextureName,
	VK_GL_TextureData,
	VK_GL_LoadTexture,
	VK_GL_CreateTexture,
	VK_GL_LoadTextureArray,
	VK_GL_CreateTextureArray,
	VK_GL_FreeTexture,
	VK_R_OverrideTextureSourceSize,

	VK_DrawSingleDecal,
	VK_R_DecalSetupVerts,
	VK_R_EntityRemoveDecals,

	R_UploadStretchRaw,

	VK_GL_Bind,
	VK_GL_SelectTexture,
	VK_SimpleStub,
	VK_SimpleStub,
	VK_GL_CleanUpTextureUnits,
	VK_GL_TexGen,
	VK_GL_TextureTarget,
	VK_SimpleStub,
	VK_GL_UpdateTexSize,

	VK_CL_DrawParticlesExternal,
	R_LightVec,
	VK_StudioGetTexture,

	VK_GL_RenderFrame,
	VK_SimpleStub,
	VK_R_SpeedsMessage,
	Mod_GetCurrentVis,
	VK_R_NewMap,
	VK_R_ClearScene,
	VK_R_GetProcAddress,

	TriRenderMode,
	TriBegin,
	TriEnd,
	_TriColor4f,
	_TriColor4ub,
	TriTexCoord2f,
	TriVertex3fv,
	TriVertex3f,
	TriFog,
	VK_R_ScreenToWorld,
	TriGetMatrix,
	TriFogParams,
	TriCullFace,

	VGUI_SetupDrawing,
	VGUI_UploadTextureBlock,
};

int EXPORT GetRefAPI( int version, ref_interface_t *funcs, ref_api_t *engfuncs, ref_globals_t *globals );
int EXPORT GetRefAPI( int version, ref_interface_t *funcs, ref_api_t *engfuncs, ref_globals_t *globals )
{
	if( version != REF_API_VERSION )
		return 0;

	// fill in our callbacks
	*funcs = gReffuncs;
	gEngfuncs = *engfuncs;
	gpGlobals = globals;

	return REF_API_VERSION;
}

#endif // XASH_VULKAN

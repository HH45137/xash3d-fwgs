/*
vid_sdl.c - SDL vid component
Copyright (C) 2018 a1batross

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

// GL API function pointers, if any, reside in this translation unit
#define APIENTRY_LINKAGE
#include "gl4_local.h"

ref_api_t gEngfuncs;
ref_globals_t* gpGlobals;
ref_client_t* gp_cl;
ref_host_t* gp_host;

void _Mem_Free(void* data, const char* filename, int fileline) { }
void* _Mem_Alloc(poolhandle_t poolptr, size_t size, qboolean clear, const char* filename, int fileline) { return NULL; }
static void R_ClearScreen(void) { }
static const byte* R_GetTextureOriginalBuffer(unsigned int idx) { return NULL; }
static void CL_FillRGBA(int rendermode, float _x, float _y, float _w, float _h, byte r, byte g, byte b, byte a) { }
static qboolean Mod_LooksLikeWaterTexture(const char* name) { return false; }
static void Mod_BrushUnloadTextures(model_t* mod) { }
static void Mod_UnloadTextures(model_t* mod) { }
static qboolean Mod_ProcessRenderData(model_t* mod, qboolean create, const byte* buf) { return false; }
static int GL_RefGetParm(int parm, int arg) { return 0; }
static void R_GetDetailScaleForTexture(int texture, float* xScale, float* yScale) { }
static void R_GetExtraParmsForTexture(int texture, byte* red, byte* green, byte* blue, byte* density) { }
static void R_SetCurrentEntity(cl_entity_t* ent) { }
static void R_SetCurrentModel(model_t* mod) { }
static float R_GetFrameTime(void) { return 0.0f; }
static const char* GL_TextureName(unsigned int texnum) { return NULL; }
static const byte* GL_TextureData(unsigned int texnum) { return NULL; }
static void R_ProcessEntData(qboolean allocate, cl_entity_t* entities, unsigned int max_entities) { }
static void GAME_EXPORT R_Flush(unsigned int flags) { }
static void GAME_EXPORT R_SetSkyCloudsTextures(int solidskyTexture, int alphaskyTexture) { }
static void GAME_EXPORT R_SetupSky(int* skyboxTextures) { }
static qboolean R_SetDisplayTransform(ref_screen_rotation_t rotate, int offset_x, int offset_y, float scale_x, float scale_y) { return false; }
static void GAME_EXPORT VGUI_UploadTextureBlock(int drawX, int drawY, const byte* rgba, int blockWidth, int blockHeight) { }
static void GAME_EXPORT VGUI_SetupDrawing(qboolean rect) { }
static void GAME_EXPORT R_OverrideTextureSourceSize(unsigned int texnum, uint srcWidth, uint srcHeight) { }
static void* GAME_EXPORT R_GetProcAddress(const char* name) { return NULL; }
static const char* R_GetConfigName(void) { return NULL; }

static const ref_interface_t gReffuncs = {
    R_Init,
    R_Shutdown,
    R_GetConfigName,
    R_SetDisplayTransform,

    GL_SetupAttributes,
    GL_InitExtensions,
    GL_ClearExtensions,

    R_GammaChanged,
    R_BeginFrame,
    R_RenderScene,
    R_EndFrame,
    R_PushScene,
    R_PopScene,
    GL_BackendStartFrame,
    GL_BackendEndFrame,

    R_ClearScreen,
    R_AllowFog,
    GL_SetRenderMode,

    R_AddEntity,
    CL_AddCustomBeam,
    R_ProcessEntData,
    R_Flush,

    R_ShowTextures,

    R_GetTextureOriginalBuffer,
    GL_LoadTextureFromBuffer,
    GL_ProcessTexture,
    R_SetupSky,

    R_Set2DMode,
    R_DrawStretchRaw,
    R_DrawStretchPic,
    CL_FillRGBA,
    R_WorldToScreen,

    VID_ScreenShot,
    VID_CubemapShot,

    R_LightPoint,

    R_DecalShoot,
    R_DecalRemoveAll,
    R_CreateDecalList,
    R_ClearAllDecals,

    R_StudioEstimateFrame,
    R_StudioLerpMovement,
    CL_InitStudioAPI,

    R_SetSkyCloudsTextures,
    GL_SubdivideSurface,
    CL_RunLightStyles,

    R_GetSpriteParms,
    R_GetSpriteTexture,

    Mod_ProcessRenderData,
    Mod_StudioLoadTextures,

    CL_DrawParticles,
    CL_DrawTracers,
    CL_DrawBeams,
    R_BeamCull,

    GL_RefGetParm,
    R_GetDetailScaleForTexture,
    R_GetExtraParmsForTexture,
    R_GetFrameTime,

    R_SetCurrentEntity,
    R_SetCurrentModel,

    GL_FindTexture,
    GL_TextureName,
    GL_TextureData,
    GL_LoadTexture,
    GL_CreateTexture,
    GL_LoadTextureArray,
    GL_CreateTextureArray,
    GL_FreeTexture,
    R_OverrideTextureSourceSize,

    DrawSingleDecal,
    R_DecalSetupVerts,
    R_EntityRemoveDecals,

    R_UploadStretchRaw,

    GL_Bind,
    GL_SelectTexture,
    GL_LoadTexMatrixExt,
    GL_LoadIdentityTexMatrix,
    GL_CleanUpTextureUnits,
    GL_TexGen,
    GL_TextureTarget,
    GL_SetTexCoordArrayMode,
    GL_UpdateTexSize,
    NULL,
    NULL,

    CL_DrawParticlesExternal,
    R_LightVec,
    R_StudioGetTexture,

    R_RenderFrame,
    Mod_SetOrthoBounds,
    R_SpeedsMessage,
    Mod_GetCurrentVis,
    R_NewMap,
    R_ClearScene,
    R_GetProcAddress,

    TriRenderMode,
    TriBegin,
    TriEnd,
    _TriColor4f,
    _TriColor4ub,
    TriTexCoord2f,
    TriVertex3fv,
    TriVertex3f,
    TriFog,
    R_ScreenToWorld,
    TriGetMatrix,
    TriFogParams,
    TriCullFace,

    VGUI_SetupDrawing,
    VGUI_UploadTextureBlock,
};

int EXPORT GetRefAPI(int version, ref_interface_t* funcs, ref_api_t* engfuncs, ref_globals_t* globals);
int EXPORT GetRefAPI(int version, ref_interface_t* funcs, ref_api_t* engfuncs, ref_globals_t* globals)
{
    if (version != REF_API_VERSION)
        return 0;

    // fill in our callbacks
    *funcs = gReffuncs;
    gEngfuncs = *engfuncs;
    gpGlobals = globals;

    gp_cl = (ref_client_t*)ENGINE_GET_PARM(PARM_GET_CLIENT_PTR);
    gp_host = (ref_host_t*)ENGINE_GET_PARM(PARM_GET_HOST_PTR);

    return REF_API_VERSION;
}

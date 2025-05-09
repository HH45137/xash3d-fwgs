#include "gl4_local.h"

ref_speeds_t r_stats;
ref_instance_t RI;
gl_globals_t tr;

// gl_backend.c
void GL_BackendStartFrame(void) { }
void GL_BackendEndFrame(void) { }
void GL_CleanUpTextureUnits(int last) { }
void GL_Bind(GLint tmu, GLenum texnum) { }
void GL_MultiTexCoord2f(GLenum texture, GLfloat s, GLfloat t) { }
void GL_SetTexCoordArrayMode(GLenum mode) { }
void GL_LoadTexMatrixExt(const float* glmatrix) { }
void GL_LoadMatrix(const matrix4x4 source) { }
void GL_TexGen(GLenum coord, GLenum mode) { }
void GL_SelectTexture(GLint texture) { }
void GL_CleanupAllTextureUnits(void) { }
void GL_LoadIdentityTexMatrix(void) { }
void GL_DisableAllTexGens(void) { }
void GL_SetRenderMode(int mode) { }
void GL_EnableTextureUnit(int tmu, qboolean enable) { }
void GL_TextureTarget(uint target) { }
void GL_Cull(GLenum cull) { }
void R_ShowTextures(void) { }
void SCR_TimeRefresh_f(void) { }

// gl_beams.c
void CL_DrawBeams(int fTrans, BEAM* active_beams) { }
qboolean R_BeamCull(const vec3_t start, const vec3_t end, qboolean pvsOnly) { return false; }

// gl_cull.c
int R_CullModel(cl_entity_t* e, const vec3_t absmin, const vec3_t absmax) { return 0; }
qboolean R_CullBox(const vec3_t mins, const vec3_t maxs) { return false; }

// gl_decals.c
void DrawSurfaceDecals(msurface_t* fa, qboolean single, qboolean reverse) { }
float* R_DecalSetupVerts(decal_t* pDecal, msurface_t* surf, int texture, int* outCount) { return NULL; }
void DrawSingleDecal(decal_t* pDecal, msurface_t* fa) { }
void R_EntityRemoveDecals(model_t* mod) { }
void DrawDecalsBatch(void) { }
void R_ClearDecals(void) { }

// gl_draw.c
void R_Set2DMode(qboolean enable) { }
void R_UploadStretchRaw(int texture, int cols, int rows, int width, int height, const byte* data) { }

// gl_drawhulls.c
void R_DrawWorldHull(void) { }
void R_DrawModelHull(void) { }

// gl_image.c
void R_SetTextureParameters(void) { }
gl_texture_t* R_GetTexture(GLenum texnum) { return NULL; }
const char* GL_TargetToString(GLenum target) { return NULL; }
int GL_LoadTexture(const char* name, const byte* buf, size_t size, int flags) { return 0; }
int GL_LoadTextureArray(const char** names, int flags) { return 0; }
int GL_LoadTextureFromBuffer(const char* name, rgbdata_t* pic, texFlags_t flags, qboolean update) { return 0; }
byte* GL_ResampleTexture(const byte* source, int in_w, int in_h, int out_w, int out_h, qboolean isNormalMap) { return NULL; }
int GL_CreateTexture(const char* name, int width, int height, const void* buffer, texFlags_t flags) { return 0; }
int GL_CreateTextureArray(const char* name, int width, int height, int depth, const void* buffer, texFlags_t flags) { return 0; }
void GL_ProcessTexture(int texnum, float gamma, int topColor, int bottomColor) { }
void GL_UpdateTexSize(int texnum, int width, int height, int depth) { }
qboolean GL_TextureFilteringEnabled(const gl_texture_t* tex) { return false; }
void GL_ApplyTextureParams(gl_texture_t* tex) { }
int GL_FindTexture(const char* name) { return 0; }
void GL_FreeTexture(GLenum texnum) { }
const char* GL_Target(GLenum target) { return NULL; }
void R_InitDlightTexture(void) { }
void R_TextureList_f(void) { }
void R_InitImages(void) { }
void R_ShutdownImages(void) { }
int GL_TexMemory(void) { return 0; }
qboolean R_SearchForTextureReplacement(char* out, size_t size, const char* modelname, const char* fmt, ...) { return false; }
void R_TextureReplacementReport(const char* modelname, int gl_texturenum, const char* foundpath) { }

// gl_rlight.c
void CL_RunLightStyles(lightstyle_t* ls) { }
void R_PushDlights(void) { }
void R_GetLightSpot(vec3_t lightspot) { }
void R_MarkLights(const dlight_t* light, int bit, const mnode_t* node) { }
colorVec R_LightVec(const vec3_t start, const vec3_t end, vec3_t lightspot, vec3_t lightvec) { return (colorVec) { 0 }; }
colorVec R_LightPoint(const vec3_t p0) { return (colorVec) { 0 }; }

// gl_rmain.c
void R_ClearScene(void) { }
void R_LoadIdentity(void) { }
void R_RenderScene(void) { }
void R_DrawCubemapView(const vec3_t origin, const vec3_t angles, int size) { }
void R_SetupRefParams(const struct ref_viewpass_s* rvp) { }
void R_TranslateForEntity(cl_entity_t* e) { }
void R_RotateForEntity(cl_entity_t* e) { }
void R_SetupGL(qboolean set_gl_state) { }
void R_AllowFog(qboolean allowed) { }
qboolean R_OpaqueEntity(cl_entity_t* ent) { return false; }
void R_SetupFrustum(void) { }
void R_FindViewLeaf(void) { }
void R_PushScene(void) { }
void R_PopScene(void) { }
void R_DrawFog(void) { }
int CL_FxBlend(cl_entity_t* e) { return 0; }
void R_RenderFrame(const ref_viewpass_t* rvp) { }
int R_WorldToScreen(const vec3_t point, vec3_t screen) { return 0; }
void R_ScreenToWorld(const vec3_t screen, vec3_t point) { }

// gl_rmath.c
void Matrix4x4_ToArrayFloatGL(const matrix4x4 in, float out[16]) { }
void Matrix4x4_Concat(matrix4x4 out, const matrix4x4 in1, const matrix4x4 in2) { }
void Matrix4x4_ConcatTranslate(matrix4x4 out, float x, float y, float z) { }
void Matrix4x4_ConcatRotate(matrix4x4 out, float angle, float x, float y, float z) { }
void Matrix4x4_CreateProjection(matrix4x4 out, float xMax, float xMin, float yMax, float yMin, float zNear, float zFar) { }
void Matrix4x4_CreateOrtho(matrix4x4 m, float xLeft, float xRight, float yBottom, float yTop, float zNear, float zFar) { }
void Matrix4x4_CreateModelview(matrix4x4 out) { }

// gl_rmisc.c
void R_ClearStaticEntities(void) { }

// gl_rsurf.c
void R_MarkLeaves(void) { }
void R_DrawWorld(void) { }
void R_DrawWaterSurfaces(void) { }
void R_DrawBrushModel(cl_entity_t* e) { }
void GL_SubdivideSurface(model_t* mod, msurface_t* fa) { }
void GL_SetupFogColorForSurfaces(void) { }
void R_DrawAlphaTextureChains(void) { }
void GL_RebuildLightmaps(void) { }
void GL_InitRandomTable(void) { }
void GL_BuildLightmaps(void) { }
void GL_ResetFogColor(void) { }
void R_GenerateVBO(void) { }
void R_ClearVBO(void) { }
void R_AddDecalVBO(decal_t* pdecal, msurface_t* surf) { }
void R_LightmapCoord(const vec3_t v, const msurface_t* surf, const float sample_size, vec2_t coords) { }
qboolean R_HasGeneratedVBO(void) { return false; }
void R_EnableVBO(qboolean enable) { }
qboolean R_HasEnabledVBO(void) { return false; }
void Mod_SetOrthoBounds(const float* mins, const float* maxs) { }

// gl_rpart.c
void CL_DrawParticlesExternal(const ref_viewpass_t* rvp, qboolean trans_pass, float frametime) { }
void CL_DrawParticles(double frametime, particle_t* cl_active_particles, float partsize) { }
void CL_DrawTracers(double frametime, particle_t* cl_active_tracers) { }

// gl_sprite.c
void R_SpriteInit(void) { }
void Mod_LoadSpriteModel(model_t* mod, const void* buffer, qboolean* loaded, uint texFlags) { }
mspriteframe_t* R_GetSpriteFrame(const model_t* pModel, int frame, float yaw) { return NULL; }
void R_DrawSpriteModel(cl_entity_t* e) { }

// gl_studio.c
void R_StudioInit(void) { }
void R_StudioLerpMovement(cl_entity_t* e, double time, vec3_t origin, vec3_t angles) { }
struct mstudiotex_s* R_StudioGetTexture(cl_entity_t* e) { return NULL; }
int R_GetEntityRenderMode(cl_entity_t* ent) { return 0; }
void R_DrawStudioModel(cl_entity_t* e) { }
player_info_t* pfnPlayerInfo(int index) { return NULL; }
void R_GatherPlayerLight(void) { }
float R_StudioEstimateFrame(cl_entity_t* e, mstudioseqdesc_t* pseqdesc, double time) { return 0.0f; }
void R_StudioResetPlayerModels(void) { }
void CL_InitStudioAPI(void) { }
void Mod_StudioLoadTextures(model_t* mod, void* data) { }
void Mod_StudioUnloadTextures(void* data) { }
void R_RunViewmodelEvents() { }

// gl_alias.c
void Mod_LoadAliasModel(model_t* mod, const void* buffer, qboolean* loaded) { }
void R_DrawAliasModel(cl_entity_t* e) { }
void R_AliasInit(void) { }

// gl_warp.c
void R_AddSkyBoxSurface(msurface_t* fa) { }
void R_ClearSkyBox(void) { }
void R_DrawSkyBox(void) { }
void R_DrawClouds(void) { }
void R_UnloadSkybox(void) { }
void EmitWaterPolys(msurface_t* warp, qboolean reverse, qboolean ripples) { }
void R_ResetRipples(void) { }
void R_AnimateRipples(void) { }
qboolean R_UploadRipples(texture_t* image) { return false; }

// gl_opengl.c
void GL_CheckForErrors_(const char* filename, const int fileline) { }
const char* GL_ErrorString(int err) { return NULL; }

// gl_triapi.c
void TriRenderMode(int mode) { }
void TriBegin(int mode) { }
void TriEnd(void) { }
void TriTexCoord2f(float u, float v) { }
void TriVertex3fv(const float* v) { }
void TriVertex3f(float x, float y, float z) { }
void _TriColor4f(float r, float g, float b, float a) { }
void _TriColor4ub(byte r, byte g, byte b, byte a) { }
void TriColor4f(float r, float g, float b, float a) { }
void TriColor4ub(byte r, byte g, byte b, byte a) { }
void TriBrightness(float brightness) { }
int TriWorldToScreen(const float* world, float* screen) { return 0; }
int TriSpriteTexture(model_t* pSpriteModel, int frame) { return 0; }
void TriFog(float flFogColor[3], float flStart, float flEnd, int bOn) { }
void TriGetMatrix(const int pname, float* matrix) { }
void TriFogParams(float flDensity, int iFogSkybox) { }
void TriCullFace(TRICULLSTYLE mode) { }

// Define default empty implementations for all function pointers in ref_interface_t
qboolean R_Init(void)
{
    gEngfuncs.R_Init_Video(REF_GL4);

    if (gladLoadGL() == 0) {
        gEngfuncs.Con_Printf(S_ERROR "GLAD failed to load OpenGL\n");
        return false;
    }
    gEngfuncs.Con_Printf("GLAD loaded\n");
    gEngfuncs.Con_Printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);

    glViewport(0, 0, gpGlobals->width, gpGlobals->height);

    R_InitImages();
    R_SpriteInit();
    R_StudioInit();
    R_AliasInit();
    R_ClearDecals();
    R_ClearScene();

    return true;
}
void R_Shutdown(void) { }
const char* R_GetConfigName(void) { return "ref_gl4"; }
qboolean R_SetDisplayTransform(ref_screen_rotation_t rotate, int x, int y, float scale_x, float scale_y) { return false; }
void GL_SetupAttributes(int safegl) { }
void GL_InitExtensions(void)
{
}
void GL_ClearExtensions(void) { }
void R_GammaChanged(qboolean do_reset_gamma) { }
void R_BeginFrame(qboolean clearScene)
{
    // gEngfuncs.Con_Printf("Begin Frame\n");

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gEngfuncs.CL_ExtraUpdate();
}
void R_EndFrame(void)
{
    gEngfuncs.GL_SwapBuffers();
}
void R_ClearScreen(void) { }
qboolean R_AddEntity(struct cl_entity_s* clent, int type) { return false; }
void CL_AddCustomBeam(cl_entity_t* pEnvBeam) { }
void R_ProcessEntData(qboolean allocate, cl_entity_t* entities, unsigned int max_entities) { }
void R_Flush(unsigned int flush_flags) { }
const byte* R_GetTextureOriginalBuffer(unsigned int idx) { return NULL; }
void R_SetupSky(int* skyboxTextures) { }
void R_DrawStretchRaw(float x, float y, float w, float h, int cols, int rows, const byte* data, qboolean dirty) { }
void R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, int texnum) { }
void FillRGBA(int rendermode, float x, float y, float w, float h, byte r, byte g, byte b, byte a) { }
int WorldToScreen(const vec3_t world, vec3_t screen) { return 0; }
qboolean VID_ScreenShot(const char* filename, int shot_type) { return false; }
qboolean VID_CubemapShot(const char* base, uint size, const float* vieworg, qboolean skyshot) { return false; }
void R_DecalShoot(int textureIndex, int entityIndex, int modelIndex, vec3_t pos, int flags, float scale) { }
void R_DecalRemoveAll(int texture) { }
int R_CreateDecalList(struct decallist_s* pList) { return 0; }
void R_ClearAllDecals(void) { }
void R_SetSkyCloudsTextures(int solidskyTexture, int alphaskyTexture) { }
void R_GetSpriteParms(int* frameWidth, int* frameHeight, int* numFrames, int currentFrame, const model_t* pSprite) { }
int R_GetSpriteTexture(const model_t* m_pSpriteModel, int frame) { return 0; }
qboolean Mod_ProcessRenderData(model_t* mod, qboolean create, const byte* buffer) { return false; }
int RefGetParm(int parm, int arg) { return 0; }
void GetDetailScaleForTexture(int texture, float* xScale, float* yScale) { }
void GetExtraParmsForTexture(int texture, byte* red, byte* green, byte* blue, byte* alpha) { }
float GetFrameTime(void) { return 0.0f; }
void R_SetCurrentEntity(struct cl_entity_s* ent) { }
void R_SetCurrentModel(struct model_s* mod) { }
const char* GL_TextureName(unsigned int texnum) { return NULL; }
const byte* GL_TextureData(unsigned int texnum) { return NULL; }
void R_OverrideTextureSourceSize(unsigned int texnum, unsigned int srcWidth, unsigned int srcHeight) { }
void AVI_UploadRawFrame(int texture, int cols, int rows, int width, int height, const byte* data) { }
void GL_LoadTextureMatrix(const float* glmatrix) { }
void GL_TexMatrixIdentity(void) { }
void GL_Reserved0(void) { }
void GL_Reserved1(void) { }
void GL_DrawParticles(const struct ref_viewpass_s* rvp, qboolean trans_pass, float frametime) { }
colorVec LightVec(const float* start, const float* end, float* lightspot, float* lightvec) { return (colorVec) { 0, 0, 0, 0 }; }
struct mstudiotex_s* StudioGetTexture(struct cl_entity_s* e) { return NULL; }
void GL_RenderFrame(const struct ref_viewpass_s* rvp) { }
void GL_OrthoBounds(const float* mins, const float* maxs) { }
qboolean R_SpeedsMessage(char* out, size_t size) { return false; }
byte* Mod_GetCurrentVis(void) { return NULL; }
void R_NewMap(void) { }
void* R_GetProcAddress(const char* name) { return NULL; }
void Begin(int primitiveCode) { }
void End(void) { }
void Color4f(float r, float g, float b, float a) { }
void Color4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) { }
void TexCoord2f(float u, float v) { }
void Vertex3fv(const float* worldPnt) { }
void Vertex3f(float x, float y, float z) { }
void Fog(float flFogColor[3], float flStart, float flEnd, int bOn) { }
void ScreenToWorld(const float* screen, float* world) { }
void GetMatrix(const int pname, float* matrix) { }
void FogParams(float flDensity, int iFogSkybox) { }
void CullFace(TRICULLSTYLE mode) { }
void VGUI_SetupDrawing(qboolean rect) { }
void VGUI_UploadTextureBlock(int drawX, int drawY, const byte* rgba, int blockWidth, int blockHeight) { }

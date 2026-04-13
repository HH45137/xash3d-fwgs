/*
 * vk_local.h - Vulkan renderer local declarations
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

#ifndef VK_LOCAL_H
#define VK_LOCAL_H

#if XASH_VULKAN

#include "ref_common.h"
#include "port.h"
#include "xash3d_types.h"
#include "cvardef.h"
#include "protocol.h"
#include "ref_params.h"
#include "enginefeatures.h"
#include "com_strings.h"
#include "cvardef.h"
#include "vk_export.h"
#include "wadfile.h"
#include "common/mod_local.h"
#include "pmove.h"
#include "gl_frustum.h"
#include "studio.h"

#include <vulkan/vulkan.h>

// ============================================================================
// Vulkan-specific Constants
// ============================================================================

#define VK_MAX_TEXTURES      8192
#define VK_MAX_LIGHTMAPS     256
#define VK_MAX_DRAW_STACK    2
#define VK_SUBDIVIDE_SIZE    64
#define VK_MAX_DECAL_SURFS   4096

// Frame buffering
#define VK_NUM_FRAMES       2

// Texture units
#define VK_MAX_TEXTURE_UNITS 32

// ============================================================================
// Vulkan Core Structures
// ============================================================================

// Texture structure
typedef struct vktexture_s
{
	char            name[256];   // game path, including extension
	word            srcWidth;    // keep unscaled sizes
	word            srcHeight;
	word            width;       // upload width\height
	word            height;
	word            depth;       // texture depth or count of layers for 2D_ARRAY
	byte            numMips;     // mipmap count

	VkImage         image;       // Vulkan image handle
	VkImageView     imageView;   // Vulkan image view
	VkDeviceMemory  memory;       // device memory
	VkSampler       sampler;     // texture sampler

	VkFormat        format;      // uploaded format
	texFlags_t      flags;

	rgba_t          fogParams;   // some water textures contain info about underwater fog
	rgbdata_t      *original;    // keep original image

	size_t          size;        // upload size for debug targets

	float           xscale;      // detail textures stuff
	float           yscale;

	uint            hashValue;
	struct vktexture_s *nextHash;
} vk_texture_t;

// Render context structure
typedef struct vk_context_s
{
	// Instance and device
	VkInstance              instance;
	VkPhysicalDevice        physicalDevice;
	VkDevice               device;

	// Queue family indices
	uint32_t                graphicsQueueFamily;
	uint32_t                presentQueueFamily;

	// Queues
	VkQueue                 graphicsQueue;
	VkQueue                 presentQueue;

	// Swap chain
	VkSwapchainKHR          swapChain;
	VkFormat                swapChainImageFormat;
	VkExtent2D              swapChainExtent;
	VkImage                *swapChainImages;
	uint32_t                swapChainImageCount;

	// Command pools
	VkCommandPool           commandPool;
	VkCommandPool           transientCommandPool;

	// Depth buffer
	VkImage                 depthImage;
	VkDeviceMemory          depthMemory;
	VkImageView             depthImageView;

	// Render pass
	VkRenderPass            renderPass;

	// Framebuffers
	VkFramebuffer          *framebuffers;

	// Descriptor pool
	VkDescriptorPool        descriptorPool;

	// Pipeline cache
	VkPipelineCache         pipelineCache;

	qboolean                initialized;
	qboolean                validationEnabled;
} vk_context_t;

// Frame data for triple buffering
typedef struct vk_frame_s
{
	VkCommandBuffer     commandBuffer;
	VkSemaphore         imageAcquired;
	VkSemaphore         renderComplete;
	VkFence             inFlight;

	// Per-frame descriptor sets
	VkDescriptorSet     descriptorSet;
} vk_frame_t;

// Backend state
typedef struct vk_backend_s
{
	vk_frame_t           frames[VK_NUM_FRAMES];
	uint32_t             currentFrame;

	// Descriptor set layout
	VkDescriptorSetLayout globalDescriptorSetLayout;
	VkDescriptorSetLayout textureDescriptorSetLayout;

	// Pipeline layouts
	VkPipelineLayout      opaquePipelineLayout;
	VkPipelineLayout      alphaPipelineLayout;

	// Pipelines
	VkPipeline           opaquePipeline;       // opaque geometry
	VkPipeline           alphaTestPipeline;   // alpha test (decal, etc)
	VkPipeline           alphaBlendPipeline; // alpha blend
	VkPipeline           particlePipeline;   // particles
	VkPipeline           skyPipeline;        // skybox
	VkPipeline           lightmapPipeline;   // lightmapped surfaces

	// Vertex buffers
	VkBuffer              vertexBuffer;
	VkDeviceMemory        vertexMemory;
	VkBuffer              indexBuffer;
	VkDeviceMemory        indexMemory;

	// Uniform buffers
	VkBuffer              uniformBuffer;
	VkDeviceMemory        uniformMemory;
	void                 *uniformMapped;

	// Current render state
	int                   renderMode;
	qboolean              depthWriteEnabled;
	qboolean              blendingEnabled;
	VkBlendFactor         srcBlend;
	VkBlendFactor         dstBlend;

	// Texture array for binding
	vk_texture_t         *boundTextures[VK_MAX_TEXTURE_UNITS];
	int                  activeTextureUnit;
} vk_backend_t;

// ============================================================================
// Vulkan Renderer State
// ============================================================================

// refparams similar to GL
#define VK_RP_NONE          0
#define VK_RP_ENVVIEW       BIT(0)
#define VK_RP_OLDVIEWLEAF   BIT(1)
#define VK_RP_CLIPPLANE     BIT(2)

#define VK_RP_NONVIEWERREF   (VK_RP_ENVVIEW)

// Culling
#define VK_CULL_VISIBLE     0
#define VK_CULL_BACKSIDE    1
#define VK_CULL_FRUSTUM     2
#define VK_CULL_VISFRAME    3
#define VK_CULL_OTHER       4

// refdef_t similar to GL
typedef struct vk_refdef_s
{
	int                  params;

	qboolean             drawWorld;
	qboolean             isSkyVisible;
	qboolean             onlyClientDraw;
	qboolean             drawOrtho;

	float                fov_x, fov_y;

	cl_entity_t          *currententity;
	model_t             *currentmodel;
	cl_entity_t          *currentbeam;

	int                  viewport[4];
	gl_frustum_t         frustum;

	mleaf_t             *viewleaf;
	mleaf_t             *oldviewleaf;
	vec3_t               pvsorigin;
	vec3_t               vieworg;
	vec3_t               viewangles;
} vk_refdef_t;

// ============================================================================
// Global Variables Declaration
// ============================================================================

// Vulkan context
extern vk_context_t      vk;
extern vk_backend_t       vkB;
extern vk_refdef_t        vkR;

// ============================================================================
// Function Prototypes - Minimal stubs for interface
// ============================================================================

// VK_main.c - Entry point
qboolean VK_Init( void );
void VK_Shutdown( void );
const char *VK_GetConfigName( void );
qboolean VK_SetDisplayTransform( ref_screen_rotation_t rotate, int x, int y, float scale_x, float scale_y );

// VK_context.c - Context management
void VK_GL_SetupAttributes( int safegl );
void VK_GL_InitExtensions( void );
void VK_GL_ClearExtensions( void );

// VK_image.c - Texture management
int  VK_GL_LoadTextureFromBuffer( const char *name, rgbdata_t *pic, texFlags_t flags, qboolean update );
void VK_GL_ProcessTexture( int texnum, float gamma, int topColor, int bottomColor );
const byte *VK_R_GetTextureOriginalBuffer( unsigned int idx );
void VK_R_SetupSky( int *skyboxTextures );

// VK_backend.c - Rendering backend
void VK_R_ClearScreen( void );
void VK_R_AllowFog( qboolean allow );
void VK_GL_SetRenderMode( int renderMode );
void VK_GL_BackendStartFrame( void );
void VK_GL_BackendEndFrame( void );
void VK_R_PushScene( void );
void VK_R_PopScene( void );

// VK_rmain.c - Main rendering
void VK_R_GammaChanged( qboolean do_reset_gamma );
void VK_R_BeginFrame( qboolean clearScene );
void VK_R_RenderScene( void );
void VK_R_EndFrame( void );

// VK_alias.c - Alias model rendering
qboolean VK_R_AddEntity( struct cl_entity_s *clent, int type );
void VK_R_ProcessEntData( qboolean allocate, cl_entity_t *entities, unsigned int max_entities );
void VK_R_Flush( unsigned int flush_flags );

// VK_studio.c - Studio model rendering
void VK_CL_InitStudioAPI( void );
float VK_R_StudioEstimateFrame( cl_entity_t *e, mstudioseqdesc_t *pseqdesc, double time );
void VK_R_StudioLerpMovement( cl_entity_t *e, double time, vec3_t origin, vec3_t angles );

// VK_particles.c - Particle system
void VK_CL_DrawParticles( double frametime, particle_t *particles, float partsize );
void VK_CL_DrawTracers( double frametime, particle_t *tracers );
void VK_CL_DrawBeams( int fTrans, BEAM *beams );
qboolean VK_R_BeamCull( const vec3_t start, const vec3_t end, qboolean pvsOnly );

// VK_sprite.c - Sprite rendering
struct mspriteframe_s *VK_R_GetSpriteFrame( const struct model_s *pModel, int frame, float yaw );

// VK_warp.c - Sky and water
void VK_R_SetSkyCloudsTextures( int solidskyTexture, int alphaskyTexture );

// VK_draw.c - 2D rendering
void VK_R_Set2DMode( qboolean enable );
void VK_R_DrawStretchRaw( float x, float y, float w, float h, int cols, int rows, const byte *data, qboolean dirty );
void VK_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, int texnum );
void VK_FillRGBA( int rendermode, float x, float y, float w, float h, byte r, byte g, byte b, byte a );

// VK_beams.c - Beam rendering
void VK_R_DecalShoot( int textureIndex, int entityIndex, int modelIndex, vec3_t pos, int flags, float scale );
void VK_R_DecalRemoveAll( int texture );
int  VK_R_CreateDecalList( struct decallist_s *pList );
void VK_R_ClearAllDecals( void );

// ============================================================================
// Stubs for missing functions (implement as needed)
// ============================================================================

// Debug
void VK_R_ShowTextures( void );

// Screen transforms
int VK_WorldToScreen( const vec_t *world, vec_t *screen );

// Screenshots
qboolean VK_VID_ScreenShot( const char *filename, int shot_type );
qboolean VK_VID_CubemapShot( const char *base, uint size, const float *vieworg, qboolean skyshot );

// Lighting
colorVec VK_R_LightPoint( const float *p );

// BModel
void VK_R_SubdivideSurface( model_t *mod, msurface_t *fa );
void VK_CL_RunLightStyles( lightstyle_t *ls );

// Model management
qboolean VK_Mod_ProcessRenderData( model_t *mod, qboolean create, const byte *buffer, size_t buffersize );
void VK_Mod_StudioLoadTextures( model_t *mod, void *data );

// Xash3D Render Interface
int VK_RefGetParm( int parm, int arg );
void VK_GetDetailScaleForTexture( int texture, float *xScale, float *yScale );
void VK_GetExtraParmsForTexture( int texture, byte *red, byte *green, byte *blue, byte *alpha );
float VK_GetFrameTime( void );

// Set renderer info
void VK_R_SetCurrentEntity( struct cl_entity_s *ent );
void VK_R_SetCurrentModel( struct model_s *mod );

// Texture tools
int VK_GL_FindTexture( const char *name );
const char *VK_GL_TextureName( unsigned int texnum );
const byte *VK_GL_TextureData( unsigned int texnum );
int VK_GL_LoadTexture( const char *name, const byte *buf, size_t size, int flags );
int VK_GL_CreateTexture( const char *name, int width, int height, const void *data, texFlags_t flags );
int VK_GL_LoadTextureArray( const char **names, int count );
int VK_GL_CreateTextureArray( const char *name, int width, int height, int depth, const void *data, texFlags_t flags );
void VK_GL_FreeTexture( unsigned int texnum );
void VK_R_OverrideTextureSourceSize( unsigned int texnum, unsigned int width, unsigned int height );

// Decals
void VK_DrawSingleDecal( decal_t *pDecal, msurface_t *fa );
float *VK_R_DecalSetupVerts( decal_t *pDecal, msurface_t *fa, int numPoints, int *padding );
void VK_R_EntityRemoveDecals( model_t *mod );

// AVI
void VK_AVI_UploadRawFrame( int texture, int cols, int rows, int width, int height, const byte *data );

// Texture state
void VK_GL_Bind( int texnum, unsigned int texid );
void VK_GL_SelectTexture( int texunit );
void VK_GL_LoadTextureMatrix( const float *matrix );
void VK_GL_TexMatrixIdentity( void );
void VK_GL_CleanUpTextureUnits( int );
void VK_GL_TexGen( unsigned int mode, unsigned int func );
void VK_GL_TextureTarget( unsigned int target );
void VK_GL_TexCoordArrayMode( unsigned int mode );
void VK_GL_UpdateTexSize( int texnum, int width, int height );

// Particles and lighting
void VK_GL_DrawParticles( const ref_viewpass_t *pass, qboolean trans_pass, float frametime );
colorVec VK_LightVec( const float *start, const float *end, float *lightspot, float *lightvec );
mstudiotexture_t *VK_StudioGetTexture( cl_entity_t *e );
void VK_GL_RenderFrame( const ref_viewpass_t *rvp );
void *VK_R_GetProcAddress( const char *name );

// VGUI
void VK_VGUI_SetupDrawing( qboolean enable );
void VK_VGUI_UploadTextureBlock( int texture, int x, const byte *data, int width, int height );

#endif // XASH_VULKAN

#endif // VK_LOCAL_H

#include "gl4_local.h"

CVAR_DEFINE( gl_extensions, "gl_allow_extensions", "1", FCVAR_GLCONFIG|FCVAR_READ_ONLY, "allow gl_extensions" );
CVAR_DEFINE( gl_texture_anisotropy, "gl_anisotropy", "8", FCVAR_GLCONFIG, "textures anisotropic filter" );
CVAR_DEFINE_AUTO( gl_texture_lodbias, "0.0", FCVAR_GLCONFIG, "LOD bias for mipmapped textures (perfomance|quality)" );
CVAR_DEFINE_AUTO( gl_texture_nearest, "0", FCVAR_GLCONFIG, "disable texture filter" );
CVAR_DEFINE_AUTO( gl_lightmap_nearest, "0", FCVAR_GLCONFIG, "disable lightmap filter" );
CVAR_DEFINE_AUTO( gl_keeptjunctions, "1", FCVAR_GLCONFIG, "removing tjuncs causes blinking pixels" );
CVAR_DEFINE_AUTO( gl_check_errors, "1", FCVAR_GLCONFIG, "ignore video engine errors" );
CVAR_DEFINE_AUTO( gl_polyoffset, "2.0", FCVAR_GLCONFIG, "polygon offset for decals" );
CVAR_DEFINE_AUTO( gl_wireframe, "0", FCVAR_GLCONFIG|FCVAR_SPONLY, "show wireframe overlay" );
CVAR_DEFINE_AUTO( gl_finish, "0", FCVAR_GLCONFIG, "use glFinish instead of glFlush" );
CVAR_DEFINE_AUTO( gl_nosort, "0", FCVAR_GLCONFIG, "disable sorting of translucent surfaces" );
CVAR_DEFINE_AUTO( gl_test, "0", 0, "engine developer cvar for quick testing new features" );
CVAR_DEFINE_AUTO( gl_msaa, "1", FCVAR_GLCONFIG, "enable or disable multisample anti-aliasing" );
CVAR_DEFINE_AUTO( gl_stencilbits, "8", FCVAR_GLCONFIG|FCVAR_READ_ONLY, "pixelformat stencil bits (0 - auto)" );
CVAR_DEFINE_AUTO( gl_overbright, "1", FCVAR_GLCONFIG, "overbrights" );
CVAR_DEFINE_AUTO( gl_fog, "1", FCVAR_GLCONFIG, "allow for rendering fog using built-in OpenGL fog implementation" );
CVAR_DEFINE_AUTO( r_lighting_extended, "1", FCVAR_GLCONFIG, "allow to get lighting from world and bmodels" );
CVAR_DEFINE_AUTO( r_lighting_ambient, "0.3", FCVAR_GLCONFIG, "map ambient lighting scale" );
CVAR_DEFINE_AUTO( r_detailtextures, "1", FCVAR_GLCONFIG, "enable detail textures support" );
CVAR_DEFINE_AUTO( r_novis, "0", 0, "ignore vis information (perfomance test)" );
CVAR_DEFINE_AUTO( r_nocull, "0", 0, "ignore frustrum culling (perfomance test)" );
CVAR_DEFINE_AUTO( r_lockpvs, "0", FCVAR_CHEAT, "lockpvs area at current point (pvs test)" );
CVAR_DEFINE_AUTO( r_lockfrustum, "0", FCVAR_CHEAT, "lock frustrum area at current point (cull test)" );
CVAR_DEFINE_AUTO( r_traceglow, "0", FCVAR_GLCONFIG, "cull flares behind models" );
CVAR_DEFINE_AUTO( gl_round_down, "2", FCVAR_GLCONFIG|FCVAR_READ_ONLY, "round texture sizes to nearest POT value" );
CVAR_DEFINE( r_vbo, "gl_vbo", "0", FCVAR_GLCONFIG, "draw world using VBO (known to be glitchy)" );
CVAR_DEFINE( r_vbo_detail, "gl_vbo_detail", "0", FCVAR_GLCONFIG, "detail vbo mode (0: disable, 1: multipass, 2: singlepass, broken decal dlights)" );
CVAR_DEFINE( r_vbo_dlightmode, "gl_vbo_dlightmode", "1", FCVAR_GLCONFIG, "vbo dlight rendering mode (0-1)" );
CVAR_DEFINE( r_vbo_overbrightmode, "gl_vbo_overbrightmode", "0", FCVAR_GLCONFIG, "vbo overbright rendering mode (0-1)" );
CVAR_DEFINE_AUTO( r_ripple, "0", FCVAR_GLCONFIG, "enable software-like water texture ripple simulation" );
CVAR_DEFINE_AUTO( r_ripple_updatetime, "0.05", FCVAR_GLCONFIG, "how fast ripple simulation is" );
CVAR_DEFINE_AUTO( r_ripple_spawntime, "0.1", FCVAR_GLCONFIG, "how fast new ripples spawn" );
CVAR_DEFINE_AUTO( r_large_lightmaps, "0", FCVAR_GLCONFIG|FCVAR_LATCH, "enable larger lightmap atlas textures (might break custom renderer mods)" );
CVAR_DEFINE_AUTO( r_dlight_virtual_radius, "3", FCVAR_GLCONFIG, "increase dlight radius virtually by this amount, should help against ugly cut off dlights on highly scaled textures" );

DEFINE_ENGINE_SHARED_CVAR_LIST()

poolhandle_t r_temppool;

gl_globals_t	tr;
glconfig_t	glConfig;
glstate_t	glState;
glwstate_t	glw_state;


/*
========================
DebugCallback

For ARB_debug_output
========================
*/
static void APIENTRY GL_DebugOutput( GLuint source, GLuint type, GLuint id, GLuint severity, GLint length, const GLcharARB *message, GLvoid *userParam )
{
}

/*
=================
GL_SetExtension
=================
*/
static void GL_SetExtension( int r_ext, int enable )
{
}

/*
=================
GL_CheckExtension
=================
*/
static qboolean GL_CheckExtension( const char *name, const dllfunc_t *funcs, size_t num_funcs, const char *cvarname, int r_ext, float minver )
{
	return false;
}

/*
==============
GL_GetProcAddress

defined just for nanogl/glwes, so it don't link to SDL2 directly, nor use dlsym
==============
*/
void GAME_EXPORT *GL_GetProcAddress( const char *name ); // keep defined for nanogl/wes
void GAME_EXPORT *GL_GetProcAddress( const char *name )
{
	return gEngfuncs.GL_GetProcAddress( name );
}

/*
===============
GL_SetDefaultTexState
===============
*/
static void GL_SetDefaultTexState( void )
{
}

/*
===============
GL_SetDefaultState
===============
*/
static void GL_SetDefaultState( void )
{
}

/*
===============
GL_SetDefaults
===============
*/
static void GL_SetDefaults( void )
{
}


/*
=================
R_RenderInfo_f
=================
*/
static void R_RenderInfo_f( void )
{
}


void GL_InitExtensions( void )
{
}

//=======================================================================

/*
=================
GL_InitCommands
=================
*/
static void GL_InitCommands( void )
{
	RETRIEVE_ENGINE_SHARED_CVAR_LIST();

	gEngfuncs.Cvar_RegisterVariable( &r_lighting_extended );
	gEngfuncs.Cvar_RegisterVariable( &r_lighting_ambient );
	gEngfuncs.Cvar_RegisterVariable( &r_novis );
	gEngfuncs.Cvar_RegisterVariable( &r_nocull );
	gEngfuncs.Cvar_RegisterVariable( &r_detailtextures );
	gEngfuncs.Cvar_RegisterVariable( &r_lockpvs );
	gEngfuncs.Cvar_RegisterVariable( &r_lockfrustum );
	gEngfuncs.Cvar_RegisterVariable( &r_traceglow );
	gEngfuncs.Cvar_RegisterVariable( &r_studio_sort_textures );
	gEngfuncs.Cvar_RegisterVariable( &r_studio_drawelements );
	gEngfuncs.Cvar_RegisterVariable( &r_ripple );
	gEngfuncs.Cvar_RegisterVariable( &r_ripple_updatetime );
	gEngfuncs.Cvar_RegisterVariable( &r_ripple_spawntime );
	gEngfuncs.Cvar_RegisterVariable( &r_shadows );
	gEngfuncs.Cvar_RegisterVariable( &r_vbo );
	gEngfuncs.Cvar_RegisterVariable( &r_vbo_dlightmode );
	gEngfuncs.Cvar_RegisterVariable( &r_vbo_overbrightmode );
	gEngfuncs.Cvar_RegisterVariable( &r_vbo_detail );
	gEngfuncs.Cvar_RegisterVariable( &r_large_lightmaps );
	gEngfuncs.Cvar_RegisterVariable( &r_dlight_virtual_radius );

	gEngfuncs.Cvar_RegisterVariable( &gl_extensions );
	gEngfuncs.Cvar_RegisterVariable( &gl_texture_nearest );
	gEngfuncs.Cvar_RegisterVariable( &gl_lightmap_nearest );
	gEngfuncs.Cvar_RegisterVariable( &gl_check_errors );
	gEngfuncs.Cvar_RegisterVariable( &gl_texture_anisotropy );
	gEngfuncs.Cvar_RegisterVariable( &gl_texture_lodbias );
	gEngfuncs.Cvar_RegisterVariable( &gl_keeptjunctions );
	gEngfuncs.Cvar_RegisterVariable( &gl_finish );
	gEngfuncs.Cvar_RegisterVariable( &gl_nosort );
	gEngfuncs.Cvar_RegisterVariable( &gl_test );
	gEngfuncs.Cvar_RegisterVariable( &gl_wireframe );
	gEngfuncs.Cvar_RegisterVariable( &gl_msaa );
	gEngfuncs.Cvar_RegisterVariable( &gl_stencilbits );
	gEngfuncs.Cvar_RegisterVariable( &gl_round_down );
	gEngfuncs.Cvar_RegisterVariable( &gl_overbright );
	gEngfuncs.Cvar_RegisterVariable( &gl_fog );

	// these cvar not used by engine but some mods requires this
	gEngfuncs.Cvar_RegisterVariable( &gl_polyoffset );

	// make sure gl_vsync is checked after vid_restart
	SetBits( gl_vsync->flags, FCVAR_CHANGED );

	gEngfuncs.Cmd_AddCommand( "r_info", R_RenderInfo_f, "display renderer info" );
	gEngfuncs.Cmd_AddCommand( "timerefresh", SCR_TimeRefresh_f, "turn quickly and print rendering statistcs" );
}

/*
===============
R_CheckVBO

register VBO cvars and get default value
===============
*/
static void R_CheckVBO( void )
{
}

/*
=================
GL_RemoveCommands
=================
*/
static void GL_RemoveCommands( void )
{
	gEngfuncs.Cmd_RemoveCommand( "r_info" );
	gEngfuncs.Cmd_RemoveCommand( "timerefresh" );
}

/*
===============
R_Init
===============
*/
qboolean R_Init( void )
{
	return true;
}

/*
===============
R_Shutdown
===============
*/
void R_Shutdown( void )
{
}

void GL_SetupAttributes( int safegl )
{
	int context_flags = 0; // REFTODO!!!!!
	int samples = 0;

#if XASH_GLES
	gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_PROFILE_MASK, REF_GL_CONTEXT_PROFILE_ES );
	gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_EGL, 1 );
#if XASH_NANOGL
	gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_MAJOR_VERSION, 1 );
	gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_MINOR_VERSION, 1 );
#else // !XASH_NANOGL
	gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_MAJOR_VERSION, 2 );
	gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_MINOR_VERSION, 0 );
#endif

#elif XASH_GL4ES
	gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_PROFILE_MASK, REF_GL_CONTEXT_PROFILE_ES );
	gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_EGL, 1 );
	gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_MAJOR_VERSION, 2 );
	gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_MINOR_VERSION, 0 );
#else // GL1.x
	if( gEngfuncs.Sys_CheckParm( "-glcore" ))
	{
		SetBits( context_flags, FCONTEXT_CORE_PROFILE );

		gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_PROFILE_MASK, REF_GL_CONTEXT_PROFILE_CORE );
		gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_MAJOR_VERSION, 3 );
		gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_MINOR_VERSION, 3 );
	}
	else
	{
		if( !safegl )
			gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_PROFILE_MASK, REF_GL_CONTEXT_PROFILE_COMPATIBILITY );
		else
		{
			gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_MAJOR_VERSION, 1 );
			gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_MINOR_VERSION, 1 );
		}
	}
#endif // XASH_GLES

	if( gEngfuncs.Sys_CheckParm( "-gldebug" ))
	{
		gEngfuncs.Con_Reportf( "Creating an extended GL context for debug...\n" );
		SetBits( context_flags, FCONTEXT_DEBUG_ARB );
		gEngfuncs.GL_SetAttribute( REF_GL_CONTEXT_FLAGS, REF_GL_CONTEXT_DEBUG_FLAG );
		glw_state.extended = true;
	}

	if( safegl > SAFE_DONTCARE )
	{
		safegl = -1; // can't retry anymore, can only shutdown engine
		return;
	}

	gEngfuncs.Con_Printf( "Trying safe opengl mode %d\n", safegl );

	if( safegl == SAFE_DONTCARE )
		return;

	gEngfuncs.GL_SetAttribute( REF_GL_DOUBLEBUFFER, 1 );

	if( safegl < SAFE_NOACC )
		gEngfuncs.GL_SetAttribute( REF_GL_ACCELERATED_VISUAL, 1 );

	gEngfuncs.Con_Printf( "bpp %d\n", gpGlobals->desktopBitsPixel );

	if( safegl < SAFE_NOSTENCIL )
		gEngfuncs.GL_SetAttribute( REF_GL_STENCIL_SIZE, gl_stencilbits.value );

	if( safegl < SAFE_NOALPHA )
		gEngfuncs.GL_SetAttribute( REF_GL_ALPHA_SIZE, 8 );

	if( safegl < SAFE_NODEPTH )
		gEngfuncs.GL_SetAttribute( REF_GL_DEPTH_SIZE, 24 );
	else
		gEngfuncs.GL_SetAttribute( REF_GL_DEPTH_SIZE, 8 );

	if( safegl < SAFE_NOCOLOR )
	{
		if( gpGlobals->desktopBitsPixel >= 24 )
		{
			gEngfuncs.GL_SetAttribute( REF_GL_RED_SIZE, 8 );
			gEngfuncs.GL_SetAttribute( REF_GL_GREEN_SIZE, 8 );
			gEngfuncs.GL_SetAttribute( REF_GL_BLUE_SIZE, 8 );
		}
		else if( gpGlobals->desktopBitsPixel >= 16 )
		{
			gEngfuncs.GL_SetAttribute( REF_GL_RED_SIZE, 5 );
			gEngfuncs.GL_SetAttribute( REF_GL_GREEN_SIZE, 6 );
			gEngfuncs.GL_SetAttribute( REF_GL_BLUE_SIZE, 5 );
		}
		else
		{
			gEngfuncs.GL_SetAttribute( REF_GL_RED_SIZE, 3 );
			gEngfuncs.GL_SetAttribute( REF_GL_GREEN_SIZE, 3 );
			gEngfuncs.GL_SetAttribute( REF_GL_BLUE_SIZE, 2 );
		}
	}

	if( safegl < SAFE_NOMSAA )
	{
		switch( (int)gEngfuncs.pfnGetCvarFloat( "gl_msaa_samples" ))
		{
		case 2:
		case 4:
		case 8:
		case 16:
			samples = gEngfuncs.pfnGetCvarFloat( "gl_msaa_samples" );
			break;
		default:
			samples = 0; // don't use, because invalid parameter is passed
		}

		if( samples )
		{
			gEngfuncs.GL_SetAttribute( REF_GL_MULTISAMPLEBUFFERS, 1 );
			gEngfuncs.GL_SetAttribute( REF_GL_MULTISAMPLESAMPLES, samples );

			glConfig.max_multisamples = samples;
		}
		else
		{
			gEngfuncs.GL_SetAttribute( REF_GL_MULTISAMPLEBUFFERS, 0 );
			gEngfuncs.GL_SetAttribute( REF_GL_MULTISAMPLESAMPLES, 0 );

			glConfig.max_multisamples = 0;
		}
	}
	else
	{
		gEngfuncs.Cvar_Set( "gl_msaa_samples", "0" );
	}
}

void GL_OnContextCreated( void )
{
}

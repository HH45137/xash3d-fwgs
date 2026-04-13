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
 * GetRefAPI - Main entry point for the renderer DLL
 */
EXPORT int GetRefAPI( int version, ref_interface_t *pFunctionTable, ref_api_t *engfuncs, ref_globals_t *pGlobals )
{
	if( version != REF_API_VERSION )
	{
		return -1;
	}

	// Store engine function table
	gEngfuncs = *engfuncs;
	gVars = *pGlobals;

	// Initialize render interface
	pFunctionTable->R_Init = VK_Init;
	pFunctionTable->R_Shutdown = VK_Shutdown;

	return REF_API_VERSION;
}

#endif // XASH_VULKAN

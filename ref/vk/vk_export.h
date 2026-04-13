/*
 * vk_export.h - Vulkan API function definitions
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

#ifndef VK_EXPORT_H
#define VK_EXPORT_H

#if XASH_VULKAN

#include <vulkan/vulkan.h>

/*
 * VK_LoadFunctions - Load all Vulkan function pointers
 * Returns: true if all functions loaded successfully, false otherwise
 *
 * TODO: Implement actual function loading using vkGetInstanceProcAddr and vkGetDeviceProcAddr
 */
qboolean VK_LoadFunctions( VkInstance instance, VkDevice device );

/*
 * VK_GetInstanceProcAddr - Get instance-level Vulkan function
 */
void *VK_GetInstanceProcAddr( const char *name );

/*
 * VK_GetDeviceProcAddr - Get device-level Vulkan function
 */
void *VK_GetDeviceProcAddr( const char *name );

#endif // XASH_VULKAN

#endif // VK_EXPORT_H

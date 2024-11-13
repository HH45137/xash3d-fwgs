#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>

VkInstance Vk_CreateInstance( SDL_Window *hWnd );

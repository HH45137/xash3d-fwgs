#include "vk_until.h"

VkInstance Vk_CreateInstance( SDL_Window *hWnd )
{
	// SDL_Window *hWnd = NULL;

	VkApplicationInfo applicationInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		0,
		VK_NULL_HANDLE,
		0,
		VK_API_VERSION_1_3};

	const char layerList[][VK_MAX_EXTENSION_NAME_SIZE] = {
		"VK_LAYER_KHRONOS_validation"};
	const char *layers[] = {
		layerList[0]};

	uint32_t extensionNumber = 0;
	const char **extensions = NULL;

	if (SDL_Vulkan_GetInstanceExtensions(hWnd, &extensionNumber, NULL) != SDL_TRUE)
	{
		printf("Failed to get the number of Vulkan instance extensions: %s\n", SDL_GetError());
		return;
	}
	if (extensionNumber > 0)
	{
		extensions = (const char **)malloc(extensionNumber * sizeof(const char *));
		if (extensions == NULL)
		{
			printf("Failed to allocate memory for Vulkan instance extensions.\n");
			return;
		}
		if (SDL_Vulkan_GetInstanceExtensions(hWnd, &extensionNumber, extensions) != SDL_TRUE)
		{
			free(extensions);
			printf("Failed to get the Vulkan instance extensions: %s\n", SDL_GetError());
			return;
		}
	}

	VkInstanceCreateInfo instanceCreateInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		&applicationInfo,
		1,
		layers,
		extensionNumber,
		extensions};

	VkInstance instance;
	vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &instance);
	return instance;
}

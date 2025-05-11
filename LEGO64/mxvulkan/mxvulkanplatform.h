#pragma once

#include "mxvulkanfunctions.h"

#include <SDL_vulkan.h>

class MxVulkanPlatform
{
public:
	static bool LoadVulkanLibrary();
	static void ReleaseVulkanLibrary();

	static bool LoadVulkanInstanceFunctions(VkInstance instance);

	static VkSurfaceKHR CreateSurface(SDL_Window* WindowHandle, VkInstance VulkanInstance);
};

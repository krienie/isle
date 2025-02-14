#pragma once

#include "mxvulkanplatform.h"

#include <SDL_vulkan.h>

class MxVulkanDevice;

class MxVulkanSwapchain
{
public:
	MxVulkanSwapchain(VkInstance InVulkanInstance, MxVulkanDevice* InVulkanDevice);
	~MxVulkanSwapchain();

	bool Create(SDL_Window* WindowHandle);

	void Present();

private:
	VkSwapchainKHR Swapchain = nullptr;
	VkSurfaceKHR Surface = nullptr;
	VkInstance VulkanInstance = nullptr;
	MxVulkanDevice* VulkanDevice = nullptr;
};

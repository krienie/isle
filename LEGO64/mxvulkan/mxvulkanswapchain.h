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
	VkSwapchainKHR m_swapchain = nullptr;
	VkSurfaceKHR m_surface = nullptr;
	VkInstance m_vulkanInstance = nullptr;
	MxVulkanDevice* m_vulkanDevice = nullptr;

	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
};

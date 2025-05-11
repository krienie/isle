#pragma once

#include "mxvulkanplatform.h"
#include "mxvulkansemaphore.h"

#include <SDL_vulkan.h>

#include <memory>

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

	std::vector<std::unique_ptr<MxVulkanSemaphore>> m_imageSemaphores;
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
};

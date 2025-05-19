#pragma once

#include "mxvulkanplatform.h"
#include "mxvulkansemaphore.h"

#include <SDL_vulkan.h>

#include <memory>

class MxVulkanDevice;

class MxVulkanViewport
{
public:
	MxVulkanViewport(VkInstance InVulkanInstance, MxVulkanDevice* InVulkanDevice);
	~MxVulkanViewport();

	bool Create(SDL_Window* WindowHandle);

	void Present();

private:
	struct AqcuiredBufferInfo
	{
		int32_t CurrentBufferIndex = -1;
		MxVulkanSemaphore* CurrentBufferSemaphore = nullptr;
	};

	AqcuiredBufferInfo GetNextBufferIndex();

	VkSwapchainKHR m_swapchain = nullptr;
	VkSurfaceKHR m_surface = nullptr;
	VkInstance m_vulkanInstance = nullptr;
	MxVulkanDevice* m_vulkanDevice = nullptr;

	int32_t m_currentBufferIndex = -1;
	std::vector<std::unique_ptr<MxVulkanSemaphore>> m_imageSemaphores;
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
};

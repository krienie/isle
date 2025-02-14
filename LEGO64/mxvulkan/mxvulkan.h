#pragma once

#include "mxvulkandevice.h"
#include "mxvulkanplatform.h"
#include "mxvulkanswapchain.h"

#include <SDL2/SDL_video.h>

#include <memory>

class MxVulkan
{
public:
	MxVulkan() = default;
	~MxVulkan();

	bool InitForWindow(SDL_Window* window);
	void Shutdown();

private:
	VkInstance VulkanInstance = nullptr;
	VkQueue GraphicsQueue = nullptr;

	std::unique_ptr<MxVulkanDevice> VulkanDevice = nullptr;
	std::unique_ptr<MxVulkanSwapchain> Swapchain = nullptr;
};

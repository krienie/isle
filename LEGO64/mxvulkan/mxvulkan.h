#pragma once

#include "mxvulkandevice.h"
#include "mxvulkanplatform.h"
#include "MxVulkanViewport.h"

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
	VkInstance m_vulkanInstance = nullptr;
	VkQueue m_graphicsQueue = nullptr;

	std::unique_ptr<MxVulkanDevice> m_vulkanDevice = nullptr;
	std::unique_ptr<MxVulkanViewport> m_viewport = nullptr;
};

#pragma once

#include "mxvulkandevice.h"
#include "MxVulkanViewport.h"

#include <SDL2/SDL_video.h>

#include <memory>

class MxVulkanRHI
{
public:
	MxVulkanRHI() = default;
	~MxVulkanRHI();

	bool InitForWindow(SDL_Window* window);
	void Shutdown();

	MxVulkanDevice* GetVulkanDevice() const { return m_vulkanDevice.get(); }
	VkInstance GetVulkanInstance() const { return m_vulkanInstance; }

private:
	VkInstance m_vulkanInstance = nullptr;

	std::unique_ptr<MxVulkanDevice> m_vulkanDevice = nullptr;
};

#pragma once

#include "mxvulkanplatform.h"

#include <SDL_vulkan.h>

class MxVulkanDevice
{
public:
	MxVulkanDevice(VkInstance InVulkanInstance);
	~MxVulkanDevice();

	bool Create();
	VkDevice GetDeviceInstance() const { return m_vulkanDeviceInstance; }
	VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
	VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }

private:
	VkInstance m_vulkanInstance = nullptr;
	VkDevice m_vulkanDeviceInstance = nullptr;
	VkPhysicalDevice m_physicalDevice;

	VkQueue m_graphicsQueue = nullptr;

	int m_graphicsQueueIndex = -1;
};

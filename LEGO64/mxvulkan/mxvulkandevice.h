#pragma once

#include "mxvulkanplatform.h"

#include <SDL_vulkan.h>

class MxVulkanDevice
{
public:
	MxVulkanDevice(VkInstance InVulkanInstance);
	~MxVulkanDevice();

	bool Create();
	VkDevice GetDeviceInstance() const { return VulkanDeviceInstance; }
	VkPhysicalDevice GetPhysicalDevice() const { return PhysicalDevice; }
	VkQueue GetGraphicsQueue() const;

private:
	VkInstance VulkanInstance = nullptr;
	VkDevice VulkanDeviceInstance = nullptr;
	VkPhysicalDevice PhysicalDevice;

	int GraphicsQueueIndex = -1;
};

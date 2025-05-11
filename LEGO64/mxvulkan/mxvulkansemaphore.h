#pragma once

#include "mxvulkanfunctions.h"

class MxVulkanDevice;

class MxVulkanSemaphore
{
public:
	MxVulkanSemaphore(MxVulkanDevice* InVulkanDevice);
	~MxVulkanSemaphore();

	VkSemaphore GetHandle() const;

private:
	MxVulkanDevice* m_vulkanDevice;
	VkSemaphore m_semaphore = VK_NULL_HANDLE;
};

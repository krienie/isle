#pragma once

#include "mxvulkanfunctions.h"

class MxVulkanDevice;

class MxVulkanFence
{
public:
	MxVulkanFence(MxVulkanDevice* InVulkanDevice);
	~MxVulkanFence();

	bool IsSignaled() const;
	void Reset();
	bool WaitForSignal(uint64_t TimeInNanoseconds);
	bool WaitAndReset(uint64_t TimeInNanoseconds);

private:
	MxVulkanDevice* m_vulkanDevice;
	VkFence m_fenceHandle = VK_NULL_HANDLE;
	bool m_isSignaled = false;
};

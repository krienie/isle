#pragma once

#include "mxvulkanfunctions.h"

#include <SDL_vulkan.h>

class MxVulkanDevice;
class MxVulkanCommandPool;

class MxVulkanCommandBuffer
{
public:
	MxVulkanCommandBuffer(MxVulkanDevice* InVulkanDevice, MxVulkanCommandPool* InVulkanCommandBuffer);
	~MxVulkanCommandBuffer();

	void Begin() const;
	void End() const;

private:
	MxVulkanDevice* m_vulkanDevice = nullptr;
	MxVulkanCommandPool* m_vulkanCommandBuffer = nullptr;
	VkCommandBuffer m_commandBufferHandle = nullptr;
};

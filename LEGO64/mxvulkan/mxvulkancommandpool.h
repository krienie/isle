#pragma once

#include "mxvulkancommandbuffer.h"
#include "mxvulkanplatform.h"

#include <SDL_vulkan.h>

#include <memory>

class MxVulkanDevice;

class MxVulkanCommandPool
{
public:
	MxVulkanCommandPool(MxVulkanDevice* InVulkanDevice);
	~MxVulkanCommandPool();

	VkCommandPool GetHandle() const;

	MxVulkanCommandBuffer* CreateNewCommandBuffer();

private:
	MxVulkanDevice* m_vulkanDevice = nullptr;
	VkCommandPool m_commandPoolHandle = nullptr;
	std::vector<std::unique_ptr<MxVulkanCommandBuffer>> m_commandBuffers;
};

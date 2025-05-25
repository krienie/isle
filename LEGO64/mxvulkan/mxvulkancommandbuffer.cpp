#include "mxvulkancommandbuffer.h"

#include "mxvulkancommandpool.h"
#include "mxvulkandevice.h"

#include <iostream>

MxVulkanCommandBuffer::MxVulkanCommandBuffer(MxVulkanDevice* InVulkanDevice, MxVulkanCommandPool* InVulkanCommandBuffer)
	: m_vulkanDevice(InVulkanDevice), m_vulkanCommandBuffer(InVulkanCommandBuffer)
{
	assert(m_vulkanDevice);
	assert(m_vulkanCommandBuffer);

	VkCommandBufferAllocateInfo allocateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		m_vulkanCommandBuffer->GetHandle(),
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1u
	};

	VkResult result = vkAllocateCommandBuffers(m_vulkanDevice->GetDeviceInstance(), &allocateInfo, &m_commandBufferHandle);
	if (result != VK_SUCCESS)
	{
		std::cout << "Could not allocate command buffer.\n";
	}

	assert(m_commandBufferHandle);
}

MxVulkanCommandBuffer::~MxVulkanCommandBuffer()
{
	if (m_commandBufferHandle)
	{
		vkFreeCommandBuffers(m_vulkanDevice->GetDeviceInstance(), m_vulkanCommandBuffer->GetHandle(), 1, &m_commandBufferHandle);
		m_commandBufferHandle = nullptr;
	}
}

void MxVulkanCommandBuffer::Begin() const
{
	VkCommandBufferBeginInfo beginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr
	};

	VkResult result = vkBeginCommandBuffer(m_commandBufferHandle, &beginInfo);
	assert(result == VK_SUCCESS);
}

void MxVulkanCommandBuffer::End() const
{
	VkResult result = vkEndCommandBuffer(m_commandBufferHandle);
	assert(result == VK_SUCCESS);
}

void MxVulkanCommandBuffer::Reset() const
{
	VkResult result = vkResetCommandBuffer(m_commandBufferHandle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	assert(result == VK_SUCCESS);
}

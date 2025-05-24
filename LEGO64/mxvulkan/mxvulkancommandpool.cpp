#include "mxvulkancommandpool.h"

#include "mxvulkandevice.h"

#include <iostream>

MxVulkanCommandPool::MxVulkanCommandPool(MxVulkanDevice* InVulkanDevice)
	: m_vulkanDevice(InVulkanDevice)
{
	assert(m_vulkanDevice);

	VkCommandPoolCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		static_cast<uint32_t>(m_vulkanDevice->GetGraphicsQueueIndex())
	};

	VkResult result = vkCreateCommandPool(m_vulkanDevice->GetDeviceInstance(), &createInfo, nullptr, &m_commandPoolHandle);
	if (result != VK_SUCCESS)
	{
		std::cout << "Command pool creation failed!.\n";
	}
}

MxVulkanCommandPool::~MxVulkanCommandPool()
{
	m_commandBuffers.clear();

	if (m_commandPoolHandle)
	{
		vkDestroyCommandPool(m_vulkanDevice->GetDeviceInstance(), m_commandPoolHandle, nullptr);
	}
}

VkCommandPool MxVulkanCommandPool::GetHandle() const
{
	return m_commandPoolHandle;
}

MxVulkanCommandBuffer* MxVulkanCommandPool::CreateNewCommandBuffer()
{
	auto& NewBuffer = m_commandBuffers.emplace_back(new MxVulkanCommandBuffer(m_vulkanDevice, this));
	return NewBuffer.get();
}

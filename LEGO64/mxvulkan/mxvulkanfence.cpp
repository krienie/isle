#include "mxvulkanfence.h"

#include "mxvulkandevice.h"

#include <iostream>

MxVulkanFence::MxVulkanFence(MxVulkanDevice* InVulkanDevice)
	: m_vulkanDevice(InVulkanDevice)
{
	VkFenceCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		0
	};

	VkResult result = vkCreateFence(InVulkanDevice->GetDeviceInstance(), &createInfo, nullptr, &m_fenceHandle);
	if (result != VK_SUCCESS)
	{
		std::cout << "Could not create fence.\n";
	}
}

MxVulkanFence::~MxVulkanFence()
{
	if (m_fenceHandle)
	{
		vkDestroyFence(m_vulkanDevice->GetDeviceInstance(), m_fenceHandle, nullptr);
		m_fenceHandle = nullptr;
	}
}

bool MxVulkanFence::IsSignaled() const
{
	return m_isSignaled;
}

void MxVulkanFence::Reset()
{
	VkResult result = vkResetFences(m_vulkanDevice->GetDeviceInstance(), 1u, &m_fenceHandle);
	assert(result == VK_SUCCESS);
	m_isSignaled = false;
}

bool MxVulkanFence::WaitForSignal(uint64_t TimeInNanoseconds)
{
	VkResult result = vkWaitForFences(m_vulkanDevice->GetDeviceInstance(), 1, &m_fenceHandle, true, TimeInNanoseconds);

	if (result == VK_SUCCESS)
	{
		m_isSignaled = true;
		return true;
	}

	return false;
}

bool MxVulkanFence::WaitAndReset(uint64_t TimeInNanoseconds)
{
	const bool isSignaled = WaitForSignal(TimeInNanoseconds);
	if (isSignaled)
	{
		Reset();
	}

	return isSignaled;
}

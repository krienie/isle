#include "mxvulkansemaphore.h"

#include "mxvulkandevice.h"

#include <iostream>

MxVulkanSemaphore::MxVulkanSemaphore(MxVulkanDevice* InVulkanDevice)
	: m_vulkanDevice(InVulkanDevice)
{
	VkSemaphoreCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};

	VkResult result = vkCreateSemaphore(InVulkanDevice->GetDeviceInstance(), &createInfo, nullptr, &m_semaphore);
	if (result != VK_SUCCESS)
	{
		std::cout << "Could not create a semaphore.\n";
	}
}

MxVulkanSemaphore::~MxVulkanSemaphore()
{
	if (m_semaphore)
	{
		vkDestroySemaphore(m_vulkanDevice->GetDeviceInstance(), m_semaphore, nullptr);
		m_semaphore = nullptr;
	}
}

VkSemaphore MxVulkanSemaphore::GetHandle() const
{
	return m_semaphore;
}

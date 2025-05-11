#include "mxvulkandevice.h"

#include <iostream>
#include <unordered_map>
#include <cassert>

MxVulkanDevice::MxVulkanDevice(VkInstance InVulkanInstance)
	: m_vulkanInstance(InVulkanInstance), m_physicalDevice()
{
	assert(m_vulkanInstance);
}

MxVulkanDevice::~MxVulkanDevice()
{
	vkDestroyDevice(m_vulkanDeviceInstance, nullptr);
	m_vulkanDeviceInstance = nullptr;
}

bool MxVulkanDevice::Create()
{
	//TODO(KL): For completeness, we can integrate this device selection logic into the config app.
	unsigned int PhysicalDeviceCount = 0;
	VkResult Result = vkEnumeratePhysicalDevices(m_vulkanInstance, &PhysicalDeviceCount, nullptr);
	if (Result != VK_SUCCESS || PhysicalDeviceCount == 0)
	{
		return false;
	}

	std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
	vkEnumeratePhysicalDevices(m_vulkanInstance, &PhysicalDeviceCount, PhysicalDevices.data());

	std::unordered_map<VkPhysicalDeviceType, std::pair<int, VkPhysicalDeviceProperties>> DeviceProperties;
	DeviceProperties.reserve(PhysicalDeviceCount);
	for (int i = 0; i < static_cast<int>(PhysicalDevices.size()); ++i)
	{
		VkPhysicalDeviceProperties PhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(PhysicalDevices[i], &PhysicalDeviceProperties);

		DeviceProperties[PhysicalDeviceProperties.deviceType] = std::make_pair(i, PhysicalDeviceProperties);
	}

	// We prefer discrete GPU over integrated GPU.
	if (DeviceProperties.contains(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU))
	{
		m_physicalDevice = PhysicalDevices[DeviceProperties[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU].first];
	}
	else if (DeviceProperties.contains(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU))
	{
		m_physicalDevice = PhysicalDevices[DeviceProperties[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU].first];
	}
	else
	{
		// If neither discrete of integrated GPU is available, just take whatever was found.
		m_physicalDevice = PhysicalDevices[0];
	}

	// Query queues. We are only interested in the graphics queue.
	unsigned int QueueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &QueueCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueProperties(QueueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &QueueCount, QueueProperties.data());

	for (size_t i = 0; i < QueueProperties.size(); ++i)
	{
		if ((QueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
		{
			m_graphicsQueueIndex = static_cast<int>(i);
			break;
		}
	}

	if (m_graphicsQueueIndex < 0)
	{
		std::cout << "No suitable Vulkan graphics queue found.\n";
		return false;
	}

	// Create the logical vulkan device
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo QueueCreateInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		nullptr,
		0,
		static_cast<unsigned int>(m_graphicsQueueIndex),
		1u,
		&queuePriority
	};

	std::vector<char const *> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo DeviceCreateInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		1u,
		&QueueCreateInfo,
		0,
		nullptr,
		static_cast<uint32_t>(deviceExtensions.size()),
		deviceExtensions.data(),
		// No special features for now. Will add some once they are needed.
		nullptr
	};
	
	vkCreateDevice(m_physicalDevice, &DeviceCreateInfo, nullptr, &m_vulkanDeviceInstance);

	return true;
}

VkQueue MxVulkanDevice::GetGraphicsQueue() const
{
	if (m_graphicsQueueIndex < 0)
	{
		return nullptr;
	}

	VkQueue Queue;
	vkGetDeviceQueue(m_vulkanDeviceInstance, m_graphicsQueueIndex, 0, &Queue);
	return Queue;
}

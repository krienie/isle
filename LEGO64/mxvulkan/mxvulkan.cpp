
#include "mxvulkan.h"

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

MxVulkan::~MxVulkan()
{
	Shutdown();
}

bool MxVulkan::InitForWindow(SDL_Window* window)
{
	if (!VulkanPlatform::LoadVulkanLibrary())
	{
		return false;
	}

	unsigned extension_count;
	if (!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr))
	{
		std::cout << "Could not get the number of required instance extensions from SDL.\n";
		return false;
	}

	std::vector<const char*> extensions(extension_count);
	if (!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extensions.data()))
	{
		std::cout << "Could not get the names of required instance extensions from SDL.\n";
		return false;
	}

	// Use validation layers if this is a debug build
	std::vector<const char*> layers;
#if defined(_DEBUG)
	layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

	VkApplicationInfo appInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"Lego Island Vulkan",
		VK_MAKE_VERSION( 1, 0, 0 ),
		"Omni",
		VK_MAKE_VERSION( 1, 0, 0 ),
		//NOTE(KL): Set to version 1.3 for now as that is what my GPU supports. Will make something smarter later on.
		VK_API_VERSION_1_3
	};

	VkInstanceCreateInfo instInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&appInfo,
		static_cast<uint32_t>(layers.size()),
		layers.data(),
		static_cast<uint32_t>(extensions.size()),
		extensions.data()
	};

	VkResult Result = vkCreateInstance(&instInfo, nullptr, &Instance);

	if (Result != VK_SUCCESS)
	{
		std::cout << "Unable to create Vulkan Instance\n";
		return false;
	}

	VulkanPlatform::LoadVulkanInstanceFunctions(Instance);

	CreateDevice();

	return true;
}

void MxVulkan::Shutdown()
{
	if (Device && vkDestroyDevice)
	{
		vkDestroyDevice(Device, nullptr);
		Device = nullptr;
	}

	if (Instance && vkDestroyInstance)
	{
		vkDestroyInstance(Instance, nullptr);
		Instance = nullptr;
	}

	VulkanPlatform::ReleaseVulkanLibrary();
}

//TODO(KL): This should probably be in its own class where we can query any needed features while rendering.
bool MxVulkan::CreateDevice()
{
	//TODO(KL): For completeness, we can integrate this device selection logic into the config app.
	unsigned int PhysicalDeviceCount = 0;
	VkResult Result = vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr);
	if (Result != VK_SUCCESS || PhysicalDeviceCount == 0)
	{
		return false;
	}

	std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
	vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data());

	std::unordered_map<VkPhysicalDeviceType, std::pair<int, VkPhysicalDeviceProperties>> DeviceProperties;
	DeviceProperties.reserve(PhysicalDeviceCount);
	for (int i = 0; i < static_cast<int>(PhysicalDevices.size()); ++i)
	{
		VkPhysicalDeviceProperties PhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(PhysicalDevices[i], &PhysicalDeviceProperties);

		DeviceProperties[PhysicalDeviceProperties.deviceType] = std::make_pair(i, PhysicalDeviceProperties);
	}

	// We prefer discrete GPU over integrated GPU.
	VkPhysicalDevice SelectedDevice;
	if (DeviceProperties.contains(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU))
	{
		SelectedDevice = PhysicalDevices[DeviceProperties[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU].first];
	}
	else if (DeviceProperties.contains(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU))
	{
		SelectedDevice = PhysicalDevices[DeviceProperties[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU].first];
	}
	else
	{
		// If neither discrete of integrated GPU is available, just take whatever was found.
		SelectedDevice = PhysicalDevices[0];
	}

	// Query queues. We are only interested in the graphics queue.
	int GraphicsQueueIndex = -1;
	unsigned int QueueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(SelectedDevice, &QueueCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueProperties(QueueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(SelectedDevice, &QueueCount, QueueProperties.data());

	for (size_t i = 0; i < QueueProperties.size(); ++i)
	{
		if ((QueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
		{
			GraphicsQueueIndex = static_cast<int>(i);
			break;
		}
	}

	if (GraphicsQueueIndex < 0)
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
		static_cast<unsigned int>(GraphicsQueueIndex),
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
	
	vkCreateDevice(SelectedDevice, &DeviceCreateInfo, nullptr, &Device);

	return true;
}

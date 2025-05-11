#include "mxvulkanswapchain.h"

#include "mxvulkandevice.h"

#include <algorithm>
#include <iostream>

namespace
{
VkPresentModeKHR GetPresentationMode(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface)
{
	std::cout << "Setting swapchain presentation mode..\n";

	uint32_t numPresentationModes = 0;
	
	VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &numPresentationModes, nullptr);
	if (result != VK_SUCCESS)
	{
		// FIFO should always be supported, so just use that if for some reason we cannot get the presentation modes
		std::cout << "Error enumerating presentation modes. Defaulting to VK_PRESENT_MODE_FIFO_KHR\n";
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	std::vector<VkPresentModeKHR> presentationModes(numPresentationModes);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &numPresentationModes, presentationModes.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "Error enumerating presentation modes. Defaulting to VK_PRESENT_MODE_FIFO_KHR\n";
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	bool foundPresentModeMailbox = false;
	bool foundPresentModeImmediate = false;
	bool foundPresentModeFIFO = false;

	for (size_t i = 0; i < numPresentationModes; i++)
	{
		switch (presentationModes[i])
		{
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			foundPresentModeImmediate = true;
			std::cout << "Found VK_PRESENT_MODE_IMMEDIATE_KHR\n";
			break;
		case VK_PRESENT_MODE_MAILBOX_KHR:
			foundPresentModeMailbox = true;
			std::cout << "Found VK_PRESENT_MODE_MAILBOX_KHR\n";
			break;
		
		case VK_PRESENT_MODE_FIFO_KHR:
			foundPresentModeFIFO = true;
			std::cout << "Found VK_PRESENT_MODE_FIFO_KHR\n";
			break;
		default:
			std::cout << "Found presentation mode " << presentationModes[i] << '\n';
			break;
		}
	}

	//TODO(KL): check for vsync?
	constexpr bool useVsync = true;
	if (foundPresentModeImmediate && !useVsync)
	{
		std::cout << "Using VK_PRESENT_MODE_IMMEDIATE_KHR\n";
		return VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	if (foundPresentModeMailbox)
	{
		std::cout << "Using VK_PRESENT_MODE_MAILBOX_KHR\n";
		return VK_PRESENT_MODE_MAILBOX_KHR;
	}

	if (foundPresentModeFIFO)
	{
		std::cout << "Using VK_PRESENT_MODE_FIFO_KHR\n";
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	std::cout << "Using " << presentationModes[0] << '\n';
	return presentationModes[0];
}

VkExtent2D GetBufferSize(const VkSurfaceCapabilitiesKHR& SurfaceCapabilities)
{
	if (SurfaceCapabilities.currentExtent.width != 0xFFFFFFFF)
	{
		// Use the client surface size
		return SurfaceCapabilities.currentExtent;
	}

	// Fall-back to a default size
	uint32_t DefaultWidth = 640u;
	uint32_t DefaultHeight = 480u;
	uint32_t UsedWidth = std::clamp(DefaultWidth, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width);
	uint32_t UsedHeight = std::clamp(DefaultHeight, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);

	return {UsedWidth, UsedHeight};
}

VkSurfaceFormatKHR GetSurfaceFormat(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface)
{
	constexpr VkSurfaceFormatKHR defaultSurfaceFormat = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	//TODO(KL): Properly check for the surface format and support more formats. For now this will have to do.
	return defaultSurfaceFormat;

	//uint32_t numSurfaceFormats = 0u;
	//VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &numSurfaceFormats, nullptr);
	//if (result != VK_SUCCESS)
	//{
	//	std::cout << "Error getting surface formats. Using a default in the hopes that it works..\n";
	//	return defaultSurfaceFormat;
	//}
	//
	//std::vector<VkSurfaceFormatKHR> surfaceFormats(numSurfaceFormats);
	//result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &numSurfaceFormats, surfaceFormats.data());
	//if (result != VK_SUCCESS)
	//{
	//	std::cout << "Error getting surface formats. Using a default in the hopes that it works..\n";
	//	return defaultSurfaceFormat;
	//}
}
}

MxVulkanSwapchain::MxVulkanSwapchain(VkInstance InVulkanInstance, MxVulkanDevice* InVulkanDevice)
	: m_vulkanInstance(InVulkanInstance), m_vulkanDevice(InVulkanDevice)
{
	assert(m_vulkanInstance);
	assert(m_vulkanDevice);
}

MxVulkanSwapchain::~MxVulkanSwapchain()
{
	if (m_swapchain)
	{
		vkDestroySwapchainKHR(m_vulkanDevice->GetDeviceInstance(), m_swapchain, nullptr);
		m_swapchain = nullptr;
	}

	if (m_surface)
	{
		vkDestroySurfaceKHR(m_vulkanInstance, m_surface, nullptr);
		m_surface = nullptr;
	}
}

bool MxVulkanSwapchain::Create(SDL_Window* WindowHandle)
{
	m_surface = MxVulkanPlatform::CreateSurface(WindowHandle, m_vulkanInstance);
	if (!m_surface)
	{
		return false;
	}

	const VkPhysicalDevice physicalDevice = m_vulkanDevice->GetPhysicalDevice();

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &surfaceCapabilities);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	uint32_t numBuffers = std::min(surfaceCapabilities.minImageCount + 1u, surfaceCapabilities.maxImageCount);

	VkPresentModeKHR presentationMode = GetPresentationMode(physicalDevice, m_surface);
	VkExtent2D bufferSize = GetBufferSize(surfaceCapabilities);

	VkImageUsageFlags desiredUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	VkImageUsageFlags bufferUsage = desiredUsage & surfaceCapabilities.supportedUsageFlags;
	if (bufferUsage != desiredUsage)
	{
		std::cout << "Unable to setup swapchain for usage as " << desiredUsage << ". Aborting.\n";
		return false;
	}

	VkSurfaceFormatKHR surfaceFormat = GetSurfaceFormat(physicalDevice, m_surface);

	VkSwapchainCreateInfoKHR swapchainCreateInfo =
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		m_surface,
		numBuffers,
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		bufferSize,
		1, //TODO(KL): Support multi-view
		desiredUsage,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		surfaceCapabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		presentationMode,
		VK_TRUE,
		VK_NULL_HANDLE
	};

	result = vkCreateSwapchainKHR(m_vulkanDevice->GetDeviceInstance(), &swapchainCreateInfo, nullptr, &m_swapchain);
	if (result != VK_SUCCESS)
	{
		std::cout << "Could not create a swapchain. Aborting.\n";
		return false;
	}

	uint32_t numSwapchainImages = 0u;
	result = vkGetSwapchainImagesKHR(m_vulkanDevice->GetDeviceInstance(), m_swapchain, &numSwapchainImages, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "Could not get the number of swapchain images. Aborting.\n";
		return false;
	}

	m_images.resize(numSwapchainImages);
	result = vkGetSwapchainImagesKHR(m_vulkanDevice->GetDeviceInstance(), m_swapchain, &numSwapchainImages, m_images.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "Could not enumerate swapchain images. Aborting.\n";
		return false;
	}

	m_imageSemaphores.reserve(numSwapchainImages);
	for (uint32_t i = 0; i < numSwapchainImages; ++i)
	{
		m_imageSemaphores.emplace_back(new MxVulkanSemaphore(m_vulkanDevice));
	}

	return true;
}

void MxVulkanSwapchain::Present()
{
	//TODO(KL): Implement
}

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

	if (foundPresentModeImmediate) //TODO(KL): check for vsync?
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

VkExtent2D GetBufferSize(uint32_t DesiredWidth, uint32_t DesiredHeight, const VkSurfaceCapabilitiesKHR& SurfaceCapabilities)
{
	uint32_t UsedWidth = std::clamp(DesiredWidth, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width);
	uint32_t UsedHeight = std::clamp(DesiredHeight, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);

	return {UsedWidth, UsedHeight};
}

VkSurfaceFormatKHR GetSurfaceFormat(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface)
{
	constexpr VkSurfaceFormatKHR defaultSurfaceFormat = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	uint32_t numSurfaceFormats = 0u;
	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &numSurfaceFormats, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "Error getting surface formats. Using a default in the hopes that it works..\n";
		return defaultSurfaceFormat;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(numSurfaceFormats);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &numSurfaceFormats, surfaceFormats.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "Error getting surface formats. Using a default in the hopes that it works..\n";
		return defaultSurfaceFormat;
	}


}
}

MxVulkanSwapchain::MxVulkanSwapchain(VkInstance InVulkanInstance, MxVulkanDevice* InVulkanDevice)
	: VulkanInstance(InVulkanInstance), VulkanDevice(InVulkanDevice)
{
	assert(VulkanInstance);
	assert(VulkanDevice);
}

MxVulkanSwapchain::~MxVulkanSwapchain()
{
	//TODO(KL): Destroy Swapchains
	if (Surface)
	{
		vkDestroySurfaceKHR(VulkanInstance, Surface, nullptr);
		Surface = nullptr;
	}
}

bool MxVulkanSwapchain::Create(SDL_Window* WindowHandle)
{
	Surface = MxVulkanPlatform::CreateSurface(WindowHandle, VulkanInstance);
	if (!Surface)
	{
		return false;
	}

	const VkPhysicalDevice PhysicalDevice = VulkanDevice->GetPhysicalDevice();

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &surfaceCapabilities);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	uint32_t numBuffers = std::min(surfaceCapabilities.minImageCount + 1u, surfaceCapabilities.maxImageCount);

	VkPresentModeKHR presentationMode = GetPresentationMode(PhysicalDevice, Surface);
	VkExtent2D bufferSize = GetBufferSize(640, 480, surfaceCapabilities);

	VkImageUsageFlags desiredUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	VkImageUsageFlags bufferUsage = desiredUsage & surfaceCapabilities.supportedUsageFlags;
	if (bufferUsage != desiredUsage)
	{
		std::cout << "Unable to setup swapchain for usage as " << desiredUsage << ". Aborting.\n";
		return false;
	}

	//{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }

	//vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, )

	return true;
}

void MxVulkanSwapchain::Present()
{
}

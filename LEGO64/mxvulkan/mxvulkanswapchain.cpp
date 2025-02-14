#include "mxvulkanswapchain.h"

#include "mxvulkandevice.h"

MxVulkanSwapchain::MxVulkanSwapchain(VkInstance InVulkanInstance, MxVulkanDevice* InVulkanDevice)
	: VulkanInstance(InVulkanInstance), VulkanDevice(InVulkanDevice)
{
	assert(VulkanInstance);
	assert(VulkanDevice);
}

MxVulkanSwapchain::~MxVulkanSwapchain()
{
	//TODO(KL): Destroy Swapchains
	vkDestroySurfaceKHR(VulkanInstance, Surface, nullptr);
}

bool MxVulkanSwapchain::Create(SDL_Window* WindowHandle)
{
	Surface = MxVulkanPlatform::CreateSurface(WindowHandle, VulkanInstance);
	if (!Surface)
	{
		return false;
	}

	const VkPhysicalDevice PhysicalDevice = VulkanDevice->GetPhysicalDevice();

	//vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, )

	return false;
}

void MxVulkanSwapchain::Present()
{
}

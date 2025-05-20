
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
	if (!MxVulkanPlatform::LoadVulkanLibrary())
	{
		return false;
	}

	unsigned int numExtensions = 0;
	if (!SDL_Vulkan_GetInstanceExtensions(window, &numExtensions, nullptr))
	{
		std::cout << "Could not get the number of required instance extensions from SDL.\n";
		return false;
	}

	std::vector<const char*> extensions(numExtensions);
	if (!SDL_Vulkan_GetInstanceExtensions(window, &numExtensions, extensions.data()))
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

	VkResult result = vkCreateInstance(&instInfo, nullptr, &m_vulkanInstance);

	if (result != VK_SUCCESS)
	{
		std::cout << "Unable to create Vulkan Instance\n";
		return false;
	}

	MxVulkanPlatform::LoadVulkanInstanceFunctions(m_vulkanInstance);

	m_vulkanDevice = std::make_unique<MxVulkanDevice>(m_vulkanInstance);
	if (!m_vulkanDevice->Create())
	{
		m_vulkanDevice.reset();
		std::cout << "Unable to create Vulkan Device\n";
		return false;
	}

	m_viewport = std::make_unique<MxVulkanViewport>(m_vulkanInstance, m_vulkanDevice.get());
	if (!m_viewport->Create(window))
	{
		std::cout << "Unable to create viewport.\n";
		return false;
	}

	return true;
}

void MxVulkan::Shutdown()
{
	m_viewport.reset();
	m_vulkanDevice.reset();

	if (m_vulkanInstance && vkDestroyInstance)
	{
		vkDestroyInstance(m_vulkanInstance, nullptr);
		m_vulkanInstance = nullptr;
	}

	MxVulkanPlatform::ReleaseVulkanLibrary();
}

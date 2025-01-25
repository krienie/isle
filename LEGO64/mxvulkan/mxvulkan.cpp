
#include "mxvulkan.h"

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include <iostream>
#include <vector>

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
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,             // VkStructureType           sType
		nullptr,                                            // const void              * pNext
		0,                                                  // VkInstanceCreateFlags     flags
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

	return true;
}

void MxVulkan::Shutdown()
{
	if (Instance)
	{
		vkDestroyInstance(Instance, nullptr);
	}
}

#include "vulkanplatform.h"

#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#elif defined __linux
#include <dlfcn.h>
#endif

namespace
{
#ifdef _WIN32
	#define LIBRARY_TYPE HMODULE
	#define LoadFunction GetProcAddress
#elif defined __linux
	#define LIBRARY_TYPE void*
	#define LoadFunction dlsym
#endif

LIBRARY_TYPE VulkanLibrary = nullptr;
}

#define DEFINE_VK_ENTRYPOINTS(Type,Func) Type Func = nullptr;
ENUM_VK_ENTRYPOINTS_ALL(DEFINE_VK_ENTRYPOINTS)

bool VulkanPlatform::LoadVulkanLibrary()
{
	if (VulkanLibrary)
	{
		return true;
	}

#if defined _WIN32
	VulkanLibrary = LoadLibrary( "vulkan-1.dll" );
#elif defined __linux
	VulkanDLLModule = dlopen( "libvulkan.so.1", RTLD_NOW );
#endif

	if (VulkanLibrary)
	{
		bool bFoundAllEntryPoints = true;

#define GET_VK_ENTRYPOINTS(Type,Func) Func = (Type)LoadFunction(VulkanLibrary, #Func); \
		if (!Func)                                                                     \
		{                                                                              \
			std::cout << "Unable to load Vulkan function " << #Func << '\n';           \
			bFoundAllEntryPoints = false;                                              \
		}

		ENUM_VK_ENTRYPOINTS_BASE(GET_VK_ENTRYPOINTS)
		ENUM_VK_ENTRYPOINTS_OPTIONAL_BASE(GET_VK_ENTRYPOINTS)
#undef GET_VK_ENTRYPOINTS

		if (!bFoundAllEntryPoints)
		{
			ReleaseVulkanLibrary();
			return false;
		}

		return true;
	}

	return false;
}

void VulkanPlatform::ReleaseVulkanLibrary()
{
	if (VulkanLibrary)
	{
#if defined _WIN32
		FreeLibrary(VulkanLibrary);
#elif defined __linux
		dlclose(VulkanLibrary);
#endif
		VulkanLibrary = nullptr;
	}
}

bool VulkanPlatform::LoadVulkanInstanceFunctions(VkInstance instance)
{
	if (!VulkanLibrary)
	{
		return false;
	}

	bool bFoundAllEntryPoints = true;

#define GETINSTANCE_VK_ENTRYPOINTS(Type, Func) Func = (Type)vkGetInstanceProcAddr(instance, #Func); \
		if (!Func)                                                                                  \
		{                                                                                           \
			std::cout << "Unable to load Vulkan instance function " << #Func << '\n';               \
			bFoundAllEntryPoints = false;                                                           \
		}

	ENUM_VK_ENTRYPOINTS_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS)
	ENUM_VK_ENTRYPOINTS_SURFACE_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS)
	ENUM_VK_ENTRYPOINTS_OPTIONAL_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS)
	ENUM_VK_ENTRYPOINTS_PLATFORM_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS)
#undef GETINSTANCE_VK_ENTRYPOINTS

	if (!bFoundAllEntryPoints)
	{
		ReleaseVulkanLibrary();
		return false;
	}

	return true;
}

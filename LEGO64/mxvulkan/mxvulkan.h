#pragma once

#include <SDL2/SDL_video.h>

#include "vulkanplatform.h"

class MxVulkan
{
public:
	MxVulkan() = default;
	~MxVulkan();

	bool InitForWindow(SDL_Window* window);
	void Shutdown();

private:
	bool CreateDevice();

	VkInstance Instance = nullptr;
	VkDevice Device = nullptr;
};

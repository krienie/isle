
#pragma once

#include "mxvulkan/mxvulkanviewport.h"

#include <SDL_video.h>
#include <memory>

class RenderWindow final
{
public:
	RenderWindow(SDL_Window* window);
	~RenderWindow();

	void draw() const;

private:
	std::unique_ptr<MxVulkanViewport> m_viewport;
};

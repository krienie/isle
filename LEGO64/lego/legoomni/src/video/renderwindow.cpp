
#include "video/renderwindow.h"

#include "video/renderthread.h"

#include <iostream>

RenderWindow::RenderWindow(SDL_Window* window)
{
	m_viewport = std::make_unique<MxVulkanViewport>(RenderThread::Get()->GetVulkanInstance(), RenderThread::Get()->GetVulkanDevice());
	if (!m_viewport->Create(window))
	{
		std::cout << "Unable to create viewport.\n";
	}
}

RenderWindow::~RenderWindow()
{
	m_viewport.reset();
}

void RenderWindow::draw() const
{
	RenderThread::Get()->EnqueueCommand([this]()
	{
		//TODO(KL): Implement

		//m_viewport->Present();
	});
}

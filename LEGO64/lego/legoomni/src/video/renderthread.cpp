
#include "video/renderthread.h"

#include "commandthread.h"
#include "mxvulkan/mxvulkancommandpool.h"

#include <cassert>
#include <utility>

RenderThread* RenderThread::m_instance = nullptr;

RenderThread* RenderThread::Get()
{
	return m_instance;
}

void RenderThread::Startup(SDL_Window* windowHandle)
{
	if (!m_instance)
	{
		m_instance = new RenderThread(windowHandle);
	}
}

void RenderThread::Shutdown()
{
	delete m_instance;
	m_instance = nullptr;
}

RenderThread::RenderThread(SDL_Window* windowHandle)
	: m_commandThread(std::make_unique<CommandThread>(1))
{
	m_vulkanRHI = std::make_unique<MxVulkanRHI>();

	bool rhiInitSuccess = m_vulkanRHI->InitForWindow(windowHandle);
	assert(rhiInitSuccess && "Error creating RHI!");

	m_commandPool = std::make_unique<MxVulkanCommandPool>(m_vulkanRHI->GetVulkanDevice());
	m_activeCommandBuffer = m_commandPool->CreateNewCommandBuffer();
}

RenderThread::~RenderThread()
{
	m_commandThread.reset();
	m_commandPool.reset();
	m_vulkanRHI.reset();
}

void RenderThread::EnqueueCommand(RenderCommand cmd) const
{
	//TODO(KL): encapsulate a lambda here to be able to include things like the commandlist
	
	m_commandThread->EnqueueCommand(std::move(cmd));
}

void RenderThread::Flush() const
{
	m_commandThread->Flush();
}

bool RenderThread::IsRunning() const
{
	return m_commandThread->IsRunning();
}

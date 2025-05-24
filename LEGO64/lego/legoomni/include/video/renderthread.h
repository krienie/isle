
#pragma once

#include "mxvulkan/mxvulkancommandbuffer.h"

#include <functional>
#include <memory>

#include "mxvulkan/mxvulkanrhi.h"

class CommandThread;

class RenderThread final
{
public:
	static RenderThread* Get();
	static void Startup(SDL_Window* windowHandle);
	static void Shutdown();

	using RenderCommand = std::function<void()>;

	RenderThread(const RenderThread&) noexcept            = delete;
	RenderThread(RenderThread&&) noexcept                 = delete;
	RenderThread& operator=(const RenderThread&) noexcept = delete;
	RenderThread& operator=(RenderThread&&) noexcept      = delete;

	void EnqueueCommand(RenderCommand cmd) const;
	void Flush() const;

	bool IsRunning() const;
	MxVulkanDevice* GetVulkanDevice() const { return m_vulkanRHI->GetVulkanDevice(); }
	VkInstance GetVulkanInstance() const { return m_vulkanRHI->GetVulkanInstance(); }

private:
	RenderThread(SDL_Window* windowHandle);
	~RenderThread();

	static RenderThread* m_instance;

	std::unique_ptr<CommandThread> m_commandThread;
	std::unique_ptr<MxVulkanRHI> m_vulkanRHI;

	std::unique_ptr<MxVulkanCommandPool> m_commandPool;
	MxVulkanCommandBuffer* m_activeCommandBuffer = nullptr;
};

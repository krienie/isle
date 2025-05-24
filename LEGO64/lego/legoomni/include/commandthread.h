
#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include <vector>
#include <functional>

class CommandThread final
{
public:
	using ThreadCommand = std::function<void()>;

	CommandThread(unsigned int numWorkerThreads = std::thread::hardware_concurrency());
	~CommandThread();

	CommandThread(const CommandThread&) noexcept            = delete;
	CommandThread(CommandThread&&) noexcept                 = delete;
	CommandThread& operator=(const CommandThread&) noexcept = delete;
	CommandThread& operator=(CommandThread&&) noexcept      = delete;

	void EnqueueCommand(ThreadCommand cmd);
	void Flush();

	bool IsRunning() const { return m_running; }

private:
	void processThreadCommands();

	std::atomic_bool m_running;
	std::vector<std::thread> m_workerThreads;
	std::mutex m_enqueueMutex;

	std::deque<ThreadCommand> m_commands;
	std::condition_variable m_cvCommand;
	std::condition_variable m_cvFinished;
	unsigned int m_numBusyThreads;
};

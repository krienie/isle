
#include "CommandThread.h"

CommandThread::CommandThread(unsigned int numWorkerThreads)
	: m_running(true), m_numBusyThreads(0u)
{
	m_workerThreads.reserve(numWorkerThreads);
	for (unsigned int i = 0u; i < numWorkerThreads; ++i)
	{
		m_workerThreads.emplace_back(&CommandThread::processThreadCommands, this);
	}
}

CommandThread::~CommandThread()
{
	{
		std::lock_guard lock(m_enqueueMutex);
		m_running = false;
		m_cvCommand.notify_all();
	}

	for (auto &thread : m_workerThreads)
	{
		thread.join();
	}
}

void CommandThread::EnqueueCommand(ThreadCommand cmd)
{
	std::lock_guard lock(m_enqueueMutex);
	m_commands.emplace_back(std::move(cmd));
	m_cvCommand.notify_one();
}

void CommandThread::Flush()
{
	std::unique_lock lock(m_enqueueMutex);
	m_cvFinished.wait(lock, [this](){ return m_commands.empty() && m_numBusyThreads == 0u; });
}

void CommandThread::processThreadCommands()
{
	do
	{
		// wait for commands to show up
		std::unique_lock lock(m_enqueueMutex);
		m_cvCommand.wait(lock, [this]() { return !m_running || !m_commands.empty(); });
		if (m_running && !m_commands.empty())
		{
			++m_numBusyThreads;

			ThreadCommand cmd = m_commands.front();
			m_commands.pop_front();

			// Run the command
			lock.unlock();
			cmd();
			lock.lock();

			--m_numBusyThreads;
			m_cvFinished.notify_one();
		}
	} while (m_running);
}

/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#include "CommandQueue.hpp"

void CommandQueue::Push(const Command& command)
{
	m_queue.push(command);
}

Command CommandQueue::Pop()
{
	Command command = m_queue.front();
	m_queue.pop();
	return command;
}

bool CommandQueue::IsEmpty() const
{
	return m_queue.empty();
}

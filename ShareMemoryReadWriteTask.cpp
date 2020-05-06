#include "ShareMemoryReadWriteTask.h"
#include "WindowsShareMemoryWrapper.h"
#include "ShareMemoryReadWriteHandle.h"

ShareMemoryReadWriteTask::ShareMemoryReadWriteTask(SHARE_MEM_TASK_TYPE type,WindowsSharedMemoryWrapper* mem,int taskId):
	m_taskType(type), m_shareMem(mem), m_needClear(false),m_taskDoResult(true),m_taskId(taskId)
{

}

ShareMemoryReadWriteTask::~ShareMemoryReadWriteTask()
{

}

void ShareMemoryReadWriteTask::SetWriteContent(std::string msg)
{
	m_writeMsg = msg;
}

void ShareMemoryReadWriteTask::TaskBegin()
{
	if (m_taskType == eWRITE_MSG)
	{
		m_taskDoResult = DoWriteTask(m_writeMsg);
	}
	else if (m_taskType == eREAD_MSG)
	{
		m_taskDoResult = !DoReadTask().empty();
	}
	if (m_taskDoResult)
	{
		g_ShareMemoryReadWriteHandle->PostComplateTask(m_taskId);
	}
	m_needClear = true;
}

std::string ShareMemoryReadWriteTask::GetReceiveMsg()
{
	if (!m_shareMem)
		return "";
	return m_shareMem->ReadMemory();
}

bool ShareMemoryReadWriteTask::DoWriteTask(std::string msg)
{
	bool ret = false;
	if(!m_shareMem /*|| msg.empty()*/)
		return ret;
	if (m_shareMem->MapAt())
	{
		ret = m_shareMem->WriteMemory(msg);
	}
	return ret;
}

std::string ShareMemoryReadWriteTask::DoReadTask()
{
	if (!m_shareMem)
		return "";
	std::string mem_str = "";
	//m_shareMem->Lock();
	bool ret = m_shareMem->Open();
	if (ret)
	{
		if (m_shareMem->MapAt())
		{
			mem_str = m_shareMem->ReadMemory();
		}
	}
	//m_shareMem->Unlock();
	return mem_str;
}

#include "ShareMemoryReadWriteHandle.h"
#include "WindowsShareMemoryWrapper.h"
#include "ShareMemoryReadWriteTask.h"

#include <iostream>

ShareMemoryReadWriteHandle* g_ShareMemoryReadWriteHandle = NULL;

const std::string WRITE_MAPPING_FILENAME = "BrowerToGame_ShareMem";
const std::string READ_MAPPING_FILENAME = "GameToBrower_ShareMem";
const int READ_WRITE_OFFSET = 0;
const int READ_WRITE_FILESIZE = 1024;

ShareMemoryReadWriteHandle::ShareMemoryReadWriteHandle()
{
	m_idStart = 10;
	m_writeShareMem = new WindowsSharedMemoryWrapper(READ_WRITE_OFFSET, READ_WRITE_FILESIZE,WRITE_MAPPING_FILENAME);
	m_readShareMem = new WindowsSharedMemoryWrapper(READ_WRITE_OFFSET, READ_WRITE_FILESIZE, READ_MAPPING_FILENAME);
	m_thread_name = "Read_Write_ShareMem";
	start();
}

ShareMemoryReadWriteHandle::~ShareMemoryReadWriteHandle()
{
	if (m_writeShareMem)
	{
		m_writeShareMem->Close();
		delete m_writeShareMem;
		m_writeShareMem = NULL;
	}

	if (m_readShareMem)
	{
		m_readShareMem->Close();
		delete m_readShareMem;
		m_readShareMem = NULL;
	}
	shutdown();
}

void ShareMemoryReadWriteHandle::CreateWriteShareMem()
{
	if(m_writeShareMem)
		m_writeShareMem->Create();
}

void ShareMemoryReadWriteHandle::OpenReadShareMem()
{
	if (m_readShareMem)
		m_readShareMem->Open();
}

void ShareMemoryReadWriteHandle::PostComplateTask(int taskId)
{
	Ogre::LockFunctor lockfunc(&m_Mutex);
	m_completeTaskIds.push(taskId);
}

int ShareMemoryReadWriteHandle::DequeueComplateTask(int& taskId)
{
	int rc = -1;
	Ogre::LockFunctor lockfunc(&m_Mutex);
	if (!m_completeTaskIds.empty())
	{
		taskId = m_completeTaskIds.front();
		m_completeTaskIds.pop();
		rc = 0;
	}
	return rc;
}

#include "atlbase.h"
#include "atlstr.h"
void OutputDebugPrintf(const char* strOutputString, ...)
{
	char strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
	//vsprintf(strBuffer, strOutputString, vlArgs);
	va_end(vlArgs);
	OutputDebugString(CA2W(strBuffer));
}

void ShareMemoryReadWriteHandle::Update()
{
	int taskId = 0;
	while(DequeueComplateTask(taskId) == 0)
	{
		if (m_taskMap.find(taskId) != m_taskMap.end())
		{
			ShareMemoryReadWriteTask* d_ = m_taskMap[taskId];
			if (d_)
			{
				if (d_->GetTaskType() == eWRITE_MSG)
				{
					bool result = d_->GetTaskDoResult();
				}
				else
				{
					std::string msg("receive msg:");
					msg.append(d_->GetReceiveMsg());
					OutputDebugPrintf(msg.c_str());
					OutputDebugPrintf("\n");
					ResetReadMappingFile();
				}
			}
		}
	}

	Ogre::LockFunctor lockfunc(&m_Mutex);
	clearOldTask();
}

int ShareMemoryReadWriteHandle::SendMsg(const std::string& msg)
{
	if (m_Shutdown || msg.empty()) return 0;
	int task_id_ = 0;
	ShareMemoryReadWriteTask* task = NULL;
	{
		Ogre::LockFunctor lockfunc(&m_Mutex);
		//clearOldTask();
		task_id_ = (++m_idStart);
		task = new ShareMemoryReadWriteTask(eWRITE_MSG,m_writeShareMem,task_id_);
		task->SetWriteContent(msg);
		m_taskMap[task_id_] = task;
		m_taskList.push(task);
	}
	start_next();
	return task_id_;
}

int ShareMemoryReadWriteHandle::ResetReadMappingFile()
{
	if (m_Shutdown) return 0;
	int task_id_ = 0;
	ShareMemoryReadWriteTask* task = NULL;
	{
		Ogre::LockFunctor lockfunc(&m_Mutex);
		clearOldTask();
		task_id_ = (++m_idStart);
		task = new ShareMemoryReadWriteTask(eWRITE_MSG, m_readShareMem, task_id_);
		task->SetWriteContent("");
		m_taskMap[task_id_] = task;
		m_taskList.push(task);
	}
	start_next();
	return task_id_;
}

int ShareMemoryReadWriteHandle::ReceiveMsg()
{
	if (m_Shutdown) return 0;
	int task_id_ = 0;
	ShareMemoryReadWriteTask* task = NULL;
	{
		Ogre::LockFunctor lockfunc(&m_Mutex);
		//clearOldTask();
		task_id_ = (++m_idStart);
		task = new ShareMemoryReadWriteTask(eREAD_MSG, m_readShareMem, task_id_);
		m_taskMap[task_id_] = task;
		m_taskList.push(task);
	}
	start_next();
	return task_id_;
}

Ogre::OSThread::RUN_RETTYPE ShareMemoryReadWriteHandle::_run()
{
	if (m_Shutdown) return Ogre::OSThread::RUN_EXIT;
	ShareMemoryReadWriteTask* task_ = NULL;
	{
		Ogre::LockFunctor lockfunc(&m_Mutex);
		if (m_taskList.empty())
		{
			return Ogre::OSThread::RUN_WAIT; 	//任务全部完成
		}
		else
		{
			task_ = m_taskList.front();
			m_taskList.pop();
		}
	}
	if (task_)
	{
		if (!task_->IsNeedClear()) task_->TaskBegin();
	}
	return Ogre::OSThread::RUN_CONTINUE;
}

void ShareMemoryReadWriteHandle::clearOldTask()
{
	if (m_Shutdown) return;
	std::vector<int> remove_list;
	for (std::map<int, ShareMemoryReadWriteTask*>::iterator it = m_taskMap.begin(); it != m_taskMap.end(); it++)
	{
		ShareMemoryReadWriteTask* task_ = it->second;
		if (task_->IsNeedClear())
		{
			remove_list.push_back(it->first);
		}
	}
	for (size_t i = 0; i < remove_list.size(); i++) {
		ShareMemoryReadWriteTask* d_ = m_taskMap[remove_list[i]];
		if (d_) {
			delete d_;
			d_ = NULL;
		}
		m_taskMap.erase(remove_list[i]);
	}
}

void ShareMemoryReadWriteHandle::start_next()
{
	if (m_Shutdown) return;
		active();
}

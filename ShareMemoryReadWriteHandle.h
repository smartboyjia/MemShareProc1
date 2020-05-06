#pragma once
#include <string>
#include <map>
#include <set>
#include <queue>
#include "OgreThread.h"
#include "ShareMemoryReadWriteTask.h"

class WindowsSharedMemoryWrapper;

class ShareMemoryReadWriteHandle : public Ogre::OSThread 
{
public:
	ShareMemoryReadWriteHandle();
	~ShareMemoryReadWriteHandle();
	void CreateWriteShareMem();
	void OpenReadShareMem();
	void PostComplateTask(int taskId);
	int  DequeueComplateTask(int&taskId);
	void Update();
	int SendMsg(const std::string& msg);
	int ResetReadMappingFile();
	int ReceiveMsg();
private:
	RUN_RETTYPE  _run();
	void clearOldTask();     //清理已经完成的任务
	void start_next();       //开始下一个任务
private:
	Ogre::LockSection							m_Mutex;             //锁
	std::string									m_thread_name;		 //线程名字
	int											m_idStart;
	std::map<int, ShareMemoryReadWriteTask*>	m_taskMap;			 //任务列表
	std::queue<ShareMemoryReadWriteTask*>		m_taskList;			 //人物列表，先进先出
	std::queue<int>								m_completeTaskIds;	 //完成的任务ID
	WindowsSharedMemoryWrapper*					m_writeShareMem;	 //写共享内存
	WindowsSharedMemoryWrapper*					m_readShareMem;		 //读共享内存
};

extern ShareMemoryReadWriteHandle* g_ShareMemoryReadWriteHandle;

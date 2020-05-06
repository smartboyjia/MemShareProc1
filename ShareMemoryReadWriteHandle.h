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
	void clearOldTask();     //�����Ѿ���ɵ�����
	void start_next();       //��ʼ��һ������
private:
	Ogre::LockSection							m_Mutex;             //��
	std::string									m_thread_name;		 //�߳�����
	int											m_idStart;
	std::map<int, ShareMemoryReadWriteTask*>	m_taskMap;			 //�����б�
	std::queue<ShareMemoryReadWriteTask*>		m_taskList;			 //�����б��Ƚ��ȳ�
	std::queue<int>								m_completeTaskIds;	 //��ɵ�����ID
	WindowsSharedMemoryWrapper*					m_writeShareMem;	 //д�����ڴ�
	WindowsSharedMemoryWrapper*					m_readShareMem;		 //�������ڴ�
};

extern ShareMemoryReadWriteHandle* g_ShareMemoryReadWriteHandle;

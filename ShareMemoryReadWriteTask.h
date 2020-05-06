#pragma once
#include <string>
typedef enum 
{
	eWRITE_MSG = 0, //写数据到共享内存
	eREAD_MSG,		//读数据从共享内存

}SHARE_MEM_TASK_TYPE;

class WindowsSharedMemoryWrapper;
class ShareMemoryReadWriteTask 
{
public:
	ShareMemoryReadWriteTask(SHARE_MEM_TASK_TYPE type,WindowsSharedMemoryWrapper* mem,int taskId);
	~ShareMemoryReadWriteTask();
	void SetWriteContent(std::string msg);
	void TaskBegin();
	bool IsNeedClear() { return m_needClear;}
	SHARE_MEM_TASK_TYPE GetTaskType() { return m_taskType; }
	bool GetTaskDoResult() { return m_taskDoResult; }
	std::string GetReceiveMsg();
private:
	bool DoWriteTask(std::string msg);
	std::string DoReadTask();
private:
	SHARE_MEM_TASK_TYPE m_taskType;
	int							m_taskId;
	WindowsSharedMemoryWrapper* m_shareMem;
	std::string					m_writeMsg;
	bool            m_needClear;    //是否可以被清理 已经读取后设置
	bool			m_taskDoResult;
};

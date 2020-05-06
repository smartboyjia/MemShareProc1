#include "WindowsShareMemoryWrapper.h"

WindowsSharedMemoryWrapper::WindowsSharedMemoryWrapper(DWORD dwOffset, DWORD dwSize,const std::string& strName):m_hFileMap(NULL),
	m_pMemory(NULL),
	m_dwOffset(dwOffset),
	m_dwMappedSize(dwSize),
	m_strName(strName)
{
	
}

WindowsSharedMemoryWrapper::~WindowsSharedMemoryWrapper()
{
	Close();
}

bool WindowsSharedMemoryWrapper::Create()
{
	if (m_dwMappedSize <= 0 || m_strName.empty())
		return false;

	HANDLE handle = ::CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m_dwMappedSize, m_strName.c_str());
	if (!handle)
		return false;
	//// 已经存在了
	//if (GetLastError() == ERROR_ALREADY_EXISTS)
	//{
	//	Close();
	//	return false;
	//}
	m_hFileMap = handle;
	return false;
}

bool WindowsSharedMemoryWrapper::Open()
{
	if (m_strName.empty())
		return false;
	m_hFileMap = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, m_strName.c_str());
	if (!m_hFileMap)
		return false;
	return true;
}

bool WindowsSharedMemoryWrapper::MapAt()
{
	if (!m_hFileMap || m_dwMappedSize > ULONG_MAX)
		return false;

	ULARGE_INTEGER ui;
	ui.QuadPart = static_cast<ULONGLONG>(m_dwOffset);
	m_pMemory = (LPTSTR)::MapViewOfFile(m_hFileMap,FILE_MAP_ALL_ACCESS, ui.HighPart, ui.LowPart, m_dwMappedSize);
	return (m_pMemory != NULL);
}

void WindowsSharedMemoryWrapper::Unmap()
{
	if (m_pMemory)
	{
		::UnmapViewOfFile(m_pMemory);
		m_pMemory = NULL;
	}
}

bool WindowsSharedMemoryWrapper::WriteMemory(std::string data)
{
	bool ret = false;
	if (data.empty())
	{
		Lock();
		memset((void*)m_pMemory, 0, 1024);
		ret = CopyMemory((void*)m_pMemory, data.c_str(), 1024) != NULL;
		Unlock();
	}
	else
	{
		Lock();
		memset((void*)m_pMemory, 0, 1024);
		ret = CopyMemory((void*)m_pMemory, data.c_str(), data.length()) != NULL;
		Unlock();
	}
	return ret;
}

std::string WindowsSharedMemoryWrapper::ReadMemory()
{
	std::string msg = "";
	if (m_pMemory != NULL)
	{
		Lock();
		msg.assign((char*)m_pMemory);
		Unlock();
	}
	return msg;
}

HANDLE WindowsSharedMemoryWrapper::GetHandle() const
{
	return m_hFileMap;
}

bool WindowsSharedMemoryWrapper::Lock()
{
	// 如果还没有创建锁就先创建一个
	if (!m_hLock)
	{
		std::string strLockName = m_strName;
		strLockName.append("_Lock");
		// 初始化的时候不被任何线程占用
		m_hLock = ::CreateMutexA(NULL, FALSE, strLockName.c_str());
		if (!m_hLock)
			return false;
	}
	// 哪个线程最先调用等待函数就最先占用这个互斥量
	DWORD dwRet = ::WaitForSingleObject(m_hLock, INFINITE);
	return (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED);
}

void WindowsSharedMemoryWrapper::Unlock()
{
	if (m_hLock)
	{
		::ReleaseMutex(m_hLock);
	}
}

void WindowsSharedMemoryWrapper::Close()
{
	Unmap();
	if (m_hFileMap)
	{
		::CloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}
	if (m_hLock != NULL)
	{
		CloseHandle(m_hLock);
		m_hLock = NULL;
	}
}

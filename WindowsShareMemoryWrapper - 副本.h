#pragma once
#include <windows.h>
#include <string>

class WindowsSharedMemoryWrapper
{
public:
    WindowsSharedMemoryWrapper(DWORD dwOffset, DWORD dwSize,const std::string& strName);
    ~WindowsSharedMemoryWrapper();
    bool Create();
    bool Open();
    bool MapAt();
    void Unmap();
    bool WriteMemory(std::string data);
    std::string ReadMemory() const;
    HANDLE GetHandle() const;
    bool Lock(DWORD dwTime);
    void Unlock();
    void Close();
private:
    WindowsSharedMemoryWrapper(const WindowsSharedMemoryWrapper& other);
    WindowsSharedMemoryWrapper& operator = (const WindowsSharedMemoryWrapper& other);
private:
    HANDLE m_hLock;
    HANDLE m_hFileMap;
    LPCTSTR m_pMemory;
    std::string m_strName;
    DWORD m_dwMappedSize;
    DWORD m_dwOffset;
};
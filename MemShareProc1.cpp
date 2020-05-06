// MemShareProc1.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include <conio.h>
#include <time.h>
#include "MemShareProc1.h"
#include "ShareMemoryReadWriteHandle.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
bool SendMsgToShareMemory();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MEMSHAREPROC1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    ShareMemoryReadWriteHandle shareMemHandle;
    g_ShareMemoryReadWriteHandle = &shareMemHandle;
    //SendMsgToShareMemory();
    g_ShareMemoryReadWriteHandle->CreateWriteShareMem();
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MEMSHAREPROC1));
    MSG msg;
    // 主消息循环:
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            char s_time[256];
            sprintf_s(s_time, "%ld", clock());
            std::string msg("Send[");
            msg.append(s_time).append("]: ");
            msg.append("Hello Browers!!!");
            g_ShareMemoryReadWriteHandle->Update();
            g_ShareMemoryReadWriteHandle->ReceiveMsg();
            g_ShareMemoryReadWriteHandle->SendMsg(msg);
        }
        else
        {
            char s_time[256];
            sprintf_s(s_time, "%ld", clock());
            std::string msg("Send[");
            msg.append(s_time).append("]: ");
            msg.append("Hello Browers!!!");
            g_ShareMemoryReadWriteHandle->Update();
            g_ShareMemoryReadWriteHandle->ReceiveMsg();
            g_ShareMemoryReadWriteHandle->SendMsg(msg);

            Sleep(100);
        }
    }
   
    return (int) msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MEMSHAREPROC1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MEMSHAREPROC1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

#define BUFFER_SIZE    256
TCHAR szMapFileName[] = TEXT("MyFileMappingName");  //映射文件名，即共享内存的名称
TCHAR szSendData[] = TEXT("Message from the send process.");

bool SendMsgToShareMemory()
{
    //发送数据的进程先启动，用于发送数据，即将数据写入视图 。
    HANDLE  hMapFile = NULL;
    LPCTSTR pBuf = NULL;

    //1. 创建一个文件映射内核对象
    hMapFile = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUFFER_SIZE, szMapFileName);  //INVALID_HANDLE_VALUE表示创建一个进程间共享的对象
    if (NULL == hMapFile)
    {
        _tprintf(TEXT("Could not create file mapping object (%d).\n"), GetLastError());
        return -1;
    }

    //2. 将文件数据映射到进程的地址空间
    pBuf = (LPTSTR)::MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (NULL == pBuf)
    {
        _tprintf(TEXT("Could not map view of file (%d). \n"), GetLastError());

        ::CloseHandle(hMapFile);
        hMapFile = NULL;

        return -1;
    }

    //3. 写入到内存中
    CopyMemory((void*)pBuf, szSendData, _tcslen(szSendData) * sizeof(TCHAR));
    //_getch();  //这个函数是一个不回显函数，当用户按下某个字符时，函数自动读取，无需按回车

    //4. 从进程的地址空间中撤消文件数据的映像
    //::UnmapViewOfFile(pBuf);

    //5. 关闭文件映射对象和文件对象
    //::CloseHandle(hMapFile);
    return true;
}

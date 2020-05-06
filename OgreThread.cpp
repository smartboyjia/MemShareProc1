
#include "OgreThread.h"
#include <time.h>
#ifdef _USE_PTHREAD_
#else
#include <Windows.h>
#include <process.h>
#endif 


namespace Ogre
{
#ifdef _USE_PTHREAD_
	static void* ThreadMainFunc(void *pdata)
#else
	static unsigned int WINAPI ThreadMainFunc(void *pdata)
#endif
	{
		OSThread *pthread = (OSThread *)pdata;
		if(pthread == NULL) return 0;
		pthread->_onThreadStart();
		srand((unsigned)time(NULL)); //!!!

		for(;;)
		{
			pthread->m_ThreadState = OSThread::RUN_CONTINUE;
			int ret = pthread->_run();
			pthread->m_ThreadState = (OSThread::RUN_RETTYPE)ret;

			if(ret == OSThread::RUN_EXIT)
			{
				pthread->m_Shutdown = 0;
				break;
			}
			else if(ret == OSThread::RUN_WAIT)
			{
				if(1 == pthread->m_Shutdown)
				{
					pthread->m_Shutdown = 0;
					break;
				}
				
				pthread->m_WaitEvent.wait(pthread->getDefaultWaitTime());
			}

			/*
			if(1 == pthread->m_Shutdown)
			{
				pthread->m_Shutdown = 2;
			}
			else if (2 == pthread->m_Shutdown)
			{
				pthread->m_Shutdown = 0;
				return 0;
			}*/
		}

		pthread->_onThreadEnd();
		return 0;
	}

#ifndef _USE_PTHREAD_
	OSThread::OSThread() : m_hThread(0), m_Shutdown(0)
	{
	}

	OSThread::~OSThread()
	{
		shutdown();
	}

	void OSThread::start()
	{
		unsigned int threadid;
		m_Shutdown = 0;
		m_ThreadState = RUN_CONTINUE;
		m_hThread = (void *)_beginthreadex(NULL, 0, ThreadMainFunc, this, 0, &threadid);
	}

	void OSThread::shutdown()
	{
		if(m_hThread)
		{
			m_Shutdown = 1;
			while(m_Shutdown>0 && m_ThreadState!=RUN_EXIT)
			{
				::Sleep(0);
				m_WaitEvent.trigger();
			}

			CloseHandle((HANDLE)m_hThread);
			m_hThread = 0;
		}
	}
#else
	OSThread::OSThread() : m_hThread(0), m_Shutdown(0)
	{
	}

	OSThread::~OSThread()
	{
		shutdown();
	}

	void OSThread::start()
	{
		//销毁上一次的线程资源
		if(m_hThread)
		{
			void* ret;
			pthread_join(m_hThread, &ret);
			m_hThread = 0;
		}

		m_Shutdown = 0;
		m_ThreadState = RUN_CONTINUE;

		pthread_attr_t attributes;
		pthread_attr_init(&attributes);
        
        pthread_attr_setstacksize(&attributes, 1048576*2);

		if(pthread_create(&m_hThread, &attributes, ThreadMainFunc, this))
		{
			
			m_ThreadState = RUN_EXIT;
			printf("pthread_create failed\n");
			assert(0);
		}
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
		//pthread_setname_np(pthread_self(), "ogre_thread");
#endif
	}

	void OSThread::shutdown()
	{
		if(m_hThread)
		{
			m_Shutdown = 1;
			m_WaitEvent.trigger();

			void* ret;
			pthread_join(m_hThread, &ret);

			m_hThread = 0;
		}
	}
#endif
}
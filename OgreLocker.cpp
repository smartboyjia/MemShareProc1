
#include "OgreLocker.h"

#ifndef _USE_PTHREAD_
#ifdef _WIN32
#include <windows.h>
#endif
#else
#include <errno.h>
#endif

namespace Ogre
{
	LockSection::LockSection()
	{
#ifdef _USE_PTHREAD_
		pthread_mutex_init(&m_pSysObj, NULL);
#else
		LPCRITICAL_SECTION psection = new CRITICAL_SECTION;
		InitializeCriticalSection( psection );

		m_pSysObj = psection;
#endif
	}

	LockSection::~LockSection()
	{
#ifdef _USE_PTHREAD_
		pthread_mutex_destroy(&m_pSysObj);
#else
		DeleteCriticalSection( (LPCRITICAL_SECTION)m_pSysObj );
		delete((LPCRITICAL_SECTION)m_pSysObj);
#endif
	}

	void LockSection::Lock()
	{
#ifdef _USE_PTHREAD_
		pthread_mutex_lock(&m_pSysObj);
#else
		EnterCriticalSection( (LPCRITICAL_SECTION)m_pSysObj );
#endif
	}

	void LockSection::Unlock()
	{
#ifdef _USE_PTHREAD_
		pthread_mutex_unlock(&m_pSysObj);
#else
		LeaveCriticalSection( (LPCRITICAL_SECTION)m_pSysObj );
#endif
	}

	OSEvent::OSEvent(bool init_trigger, bool manual_reset)
	{
#ifdef _USE_PTHREAD_
		m_ManualReset = manual_reset;
		m_State = init_trigger;
		pthread_mutex_init(&m_Mutex, NULL);
		pthread_cond_init(&m_Cond, NULL);
#else
		m_Handle = CreateEvent(NULL, manual_reset?TRUE:FALSE, init_trigger?TRUE:FALSE, NULL);
#endif
	}

	OSEvent::~OSEvent()
	{
#ifdef _USE_PTHREAD_
		pthread_cond_destroy(&m_Cond);
		pthread_mutex_destroy(&m_Mutex);
#else
		CloseHandle((HANDLE)m_Handle);
#endif
	}

	int OSEvent::wait(unsigned int tick)
	{
#ifdef _USE_PTHREAD_
		if (tick == INFINITY)
		{
			if(pthread_mutex_lock(&m_Mutex)) return -1;

			while(!m_State)
			{
				if(!pthread_cond_wait(&m_Cond, &m_Mutex))
				{
					pthread_mutex_unlock(&m_Mutex);
					return -1;
				}
			}
			if(!m_ManualReset)
			{
				m_State = false;
			}
			if(pthread_mutex_unlock(&m_Mutex)) return -1;
		}
		else
		{
			struct timespec abstime;

			struct timeval tv;
			gettimeofday(&tv, NULL);
			abstime.tv_sec = tv.tv_sec + tick / 1000;
			abstime.tv_nsec = tv.tv_usec * 1000 + (tick % 1000) * 1000000;
			if (abstime.tv_nsec >= 1000000000)
			{
				abstime.tv_nsec -= 1000000000;
				abstime.tv_sec++;
			}

			if(pthread_mutex_lock(&m_Mutex)) return -1;
			int rc = 0;
			while(!m_State)
			{
				rc = pthread_cond_timedwait(&m_Cond, &m_Mutex, &abstime);
				if(rc)
				{
					if(rc == ETIMEDOUT) break;
					pthread_mutex_unlock(&m_Mutex);
					return -1;
				}
			}

			if(rc == 0 && !m_ManualReset)
			{
				m_State =  false;
			}
			if(pthread_mutex_unlock(&m_Mutex)) return -1;
			if(rc == ETIMEDOUT) return 1;
		}
#else
		DWORD ret = WaitForSingleObject((HANDLE)m_Handle, tick);
		if(ret == WAIT_OBJECT_0)
		{
			return 0;
		}
		if(ret == WAIT_TIMEOUT)
		{
			return 1;
		}
		return -1;
#endif
        return -1;
	}

	int OSEvent::trigger()
	{
#ifdef _USE_PTHREAD_
		if(pthread_mutex_lock(&m_Mutex)) return -1;

		m_State = true;
		if(m_ManualReset)
		{
			if(pthread_cond_broadcast(&m_Cond))
			{
				return -1;
			}
		}
		else
		{
			if(pthread_cond_signal(&m_Cond))
			{
				return -1;
			}
		}

		if(pthread_mutex_unlock(&m_Mutex)) return -1;
#else
		return !SetEvent((HANDLE)m_Handle);
#endif

		return 0;
	}

	int OSEvent::reset()
	{
#ifdef _USE_PTHREAD_
		if(pthread_mutex_lock(&m_Mutex) != 0) return -1;
		m_State = false;
		if(pthread_mutex_unlock(&m_Mutex) != 0) return -1;
		return 0;	
#else
		if (ResetEvent(m_Handle))
		{
			return 0;
		} 
		return -1;
#endif
	}

}
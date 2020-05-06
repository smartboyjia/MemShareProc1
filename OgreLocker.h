
#ifndef __Locker_H__
#define __Locker_H__

#ifdef _USE_PTHREAD_
#include <pthread.h>
#endif
#include <assert.h>

namespace Ogre
{
	class LockSection
	{
	public:
		LockSection();
		~LockSection();

		void Lock();
		void Unlock();

	private:
#ifdef _USE_PTHREAD_
        pthread_mutex_t m_pSysObj  ;//= PTHREAD_MUTEX_INITIALIZER;;
#else
		void *m_pSysObj;
#endif

	};

	class LockFunctor
	{
	public:
		LockFunctor( LockSection *pSection ) : m_pSection(pSection)
		{
			if(m_pSection == nullptr) return;
			assert( m_pSection );
			m_pSection->Lock();
		}

		~LockFunctor()
		{
			if( m_pSection )
			{
				m_pSection->Unlock();
			}			
		}

	private:
		LockSection *m_pSection;
	};

	class OSEvent
	{
	public:
		OSEvent(bool init_trigger=false, bool manual_reset=false);
		~OSEvent();

		int wait(unsigned int tick=0xffffffff);
		int trigger();
		int reset();

	private:
#ifdef _USE_PTHREAD_
		bool m_ManualReset;
		bool m_State;
		pthread_mutex_t m_Mutex;
		pthread_cond_t m_Cond;
#else
		void *m_Handle;
#endif
	};
}

#endif
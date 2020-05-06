
#ifndef __OGRE_THREAD_H__
#define __OGRE_THREAD_H__
#include "OgreLocker.h"

namespace Ogre
{
	class OSThread
	{
	public:
		enum RUN_RETTYPE
		{
			RUN_EXIT = 0,
			RUN_WAIT = 1,
			RUN_CONTINUE
		};
	public:
		OSThread();
		virtual ~OSThread();

		void start();
		void shutdown();
		void active()
		{
			m_WaitEvent.trigger();
		}

		virtual unsigned int getDefaultWaitTime() { return 0xffffffff;}

		virtual RUN_RETTYPE _run() = 0;
		virtual void _onThreadStart(){}
		virtual void _onThreadEnd(){}

	public:
#ifdef _USE_PTHREAD_
		pthread_t m_hThread;
#else
		void *m_hThread;
#endif
		OSEvent m_WaitEvent;
		volatile unsigned int m_Shutdown;
		volatile RUN_RETTYPE m_ThreadState;
	};
}

#endif
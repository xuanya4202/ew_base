#include "ewa_base/threading.h"
#include "thread_impl.h"
#include <csignal>
#undef new

EW_ENTER


void _g_signal_terminate(int param)
{

	System::LogTrace("signal_terminate_received(%d)",param);

	if (param == SIGTERM)
	{
		this_logger().LogMessage("SIGTERM Received!");
		::exit(EXIT_FAILURE);
	}

	if (ThreadImpl::sm_bReqexit)
	{
		return;
	}

	if (param == SIGINT)
	{
		this_logger().LogMessage("SIGINT Received!");
	}
	else
	{
		this_logger().LogMessage(String::Format("SIG(%d) Received", param));
	}

	ThreadImpl::sm_bReqexit = true;

}

ThreadManager::ThreadManager()
{

	signal(SIGTERM, _g_signal_terminate);
	signal(SIGINT, _g_signal_terminate);

	m_nThreadMax=10;
	m_nThreadNum=0;
	m_nThreadJob=0;


}


ThreadManager::~ThreadManager()
{

}


int ThreadManager::count()
{
	return m_nThreadNum.get();
}

void ThreadManager::close()
{
	LockGuard<Mutex> lock1(m_thrd_mutex);
	ThreadImpl::sm_bReqexit=1;
	m_nFlags.add(ThreadImpl::THREADMANAGER_DISABLED);
	m_thrd_attached.notify_all();
}

void ThreadManager::wait()
{
	BitFlags oldflag=m_nFlags;
	LockGuard<Mutex> lock1(m_thrd_mutex);
	m_nFlags.add(ThreadImpl::THREADMANAGER_DISABLED);
	m_thrd_attached.notify_all();
	while(m_nThreadNum.get()!=0)
	{
		m_cond_thrd_empty.wait_for(lock1,1000);
	}
	m_nFlags=oldflag;
}


EW_LEAVE

#include "StdAfx.h"
#include "KDThread.h"

CKDThread::CKDThread()
	:	m_hThread(NULL), m_dwThreadId(NULL)
{
	_SetCanThread();
	m_muxThread.Unlock();
}

CKDThread::~CKDThread()
{
	TerminateThread(0);
}

DWORD CKDThread::ThreadProc() {
	return 0;
}

//nPriority:
//	THREAD_PRIORITY_TIME_CRITICAL
//	THREAD_PRIORITY_HIGHEST
//	THREAD_PRIORITY_ABOVE_NORMAL
//	THREAD_PRIORITY_NORMAL
//	THREAD_PRIORITY_BELOW_NORMAL
//	THREAD_PRIORITY_LOWEST
//	THREAD_PRIORITY_IDLE
void CKDThread::CreateThread(int nPriority/* = THREAD_PRIORITY_NORMAL*/, bool bSuspend/* = false*/) {
	if (IsThreadRunning())
		return;

	if (m_muxThread.Lock()) {
		TerminateThread(0);

		DWORD dwCreationFlags = 0;
		if (bSuspend)
			dwCreationFlags = CREATE_SUSPENDED;

		_SetCanThread();
		m_hThread = ::CreateThread(NULL, 0, ThreadProc, (LPVOID) this, dwCreationFlags, &m_dwThreadId);
		VERIFY(m_hThread);
		SetThreadPriority(nPriority);

		m_muxThread.Unlock();
	}
}

void CKDThread::SetCanThread(bool bCanThread/* = true*/) {
	if (m_muxThread.Lock()) {
		_SetCanThread(bCanThread);
		m_muxThread.Unlock();
	}
}

DWORD CKDThread::SuspendThread()
{
	DWORD dwRes = 0;
	if (m_muxThread.Lock()) {
		dwRes = ::SuspendThread(m_hThread);
		m_muxThread.Unlock();
	}
	return dwRes;
}

DWORD CKDThread::ResumeThread()
{
	DWORD dwRes = 0;
	if (m_muxThread.Lock()) {
		dwRes = ::ResumeThread(m_hThread);
		m_muxThread.Unlock();
	}
	return dwRes;
}

//nPriority: (Can only use after CreateThread())
//	THREAD_PRIORITY_TIME_CRITICAL
//	THREAD_PRIORITY_HIGHEST
//	THREAD_PRIORITY_ABOVE_NORMAL
//	THREAD_PRIORITY_NORMAL
//	THREAD_PRIORITY_BELOW_NORMAL
//	THREAD_PRIORITY_LOWEST
//	THREAD_PRIORITY_IDLE
bool CKDThread::SetThreadPriority(int nPriority/* = THREAD_PRIORITY_NORMAL*/) {
	bool bRes = false;
	if (m_muxThread.Lock()) {
		bRes = ::SetThreadPriority(m_hThread, nPriority) == TRUE;
		m_muxThread.Unlock();
	}
	return bRes;
}

bool CKDThread::IsCanThread() {
	bool bRes = false;
	if (m_muxThread.Lock()) {
		if (m_semCanThread.Lock(0)) {
			m_semCanThread.Unlock();
			bRes = false;
		} else {
			bRes = true;
		}
		m_muxThread.Unlock();
	}
	return bRes;
}

bool CKDThread::IsThreadRunning()
{
	return ::WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
	//bool bRes = false;
	//if (m_muxThread.Lock()) {
	//	if (m_semThread.Lock(0)) {
	//		m_semThread.Unlock();
	//		bRes = false;
	//	} else {
	//		bRes = true;
	//	}
	//	m_muxThread.Unlock();
	//}
	//return bRes;
}

HANDLE CKDThread::GetThreadHandle()
{
	return m_hThread;
}

//return vale:
//	WAIT_OBJECT_0
//	WAIT_TIMEOUT
DWORD CKDThread::WaitForThread(DWORD dwMilliseconds) {
	return ::WaitForSingleObject(m_hThread, dwMilliseconds);
}

bool CKDThread::TerminateThread(DWORD dwExitCode/* = 0*/) {
	bool bRes = false;
	if (m_muxThread.Lock()) {
		if (m_hThread) {
			bRes = ::TerminateThread(m_hThread, dwExitCode) == TRUE;
			CloseHandle(m_hThread);
			m_hThread = NULL;
		} else {
			bRes = false;
		}
		m_muxThread.Unlock();
	}
	return bRes;
}

void CKDThread::_SetCanThread(bool bCanThread/* = true*/) {
	if (bCanThread)
		m_semCanThread.Lock(0);
	else
		m_semCanThread.Unlock();
}

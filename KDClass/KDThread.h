#pragma once
#include <afxmt.h>

/*
	inherit CKDThread and Rewrite virtual DWORD ThreadProc();
	Call CreateThread() to Run ThreadProc();

	Care: Setting a config to wait or terminate thread is better!!
	Example:
		SetCanThread(false);
		if (WAIT_TIMEOUT == WaitForThread(10000)) {
#ifdef DEBUG
			AfxMessageBox(_T("Thread is running!!"), MB_OK | MB_ICONERROR);
#endif //DEBUG
			TerminateThread(0);
		}
*/

class CKDThread
{
public:
	CKDThread();
	virtual ~CKDThread();
	virtual DWORD ThreadProc();

	void CreateThread(int nPriority = THREAD_PRIORITY_NORMAL, bool bSuspend = false);
	DWORD SuspendThread();
	DWORD ResumeThread();

	void SetCanThread(bool bCanThread = true);
	bool SetThreadPriority(int nPriority = THREAD_PRIORITY_NORMAL);
	bool IsCanThread();
	bool IsThreadRunning();
	HANDLE GetThreadHandle();
	DWORD WaitForThread(DWORD dwMilliseconds);
	bool TerminateThread(DWORD dwExutCode = 0);

protected:
	HANDLE		m_hThread;
	DWORD		m_dwThreadId;
	CSemaphore	m_semThread;
	CSemaphore	m_semCanThread;

private:
	static DWORD WINAPI ThreadProc(LPVOID pParam) {
		DWORD dwRes = 0;
		CKDThread *pThis = (CKDThread *) pParam;
		if (pThis->m_semThread.Lock(0)) {
			dwRes = pThis->ThreadProc();
			pThis->m_semThread.Unlock();
		}
		return dwRes;
	}
	void _SetCanThread(bool bCanThread = true);

	CMutex		m_muxThread;
};

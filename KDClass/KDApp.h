/*

Inherit CKDApp in MainApp class

If You Want to Support "Get App Other Dir", in StdAfx.h
	#define KDAPP_ENABLE_GETAPPOTHERDIR
If You Want to Support "Update App Online", in StdAfx.h
	#define KDAPP_ENABLE_UPDATEAPPONLINE
If You Want to Support "Get App Version", in StdAfx.h
	#define KDAPP_ENABLE_GETAPPVERSION
	import library "version.lib"
If You Want to Support "Get Changed Dll Dir", in StdAfx.h
	#define KDAPP_ENABLE_GETCHANGEDDLLDIR

*/
#pragma once

class CKDApp
{
public:
	CKDApp();
	virtual ~CKDApp();

	_inline void SetRestart(bool bRestart = true) { m_bRestart = bRestart; }
	_inline LPCTSTR GetAppName() { return m_lpAppName; }
	_inline LPCTSTR GetAppPath() { return m_lpAppPath; }
	_inline LPCTSTR GetAppDir() { return m_lpAppDir; }

private:
	bool m_bRestart;
	LPCTSTR m_lpAppName;
	LPCTSTR m_lpAppPath;
	LPCTSTR m_lpAppDir;

#ifdef KDAPP_ENABLE_GETAPPOTHERDIR
public:
	_inline LPCTSTR GetAppConfDir() { return m_lpAppConfDir; }
	_inline LPCTSTR GetAppLangDir() { return m_lpAppLangDir; }
private:
	LPCTSTR m_lpAppConfDir;
	LPCTSTR m_lpAppLangDir;
#endif //KDAPP_ENABLE_GETAPPOTHERDIR

#ifdef KDAPP_ENABLE_GETAPPVERSION
public:
	_inline LPCTSTR GetAppFileVer() { return m_lpAppFileVer; }
	_inline LPCTSTR GetAppProductVer() { return m_lpAppProductVer; }
private:
	LPCTSTR m_lpAppFileVer;
	LPCTSTR m_lpAppProductVer;
#endif //KDAPP_ENABLE_GETAPPVERSION

#ifdef KDAPP_ENABLE_UPDATEAPPONLINE
public:
	bool GetUpdateAppOnLineVer(LPCTSTR lpQueryUrl, const CStringArray &saQueryAppName,
		const CArray<int, int> &aiQueryVerSize, CStringArray &saReturnVer, CStringArray &saReturnUrl);
	CString GetUpdateAppOnLineVer(LPCTSTR lpQueryUrl, LPCTSTR lpQueryKeyword,
		const LONGLONG i64QueryOffset, const short unsigned int iQueryVerSize);
	void SetUpdateAppShowMsg(bool bShowMsg);
	bool SetUpdateApp(CStringArray &saOldAppPath, CStringArray &saNewAppPath, bool bShowMsg = true);
private:
	bool m_bUpdateApp;
	bool m_bShowUpdateMsg;
	LPCTSTR m_lpTmpBatchPath;
	CStringArray m_saOldAppPath;
	CStringArray m_saNewAppPath;
#endif //KDAPP_ENABLE_UPDATEAPPONLINE

#ifdef KDAPP_ENABLE_GETCHANGEDDLLDIR
public:
	_inline LPCTSTR GetAppDllDir() { return m_lpAppDllDir; }
private:
	LPCTSTR m_lpAppDllDir;
#endif //KDAPP_ENABLE_GETCHANGEDDLLDIR
};

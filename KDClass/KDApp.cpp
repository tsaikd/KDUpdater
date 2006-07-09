#include "StdAfx.h"
#include "afxinet.h"
#include "shlwapi.h"
#include "KDApp.h"

#ifdef KDAPP_ENABLE_UPDATEAPPONLINE
	#define UPDATE_QUERY_SIZE 1024
	#define KDAPP_BATCHFILE_EOL _T("\r\n")
#endif //KDAPP_ENABLE_UPDATEAPPONLINE

CKDApp::CKDApp()
	:	m_bRestart(false),m_lpAppName(NULL), m_lpAppPath(NULL), m_lpAppDir(NULL)
#ifdef KDAPP_ENABLE_GETAPPOTHERDIR
		,m_lpAppConfDir(NULL), m_lpAppLangDir(NULL)
#endif //KDAPP_ENABLE_GETAPPOTHERDIR
#ifdef KDAPP_ENABLE_GETAPPVERSION
		,m_lpAppFileVer(NULL), m_lpAppProductVer(NULL)
#endif //KDAPP_ENABLE_GETAPPVERSION
#ifdef KDAPP_ENABLE_UPDATEAPPONLINE
		,m_bUpdateApp(false), m_bShowUpdateMsg(true), m_lpTmpBatchPath(NULL)
#endif //KDAPP_ENABLE_UPDATEAPPONLINE
#ifdef KDAPP_ENABLE_GETCHANGEDDLLDIR
		,m_lpAppDllDir(NULL)
#endif //KDAPP_ENABLE_GETCHANGEDDLLDIR
		
{
	size_t u64Len;
	TCHAR sBuffer[MAX_PATH], *ptr;

	u64Len = GetModuleFileName(NULL, sBuffer, MAX_PATH);
	if (u64Len) {
		m_lpAppPath = new TCHAR[u64Len + 1];
		_tcscpy((LPTSTR)m_lpAppPath, sBuffer);
	}

	ptr = _tcsrchr(sBuffer, _T('\\'));
	if (ptr) {
		ptr++;
		PathRemoveExtension(ptr);
		m_lpAppName = new TCHAR[_tcslen(ptr) + 1];
		_tcscpy((LPTSTR)m_lpAppName, ptr);
		*(ptr) = _T('\0');
		SetCurrentDirectory(sBuffer);

		u64Len = _tcslen(sBuffer) + 1;
		m_lpAppDir = new TCHAR[u64Len];
		_tcscpy((LPTSTR)m_lpAppDir, sBuffer);

#ifdef KDAPP_ENABLE_GETAPPOTHERDIR
		m_lpAppConfDir = new TCHAR[u64Len + _tcslen(_T("conf\\"))];
		_stprintf((LPTSTR)m_lpAppConfDir, _T("%sconf\\"), m_lpAppDir);
		if (!PathFileExists(m_lpAppConfDir))
			::CreateDirectory(m_lpAppConfDir, NULL);

		m_lpAppLangDir = new TCHAR[u64Len + _tcslen(_T("lang\\"))];
		_stprintf((LPTSTR)m_lpAppLangDir, _T("%slang\\"), m_lpAppDir);
		if (!PathFileExists(m_lpAppLangDir))
			::CreateDirectory(m_lpAppLangDir, NULL);
#endif //KDAPP_ENABLE_GETAPPOTHERDIR

#ifdef KDAPP_ENABLE_UPDATEAPPONLINE
		m_lpTmpBatchPath = new TCHAR[u64Len + _tcslen(_T("tmp.cmd"))];
		_stprintf((LPTSTR)m_lpTmpBatchPath, _T("%stmp.vbs"), m_lpAppDir);
#endif //KDAPP_ENABLE_UPDATEAPPONLINE
	} else {
		MessageBox(NULL, _T("Can not locate the execution file!"), _T("ERROR"), MB_OK | MB_ICONERROR);
	}

#ifdef KDAPP_ENABLE_GETAPPVERSION
	if (m_lpAppPath) {
		DWORD dwRes = GetFileVersionInfoSize(m_lpAppPath, NULL);
		while (dwRes) {
			struct LANGANDCODEPAGE {
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;

			CString sQuery;
			CString sVer;
			TCHAR *btVersion;
			UINT uVersionLen;
			BYTE *pData = new BYTE[dwRes];

			GetFileVersionInfo(m_lpAppPath, NULL, dwRes, pData);

			if (!VerQueryValue(pData, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &uVersionLen))
				break;

			sQuery.Format(_T("\\StringFileInfo\\%04x%04x\\FileVersion"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
			if (VerQueryValue(pData, (LPTSTR)(LPCTSTR)sQuery, (LPVOID *)&btVersion, &uVersionLen)) {
				sVer = btVersion;
				sVer.Replace(_T(" "), _T(""));
				sVer.Replace(_T(","), _T("."));
				m_lpAppFileVer = new TCHAR[uVersionLen + 1];
				_tcscpy((LPTSTR)m_lpAppFileVer, sVer);
			}

			sQuery.Format(_T("\\StringFileInfo\\%04x%04x\\ProductVersion"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
			if (VerQueryValue(pData, (LPTSTR)(LPCTSTR)sQuery, (LPVOID *)&btVersion, &uVersionLen)) {
				sVer = btVersion;
				sVer.Replace(_T(" "), _T(""));
				sVer.Replace(_T(","), _T("."));
				m_lpAppProductVer = new TCHAR[uVersionLen + 1];
				_tcscpy((LPTSTR)m_lpAppProductVer, sVer);
			}

			delete [] pData;
			break;
		}
	}
#endif //KDAPP_ENABLE_GETAPPVERSION

#ifdef KDAPP_ENABLE_GETCHANGEDDLLDIR
	#ifndef BUFSIZE
		#define BUFSIZE MAX_PATH
	#endif //BUFSIZE

	if (m_lpAppPath) {
		m_lpAppDllDir = new TCHAR[u64Len + _tcslen(_T("lib\\"))];
		_stprintf((LPTSTR)m_lpAppDllDir, _T("%slib\\"), m_lpAppDir);
		if (!PathFileExists(m_lpAppDllDir))
			::CreateDirectory(m_lpAppDllDir, NULL);

		bool bRes = false;
		DWORD dwRes;
		TCHAR *lpData = new TCHAR[BUFSIZE];
		dwRes = GetEnvironmentVariable(_T("PATH"), lpData, BUFSIZE);

		if (dwRes) {
			if (BUFSIZE < dwRes) {
				delete [] lpData;
				lpData = new TCHAR[dwRes];

				if (GetEnvironmentVariable(_T("PATH"), lpData, dwRes))
					bRes = true;
			} else {
				bRes = true;
			}
			if (bRes) {
				CString sNewEnvPath;
				sNewEnvPath.Format(_T("%s;%s"), lpData, m_lpAppDllDir);
				SetEnvironmentVariable(_T("PATH"), sNewEnvPath);
			}
		}

		delete [] lpData;
	}
#endif //KDAPP_ENABLE_GETCHANGEDDLLDIR
}

CKDApp::~CKDApp()
{
#ifdef KDAPP_ENABLE_UPDATEAPPONLINE
	while (m_bUpdateApp && m_lpTmpBatchPath) {
		HANDLE hBatchFile = CreateFile(m_lpTmpBatchPath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hBatchFile == INVALID_HANDLE_VALUE)
			break;

		int i, iCount = m_saOldAppPath.GetCount();

		DWORD dwWriteLen;
		CString sBatchContext;

		sBatchContext.AppendFormat(
			_T("Set objShell = CreateObject(\"WScript.Shell\")") KDAPP_BATCHFILE_EOL
			_T("Set objFS = CreateObject(\"Scripting.FileSystemObject\")") KDAPP_BATCHFILE_EOL
			_T("sAppName = \"%s\"") KDAPP_BATCHFILE_EOL
			, m_lpAppName
			);

		if (iCount && (iCount == m_saNewAppPath.GetCount())) {
			sBatchContext.AppendFormat(
				KDAPP_BATCHFILE_EOL
				_T("Const iArraySize = %d") KDAPP_BATCHFILE_EOL
				_T("Dim sOldFile(%d)") KDAPP_BATCHFILE_EOL
				_T("Dim sNewFile(%d)") KDAPP_BATCHFILE_EOL
				KDAPP_BATCHFILE_EOL
				, iCount
				, iCount - 1
				, iCount - 1
				);
		}

		for (i=0 ; i<iCount ; i++) {
			sBatchContext.AppendFormat(
				_T("sOldFile(%d) = \"%s\"") KDAPP_BATCHFILE_EOL
				_T("sNewFile(%d) = \"%s\"") KDAPP_BATCHFILE_EOL
				, i, m_saOldAppPath[i]
				, i, m_saNewAppPath[i]
				);
		}

		if (iCount && (iCount == m_saNewAppPath.GetCount())) {
			sBatchContext.AppendFormat(
				KDAPP_BATCHFILE_EOL
				_T("For i=0 To (iArraySize-1)") KDAPP_BATCHFILE_EOL
				_T("	iLoopMaxTimes = 30") KDAPP_BATCHFILE_EOL
				_T("	While objFS.FileExists(sOldFile(i)) And (iLoopMaxTimes > 0)") KDAPP_BATCHFILE_EOL
				_T("		objFS.DeleteFile sOldFile(i), True") KDAPP_BATCHFILE_EOL
				_T("		iLoopMaxTimes = iLoopMaxTimes - 1") KDAPP_BATCHFILE_EOL
				_T("		If (iLoopMaxTimes < 25) Then") KDAPP_BATCHFILE_EOL
				_T("			WScript.Sleep(1000)") KDAPP_BATCHFILE_EOL
				_T("		End If") KDAPP_BATCHFILE_EOL
				_T("	Wend") KDAPP_BATCHFILE_EOL
				_T("	If objFS.FileExists(sNewFile(i)) Then") KDAPP_BATCHFILE_EOL
				_T("		objFS.MoveFile sNewFile(i), sOldFile(i)") KDAPP_BATCHFILE_EOL
				_T("	End If") KDAPP_BATCHFILE_EOL
				_T("Next") KDAPP_BATCHFILE_EOL
				);

			if (m_bShowUpdateMsg) {
				sBatchContext.AppendFormat(
					KDAPP_BATCHFILE_EOL
					_T("MsgBox \"Application Updated\", vbOKOnly + vbInformation , sAppName") KDAPP_BATCHFILE_EOL
					);
			}
		}

		sBatchContext.AppendFormat(
			KDAPP_BATCHFILE_EOL
			_T("If objFS.FileExists(WScript.ScriptFullName) Then") KDAPP_BATCHFILE_EOL
			_T("	objFS.DeleteFile WScript.ScriptFullName, True") KDAPP_BATCHFILE_EOL
			_T("End If") KDAPP_BATCHFILE_EOL
			_T("objShell.Run sAppName + \".exe\"") KDAPP_BATCHFILE_EOL
			);

		CStringA sBatchContextA;
		sBatchContextA = sBatchContext;
		WriteFile(hBatchFile, sBatchContextA, sBatchContextA.GetLength(), &dwWriteLen, NULL);
		CloseHandle(hBatchFile);

		ShellExecute(NULL, _T("open"), m_lpTmpBatchPath, NULL, NULL, SW_SHOW);

		m_bRestart = false;
		m_bUpdateApp = false;
		break;
	}
	if (m_lpTmpBatchPath)
		delete [] m_lpTmpBatchPath;
#endif //KDAPP_ENABLE_UPDATEAPPONLINE

	if (m_bRestart)
		ShellExecute(NULL, _T("open"), m_lpAppPath, NULL, NULL, SW_SHOW);

	if (m_lpAppName)
		delete [] m_lpAppName;
	if (m_lpAppPath)
		delete [] m_lpAppPath;
	if (m_lpAppDir)
		delete [] m_lpAppDir;
#ifdef KDAPP_ENABLE_GETAPPOTHERDIR
	if (m_lpAppConfDir)
		delete [] m_lpAppConfDir;
	if (m_lpAppLangDir)
		delete [] m_lpAppLangDir;
#endif //KDAPP_ENABLE_GETAPPOTHERDIR
#ifdef KDAPP_ENABLE_GETAPPVERSION
	if (m_lpAppFileVer)
		delete [] m_lpAppFileVer;
	if (m_lpAppProductVer)
		delete [] m_lpAppProductVer;
#endif //KDAPP_ENABLE_GETAPPVERSION
#ifdef KDAPP_ENABLE_GETCHANGEDDLLDIR
	if (m_lpAppDllDir)
		delete [] m_lpAppDllDir;
#endif //KDAPP_ENABLE_GETCHANGEDDLLDIR
}

#ifdef KDAPP_ENABLE_UPDATEAPPONLINE
// AppName Version Url
bool CKDApp::GetUpdateAppOnLineVer(LPCTSTR lpQueryUrl, const CStringArray &saQueryAppName,
	const CArray<int, int> &aiQueryVerSize, CStringArray &saReturnVer, CStringArray &saReturnUrl)
{
	saReturnVer.RemoveAll();
	saReturnUrl.RemoveAll();
	int i, iCount = saQueryAppName.GetCount();
	if ((iCount < 1) || (iCount != aiQueryVerSize.GetCount()))
		return false;
	saReturnVer.SetSize(iCount);
	saReturnUrl.SetSize(iCount);

	bool bRes = true;
	CInternetSession session(_T("Check Update App Session"));
	CStdioFile *pFile = NULL;
	TRY {
		pFile = session.OpenURL(lpQueryUrl);
	} CATCH_ALL(e) {
	} END_CATCH_ALL
	if (pFile) {
		CString sPageContext;
		CString sBuf;
		char *pBuf = new char[UPDATE_QUERY_SIZE];

		while (pFile->Read(pBuf, UPDATE_QUERY_SIZE)) {
			sBuf = pBuf;
			sPageContext.Append(sBuf);
		}

		int iPos;

		iPos = sPageContext.Find(_T("404 Not Found"));
		if (-1 != iPos) {
			bRes = false;
			iCount = 0;
		}

		for (i=0 ; i<iCount ; i++) {
			if (saQueryAppName[i].IsEmpty())
				continue;

			iPos = sPageContext.Find(saQueryAppName[i]);
			if (-1 != iPos) {
				LPCTSTR lpPageContext = sPageContext;
				lpPageContext += iPos + _tcslen(saQueryAppName[i]) + 1;

				CString *psReturnVer = (CString *)&saReturnVer.GetAt(i);
				LPTSTR lpVersion = psReturnVer->GetBuffer(aiQueryVerSize[i] + 1);
				_tcsncpy(lpVersion, lpPageContext, aiQueryVerSize[i]);
				*(lpVersion + aiQueryVerSize[i]) = _T('\0');
				psReturnVer->ReleaseBuffer();

				int iTextWrite = 0;
				psReturnVer = (CString *)&saReturnUrl.GetAt(i);
				lpVersion = psReturnVer->GetBuffer(MAX_PATH);
				lpPageContext += aiQueryVerSize[i] + 1;
				while ((*lpPageContext != _T('\0')) && (*lpPageContext != _T('\n')) && (iTextWrite < MAX_PATH)) {
					*lpVersion = *lpPageContext;
					lpVersion++;
					lpPageContext++;
					iTextWrite++;
				}
				*lpVersion = _T('\0');
				psReturnVer->ReleaseBuffer();
			}
		}

		delete pBuf;
		pFile->Close();
		delete pFile;
	} else {
		bRes = false;
	}

	session.Close();
	return bRes;
}

CString CKDApp::GetUpdateAppOnLineVer(LPCTSTR lpQueryUrl, LPCTSTR lpQueryKeyword,
	const LONGLONG i64QueryOffset, const short unsigned int iQueryVerSize)
{
	CString sRes;

	CInternetSession session(_T("Check Update App Session"));
	CStdioFile *pFile = NULL;
	TRY {
		pFile = session.OpenURL(lpQueryUrl);
	} CATCH_ALL(e) {
	} END_CATCH_ALL
	if (pFile) {
		CString sPageContext;
		CString sBuf;
		char *pBuf = new char[UPDATE_QUERY_SIZE];

		while (pFile->Read(pBuf, UPDATE_QUERY_SIZE)) {
			sBuf = pBuf;
			sPageContext.Append(sBuf);
		}
		int iPos = sPageContext.Find(lpQueryKeyword);
		if (-1 != iPos) {
			LPCTSTR lpPageContext = sPageContext;
			lpPageContext += iPos + i64QueryOffset;

			LPTSTR lpVersion = new TCHAR[iQueryVerSize + 1];
			_tcsncpy(lpVersion, lpPageContext, iQueryVerSize);
			*(lpVersion + iQueryVerSize) = _T('\0');

			sRes = lpVersion;
			delete [] lpVersion;
		}

		delete pBuf;
		pFile->Close();
		delete pFile;
	}

	session.Close();
	return sRes;
}

void CKDApp::SetUpdateAppShowMsg(bool bShowMsg)
{
	m_bShowUpdateMsg = bShowMsg;
}

bool CKDApp::SetUpdateApp(CStringArray &saOldAppPath, CStringArray &saNewAppPath, bool bShowMsg/* = true*/)
{
	if (saOldAppPath.GetCount() != saNewAppPath.GetCount())
		return false;

	SetUpdateAppShowMsg(bShowMsg);

	m_saOldAppPath.RemoveAll();
	m_saNewAppPath.RemoveAll();

	m_saOldAppPath.Append(saOldAppPath);
	m_saNewAppPath.Append(saNewAppPath);

	m_bUpdateApp = true;
	return true;
}
#endif

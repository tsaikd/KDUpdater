#pragma once
#if !defined(_INC_SHLWAPI)
#error Please include <shlwapi.h> and add "shlwapi.lib" to import library
#endif

#define DEL(x)					if ((x)) { delete (x); (x) = NULL; }

#define INRANGE(v, m, M)		((v) >= (m) && (v) <= (M))
#define MAKE_INRANGE(v, m, M)	((v) = min(max((v), (m)), (M)))

#define INRANGE_R(v, m, M)		((v) >= (m) && (v) < (M))
#define INRANGE_L(v, m, M)		((v) > (m) && (v) <= (M))
#define INRANGE_LR(v, m, M)		((v) > (m) && (v) < (M))

inline bool IsCtrlPressed()
{
	return (GetKeyState(VK_CONTROL) & 0x8000) != 0;
}

inline bool IsShiftPressed()
{
	return (GetKeyState(VK_SHIFT) & 0x8000) != 0;
}

inline bool IsAltPressed()
{
	return (GetKeyState(VK_MENU) & 0x8000) != 0;
}

inline bool IsWinPressed()
{
	return (GetKeyState(VK_LWIN) & 0x8000) != 0 || (GetKeyState(VK_RWIN) & 0x8000) != 0;
}

#define KDMOVEDLGITEM_WAY_LEFT		0x01
#define KDMOVEDLGITEM_WAY_TOP		0x02
#define KDMOVEDLGITEM_WAY_RIGHT		0x03
#define KDMOVEDLGITEM_WAY_BOTTOM	0x04
#define KDMOVEDLGITEM_WAY_F_OUTSIDE	0x00
#define KDMOVEDLGITEM_WAY_F_INSIDE	0x10
bool KDMoveDlgItem(CWnd *pItemWnd, CWnd *pRefWnd, int iRefWay, int iRefDist, bool bExtend = false, CPoint *ptOffset = NULL);

//	Add #include <wininet.h> before #include <afxdtctl.h> in "StdAfx.h"
bool ChooseFolder(CString &sFolder, HWND hWnd = 0, LPCTSTR lpTitle = NULL);
bool ChooseFolder(LPTSTR lpFolder, HWND hWnd = 0, LPCTSTR lpTitle = NULL);

bool RemoveFileDlg(LPCTSTR lpFiles, HWND hWnd = NULL, bool bUndo = true);
bool MoveFileDlg(LPCTSTR lpFileFrom, LPCTSTR lpFIleTo, HWND hWnd = NULL, bool bUndo = true);

bool CalcPicSize(CSize &sizeSrc, const CSize &sizeMax);

void GetTempFilePath(LPTSTR lpTempFilePath, LPCTSTR lpTempDir = NULL, LPCTSTR lpPreFix = NULL, LPCTSTR lpPostFix = NULL);
class CTempFilePath : public CString
{
public:
	CTempFilePath(LPCTSTR lpTempDir = NULL, LPCTSTR lpPreFix = NULL, LPCTSTR lpPostFix = NULL)
	{
		LPTSTR lpTempFilePath = GetBuffer(MAX_PATH);
		GetTempFilePath(lpTempFilePath, lpTempDir, lpPreFix, lpPostFix);
		ReleaseBuffer();
	}
};

class CScreenSize : public CSize
{
public:
	CScreenSize(int iOffsetX = 0, int iOffsetY = 0)
	{
		CRect rcWin;
		GetWindowRect(GetDesktopWindow(), rcWin);
		SetSize(rcWin.Width() + iOffsetX, rcWin.Height() + iOffsetY);
	}
};

#if defined(_UNICODE) && defined(_SHLOBJ_H_) && defined(_WININET_) && defined(__ATLBASE_H__) && defined(WPSTYLE_STRETCH)
/*
	Must Turn On Unicode
	include <wininet.h> before include <afxdtctl.h>
*/
bool SetWallpaper(LPCTSTR lpFilePath, DWORD dwStyle = WPSTYLE_STRETCH);
bool AddDesktopPic(LPCTSTR lpPicPath, const CSize &sizePic);
bool RemoveDesktopPic(LPCTSTR lpPicPath);
#endif //defined(_UNICODE) && defined(_SHLOBJ_H_) && defined(_WININET_)

#if defined(_WINSOCK2API_) && defined(_WININET_)
/*
	include <wininet.h> in StdAfx.h
*/
bool DownloadFileFromHttp(LPCTSTR lpURL, LPCTSTR lpLocalPath, int iQuerySize = 8192);

int GetOnInternet(LPCSTR lpszTestHost = "www.google.com", u_short uTestPort = 80);
int GetOnOffline();
#endif //defined(_WINSOCK2API_) && defined(_WININET_)

class CGetFileVersion : public CString
{
public:
	CGetFileVersion(LPCTSTR lpFilePath)
		:	m_lpAppFileVer(NULL), m_lpAppProductVer(NULL)
	{
		DWORD dwLen = GetFileVersionInfoSize(lpFilePath, NULL);
		while (dwLen) {
			struct LANGANDCODEPAGE {
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;

			CString sQuery;
			CString sVer;
			TCHAR *btVersion;
			UINT uVersionLen;
			BYTE *pData = new BYTE[dwLen];

			GetFileVersionInfo(lpFilePath, NULL, dwLen, pData);

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

			if (m_lpAppProductVer)
				SetString(m_lpAppProductVer);
			else if (m_lpAppFileVer)
				SetString(m_lpAppFileVer);

			delete [] pData;
			break;
		}
	}

	~CGetFileVersion()
	{
		if (m_lpAppFileVer)
			delete [] m_lpAppFileVer;
		if (m_lpAppProductVer)
			delete [] m_lpAppProductVer;
	}

	_inline LPCTSTR GetAppFileVer() { return m_lpAppFileVer; }
	_inline LPCTSTR GetAppProductVer() { return m_lpAppProductVer; }
private:
	LPCTSTR m_lpAppFileVer;
	LPCTSTR m_lpAppProductVer;
};

bool ExplorerFile(LPCTSTR lpFilePath);
bool ExplorerDir(LPCTSTR lpDirPath);
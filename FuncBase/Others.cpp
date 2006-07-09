#include "stdafx.h"

#include "Others.h"

//Move Dialog Item to a Reference Position
//Options:
//	pItemWnd:	The Item Wnd What you want to Move
//	pRefWnd:	The Reference Wnd
//	iRefWay:	Move Item Position by which way from Reference
//		KDMOVEDLGITEM_WAY_LEFT:		Left
//		KDMOVEDLGITEM_WAY_TOP:		Top
//		KDMOVEDLGITEM_WAY_RIGHT:	Right
//		KDMOVEDLGITEM_WAY_BOTTOM:	Bottom
//
//		Other Flags:
//		KDMOVEDLGITEM_WAY_F_OUTSIDE: Calculate from Outside (default)
//		KDMOVEDLGITEM_WAY_F_INSIDE: Calculate from Inside
//	iRefDist:	The Min Distance Between nItemID and nRefID
//		Usually Set > 0, because it is the Min Distance
//	bExtend:	Extend Border to the Destination or not
//	ptOffset:	Point to Offset, after Moving Item
bool KDMoveDlgItem(CWnd *pItemWnd, CWnd *pRefWnd, int iRefWay, int iRefDist, bool bExtend/* = false*/, CPoint *ptOffset/* = NULL*/)
{
	if (!IsWindow(pItemWnd->GetSafeHwnd()) || !IsWindow(pRefWnd->GetSafeHwnd()))
		return false;

	CRect rcItem, rcRef;
	pItemWnd->GetWindowRect(rcItem);
	pRefWnd->GetWindowRect(rcRef);

	switch(iRefWay) {
	case KDMOVEDLGITEM_WAY_LEFT | KDMOVEDLGITEM_WAY_F_OUTSIDE:	// OutSide Left
		if (bExtend)
			rcItem.right = rcRef.left - iRefDist;
		else
			rcItem.MoveToX(rcRef.left - rcItem.Width() - iRefDist);
		break;
	case KDMOVEDLGITEM_WAY_LEFT | KDMOVEDLGITEM_WAY_F_INSIDE:	// InSide Left
		if (bExtend)
			rcItem.left = rcRef.left - iRefDist;
		else
			rcItem.MoveToX(rcRef.left + iRefDist);
		break;
	case KDMOVEDLGITEM_WAY_TOP | KDMOVEDLGITEM_WAY_F_OUTSIDE:	// OutSide Top
		if (bExtend)
			rcItem.bottom = rcRef.top - iRefDist;
		else
			rcItem.MoveToY(rcRef.top - rcItem.Height() - iRefDist);
		break;
	case KDMOVEDLGITEM_WAY_TOP | KDMOVEDLGITEM_WAY_F_INSIDE:	// InSide Top
		if (bExtend)
			rcItem.top = rcRef.top - iRefDist;
		else
			rcItem.MoveToY(rcRef.top - rcItem.Height() - iRefDist);
		break;
	case KDMOVEDLGITEM_WAY_RIGHT | KDMOVEDLGITEM_WAY_F_OUTSIDE:	// OutSide Right
		if (bExtend)
			rcItem.left = rcRef.right - iRefDist;
		else
			rcItem.MoveToX(rcRef.right + iRefDist);
		break;
	case KDMOVEDLGITEM_WAY_RIGHT | KDMOVEDLGITEM_WAY_F_INSIDE:	// InSide Right
		if (bExtend)
			rcItem.right = rcRef.right - iRefDist;
		else
			rcItem.MoveToX(rcRef.right - rcItem.Width() - iRefDist);
		break;
	case KDMOVEDLGITEM_WAY_BOTTOM | KDMOVEDLGITEM_WAY_F_OUTSIDE:	// OutSide Bottom
		if (bExtend)
			rcItem.top = rcRef.bottom - iRefDist;
		else
			rcItem.MoveToY(rcRef.bottom + iRefDist);
		break;
	case KDMOVEDLGITEM_WAY_BOTTOM | KDMOVEDLGITEM_WAY_F_INSIDE:	// InSide Bottom
		if (bExtend)
			rcItem.bottom = rcRef.bottom - iRefDist;
		else
			rcItem.MoveToY(rcRef.bottom - rcItem.Height() - iRefDist);
		break;
	default:
		return false;
	}

	if (ptOffset)
		rcItem.OffsetRect(*ptOffset);

	pItemWnd->GetParent()->ScreenToClient(rcItem);
	pItemWnd->MoveWindow(rcItem, FALSE);
	return true;
}

// return true if user choice a folder
bool ChooseFolder(CString &sFolder, HWND hWnd/*= 0*/, LPCTSTR lpTitle/* = NULL*/)
{
	bool bRes = ChooseFolder(sFolder.GetBuffer(MAX_PATH), hWnd, lpTitle);
	sFolder.ReleaseBuffer();

	return bRes;
}

bool ChooseFolder(LPTSTR lpFolder, HWND hWnd/*= 0*/, LPCTSTR lpTitle/* = NULL*/)
{
	LPMALLOC pMalloc;
	bool bOK = false;

	if (SHGetMalloc(&pMalloc) == NOERROR) {
		BROWSEINFO brInfo = {0};
		brInfo.hwndOwner = hWnd;
		brInfo.pszDisplayName = lpFolder;
		if (lpTitle)
			brInfo.lpszTitle = lpTitle;
		brInfo.ulFlags = BIF_USENEWUI;

		CoInitialize(NULL);

		LPITEMIDLIST pidl;
		if ((pidl = SHBrowseForFolder(&brInfo)) != NULL){
			if (SHGetPathFromIDList(pidl, lpFolder))
				bOK = true;
			pMalloc->Free(pidl);
		}
		pMalloc->Release();

		CoUninitialize();
	}

	return bOK;
}

#include "shellapi.h"
// Open a Dialog to ask user for sure
// if not Undo, then remove files directly (default: true)
// if Remove success, then return true
// bUndo usually set to !IsShiftPressed()
bool RemoveFileDlg(LPCTSTR lpFiles, HWND hWnd/* = NULL*/, bool bUndo/* = true*/)
{
	TCHAR sFileBuf[MAX_PATH] = {0};
	_tcscpy(sFileBuf, lpFiles);

	SHFILEOPSTRUCT shFile = {0};
	shFile.hwnd = hWnd;
	shFile.wFunc = FO_DELETE;
	shFile.pFrom = sFileBuf;
	if (bUndo)
		shFile.fFlags = FOF_ALLOWUNDO;
	return (SHFileOperation(&shFile)) == 0;
}

bool MoveFileDlg(LPCTSTR lpFileFrom, LPCTSTR lpFIleTo, HWND hWnd/* = NULL*/, bool bUndo/* = true*/)
{
	TCHAR sFileFromBuf[MAX_PATH] = {0};
	_tcscpy(sFileFromBuf, lpFileFrom);
	TCHAR sFileToBuf[MAX_PATH] = {0};
	_tcscpy(sFileToBuf, lpFIleTo);

	SHFILEOPSTRUCT shFile = {0};
	shFile.hwnd = hWnd;
	shFile.wFunc = FO_MOVE;
	shFile.pFrom = sFileFromBuf;
	shFile.pTo = sFileToBuf;
	if (bUndo)
		shFile.fFlags = FOF_ALLOWUNDO;
	return (SHFileOperation(&shFile)) == 0;
}

// if need resample, then return true, and set new size at cpSizeSrc
// else return false
bool CalcPicSize(CSize &sizeSrc, const CSize &sizeMax)
{
	if ((sizeMax.cx >= sizeSrc.cx) && (sizeMax.cy >= sizeSrc.cy))
		return false;

	double dRatioX, dRatioY;
	dRatioX = (double)sizeMax.cx / (double)sizeSrc.cx;
	dRatioY = (double)sizeMax.cy / (double)sizeSrc.cy;

	double dRatio = (dRatioX<dRatioY) ? dRatioX : dRatioY;
	sizeSrc.SetSize((int)((double)sizeSrc.cx * dRatio), (int)((double)sizeSrc.cy * dRatio));
	return true;
}

// ouput lpTempFilePath and return sTempFilePath, input lpTempDir and lpPreFix
void GetTempFilePath(LPTSTR lpTempFilePath, LPCTSTR lpTempDir/* = NULL*/, LPCTSTR lpPreFix/* = NULL*/, LPCTSTR lpPostFix/* = NULL*/)
{
	if (!lpTempFilePath)
		return;

	LPTSTR lpTempDirBuf = (LPTSTR)lpTempDir;
	LPCTSTR lpPreFixBuf = lpPreFix;

	if (!lpTempDirBuf)
		lpTempDirBuf = new TCHAR [MAX_PATH];
	if (!lpPreFixBuf)
		lpPreFixBuf = _T("_TEMP_");

	GetTempPath(MAX_PATH, lpTempDirBuf);
	GetTempFileName(lpTempDirBuf, lpPreFixBuf, 0, lpTempFilePath);
	if (PathFileExists(lpTempFilePath))
		DeleteFile(lpTempFilePath);

	if (lpPostFix)
		_tcscat(lpTempFilePath, lpPostFix);

	if (lpTempDirBuf && !lpTempDir)
		delete [] lpTempDirBuf;
}

#if defined(_UNICODE) && defined(_SHLOBJ_H_) && defined(_WININET_) && defined(__ATLBASE_H__) && defined(WPSTYLE_STRETCH)

//WallPaper Options
//	WPSTYLE_CENTER
//	WPSTYLE_TILE
//	WPSTYLE_STRETCH (default)
//	WPSTYLE_MAX
bool SetWallpaper(LPCTSTR lpFilePath, DWORD dwStyle/* = WPSTYLE_STRETCH*/)
{
	if (!lpFilePath || !PathFileExists(lpFilePath))
		return false;

	CoInitialize(NULL);

	bool bMethod1 = true, bMethod2 = true;
	IActiveDesktop *pActiveDesktop;

	#define TRYEXP(exp) { if ((exp) != S_OK) throw 1; }

	try {
		WALLPAPEROPT wpOpt;
		//Create an instance of the Active Desktop
		TRYEXP(CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
							IID_IActiveDesktop, (void**) &pActiveDesktop));
		TRYEXP(pActiveDesktop->SetWallpaper(lpFilePath, 0));

		wpOpt.dwSize = sizeof(WALLPAPEROPT);
		wpOpt.dwStyle = dwStyle;

		TRYEXP(pActiveDesktop->SetWallpaperOptions(&wpOpt, 0));
		TRYEXP(pActiveDesktop->ApplyChanges(AD_APPLY_ALL));
	} catch (...) {
		bMethod1 = false;
	}

	#undef TRYEXP

	if (pActiveDesktop)
		pActiveDesktop->Release();
	else {
		CString cstrTileWallpaper, cstrWallpaperStyle;
		
		
		if (dwStyle == WPSTYLE_TILE)
		{
			// Tiled
			cstrTileWallpaper = _T("1");
			cstrWallpaperStyle = _T("1");
		}
		else if (dwStyle == WPSTYLE_CENTER)
		{
			// Center
			cstrTileWallpaper = _T("0");
			cstrWallpaperStyle = _T("1");
		}
		else//if (dwStyle == WPSTYLE_STRETCH)
		{
			// Stretch
			cstrTileWallpaper = _T("0");
			cstrWallpaperStyle = _T("2");
		}

		CRegKey reg;

		#define TRYREG(exp) {if ((exp) != ERROR_SUCCESS) throw 1;}

		try {
			TRYREG(reg.Create(HKEY_CURRENT_USER, _T("Control Panel\\Desktop")));
			TRYREG(reg.SetStringValue(_T("TileWallpaper"), cstrTileWallpaper));
			TRYREG(reg.SetStringValue(_T("WallpaperStyle"), cstrWallpaperStyle));

			bMethod2 = (SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void *) lpFilePath,
				SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE) != FALSE);
		} catch (...) {
			bMethod2 = false;
		}

		#undef TRYREG
	}

	CoUninitialize();

	return bMethod1 || bMethod2;
}

bool AddDesktopPic(LPCTSTR lpPicPath, const CSize &sizePic)
{
	if (!lpPicPath || !PathFileExists(lpPicPath))
		return false;

	bool bRes = true;
	CoInitialize(NULL);
	COMPONENT compDesk = {0};
	IActiveDesktop *pActiveDesktop = NULL;
	CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (void**) &pActiveDesktop);

	compDesk.dwSize = sizeof(COMPONENT);
	compDesk.iComponentType = COMP_TYPE_PICTURE;
	compDesk.fChecked = TRUE;
	compDesk.fDirty = FALSE;
	compDesk.fNoScroll = FALSE;
	compDesk.dwCurItemState = IS_NORMAL;
	_tcscpy(compDesk.wszFriendlyName, PathFindFileName(lpPicPath));
	_tcscpy(compDesk.wszSource, lpPicPath);

	CScreenSize sizeScreen;
	CSize sizePicBuf = sizePic;
	CalcPicSize(sizePicBuf, sizeScreen);
	compDesk.cpPos.dwSize = sizeof(COMPPOS);
	compDesk.cpPos.iLeft = (sizeScreen.cx - sizePicBuf.cx) >> 1;
	compDesk.cpPos.iTop = (sizeScreen.cy - sizePicBuf.cy) >> 1;
	compDesk.cpPos.dwWidth = sizePic.cx;
	compDesk.cpPos.dwHeight = sizePic.cy;
	compDesk.cpPos.izIndex = 0;
	compDesk.cpPos.fCanResize = FALSE;
	compDesk.cpPos.fCanResizeX = FALSE;
	compDesk.cpPos.fCanResizeY = FALSE;

	ASSERT(bRes = (pActiveDesktop->AddDesktopItem(&compDesk, 0) == S_OK) && bRes);
	ASSERT(bRes = (pActiveDesktop->ApplyChanges(AD_APPLY_ALL) == S_OK) && bRes);

	if (pActiveDesktop)
		pActiveDesktop->Release();
	CoUninitialize();

	return bRes;
}

bool RemoveDesktopPic(LPCTSTR lpPicPath)
{
	if (!lpPicPath)
		return false;

	bool bRes = true;
	CoInitialize(NULL);
	COMPONENT compDesk = {0};
	IActiveDesktop *pActiveDesktop = NULL;
	CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (void**) &pActiveDesktop);

	compDesk.dwSize = sizeof(COMPONENT);
	_tcscpy(compDesk.wszSource, lpPicPath);
	ASSERT(bRes = (pActiveDesktop->RemoveDesktopItem(&compDesk, 0) == S_OK) && bRes);
	ASSERT(bRes = (pActiveDesktop->ApplyChanges(AD_APPLY_ALL) == S_OK) && bRes);

	if (pActiveDesktop)
		pActiveDesktop->Release();
	CoUninitialize();

	return bRes;
}

#endif // defined(_UNICODE) && defined(_SHLOBJ_H_) && defined(_WININET_)

#if defined(_WINSOCK2API_) && defined(_WININET_)
#include "afxinet.h"
bool DownloadFileFromHttp(LPCTSTR lpURL, LPCTSTR lpLocalPath, int iQuerySize/* = 8192*/)
{
	bool bRes = false;
	CInternetSession session(_T("Download File Session"));
	CStdioFile *pFile = session.OpenURL(lpURL);
	HANDLE hLocalFile = CreateFile(lpLocalPath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (pFile && (hLocalFile != INVALID_HANDLE_VALUE)) {
		BYTE *pBuf = new BYTE[iQuerySize];
		UINT uReadLen; 
		DWORD dwWriteLen; 

		while (uReadLen = pFile->Read(pBuf, iQuerySize))
			WriteFile(hLocalFile, pBuf, uReadLen, &dwWriteLen, NULL);

		delete pBuf;
		CloseHandle(hLocalFile);
		bRes = true;
	} else {
		bRes = false;
	}

	if (pFile) {
		pFile->Close();
		delete pFile;
	}

	session.Close();
	return bRes;
}

//return:
//	0: Test Success
//	1: Can't Find a Usable WinSock DLL
//	2: Create Socket Error
//	3: Bind Socket Error
//	4: Invalid Host Name
//	5: Connect Host Error
#define RETURN(x) { WSACleanup(); return (x); }
int GetOnInternet(LPCSTR lpszTestHost/* = "www.google.com"*/, u_short uTestPort/* = 80*/)
{
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
		return 1;
	if ((LOBYTE(wsaData.wVersion) != 2) || (HIBYTE(wsaData.wVersion) != 2))
		RETURN(1);

	SOCKET cli = INVALID_SOCKET;
	struct sockaddr_in cliAddrInfo;
	struct sockaddr_in srvAddrInfo;
	struct hostent *host;

	cliAddrInfo.sin_family = AF_INET;
	cliAddrInfo.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	cliAddrInfo.sin_port = htons(0); // auto assigned by client
	cli = socket(AF_INET, SOCK_STREAM, 0);
	if(cli == INVALID_SOCKET)
		RETURN(2);

	if(bind(cli, (struct sockaddr *) &cliAddrInfo, sizeof(cliAddrInfo)) == SOCKET_ERROR)
		RETURN(3);

	srvAddrInfo.sin_addr.S_un.S_addr = inet_addr(lpszTestHost);
	if (srvAddrInfo.sin_addr.S_un.S_addr == -1) {
		host = gethostbyname(lpszTestHost);
		if (host == NULL)
			RETURN(4);
		memcpy(&(srvAddrInfo.sin_addr), host->h_addr_list[0], host->h_length);
	}

	srvAddrInfo.sin_family = AF_INET;
	srvAddrInfo.sin_port = htons(uTestPort);

	if (connect(cli, (struct sockaddr *) &srvAddrInfo, sizeof(srvAddrInfo))  == SOCKET_ERROR)
		RETURN(5);

	closesocket(cli);
	RETURN(0);
}
#undef RETURN

//return:
//	if On Offline, return 1
//	if On Internet, return 0
//	if Can't Load Library, return -1
int GetOnOffline()
{
	HINSTANCE urldll = LoadLibrary(_T("url.dll"));
	if (!urldll)
		return -1;

	int (WINAPI* offline)(DWORD);
	offline = (int (WINAPI*)(DWORD)) GetProcAddress(urldll, "InetIsOffline");
	if (!offline) {
		FreeLibrary(urldll);
		return -1;
	}

	if (offline(0)) {
		FreeLibrary(urldll);
		return 0;
	} else {
		FreeLibrary(urldll);
		return 1;
	}
}
#endif //defined(_WINSOCK2API_) && defined(_WININET_)

// Open explorer at the directory of file and select the file
bool ExplorerFile(LPCTSTR lpFilePath)
{
	if (PathFileExists(lpFilePath)) {
		CString sExplorer;
		GetSystemWindowsDirectory(sExplorer.GetBuffer(MAX_PATH), MAX_PATH);
		sExplorer.ReleaseBuffer();
		sExplorer.AppendFormat(_T("\\explorer.exe"));

		if (PathFileExists(sExplorer)) {
			CString sParam;
			sParam.Format(_T("/select,\"%s\""), lpFilePath);
			ShellExecute(NULL, _T("open"), sExplorer, sParam, NULL, SW_SHOW);
			return true;
		}
	}

	return false;
}

// Open explorer at the directory
bool ExplorerDir(LPCTSTR lpDirPath)
{
	CString sPath = lpDirPath;
	if (sPath.IsEmpty())
		return false;

	if (!PathIsDirectory(sPath)) {
		PathRemoveFileSpec(sPath.GetBuffer());
		sPath.ReleaseBuffer();
	}
	if (!PathIsDirectory(sPath))
		return false;

	ShellExecute(NULL, _T("open"), sPath, NULL, NULL, SW_SHOW);

	return true;
}

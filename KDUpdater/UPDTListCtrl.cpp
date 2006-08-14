#include "StdAfx.h"
#include "Resource.h"
#include "Ini.h"
#include "Others.h"
#include "SHA.h"
#include "KDUpdater.h"
#include "KDUpdaterDlg.h"

#include "UPDTListCtrl.h"

CUPDTListCtrl::CUPDTListCtrl()
	:	m_hOtherKDUpdater(NULL), m_bDownloadFailed(false), m_uhRegWnd(0), m_bNeedRevert(false)
{
}

CUPDTListCtrl::~CUPDTListCtrl()
{
}

void CUPDTListCtrl::Init()
{
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	CRect rcWin;
	GetClientRect(rcWin);
	InsertColumn(0, _T("File List"), LVCFMT_LEFT, rcWin.Width());

	GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILESIZE)->SetWindowText(_T("0"));
}

void CUPDTListCtrl::OnDestroy()
{
	__super::OnDestroy();

	SetCanThread(false);
	if (WAIT_TIMEOUT == WaitForThread(10000)) {
#ifdef DEBUG
		AfxMessageBox(_T("CUPDTListCtrl Thread is running!!"), MB_OK | MB_ICONERROR);
#endif //DEBUG
		TerminateThread(0);
	}
}

bool CUPDTListCtrl::LoadSetting(LPCTSTR lpFilePath)
{
	if (!PathFileExists(lpFilePath))
		return false;
	int i, iCount;
	CIni ini;
	CString sListURL;
	CStringArray saSections;
	CUPDTListItem *pItem;

	DeleteAllItems();
	ini.SetPathName(lpFilePath);

	((CKDUpdaterDlg *)GetParent())->m_hRegWnd = (HWND)ini.GetUInt(_T("General"), _T("hRegWnd"), 0);
	if (((CKDUpdaterDlg *)GetParent())->m_hRegWnd) {
		EnumWindows(QueryOtherKDUpdater, (LPARAM)this);
		if (m_hOtherKDUpdater) {
			theApp.Quit();
			return false;
		}
	}

	// Load hide information
	m_uhRegWnd = ini.GetUInt(_T("General"), _T("hRegWnd"), 0);
	m_sWorkDir = ini.GetString(_T("General"), _T("sWorkDir"));
	if (!m_sWorkDir.IsEmpty() && PathIsDirectory(m_sWorkDir))
		SetCurrentDirectory(m_sWorkDir);
	m_sPostCmd = ini.GetString(_T("General"), _T("sPostCmd"));

	// Load other information
	sListURL = ini.GetString(_T("General"), _T("sListURL"));
	GetParent()->GetDlgItem(IDC_UPDT_EDIT_LISTURL)->SetWindowText(sListURL);

	ini.GetSectionNames(&saSections);
	iCount = saSections.GetCount();
	for (i=0 ; i<iCount ; i++) {
		if (saSections[i] == _T("General"))
			continue;

		pItem = new CUPDTListItem;
		pItem->m_sFilePath	= ini.GetString(saSections[i], _T("sFilePath"));
		pItem->m_sVersion	= ini.GetString(saSections[i], _T("sVersion"));
		pItem->m_uFileSize	= ini.GetUInt(saSections[i], _T("uFileSize"), 0);
		pItem->m_sFileURL	= ini.GetString(saSections[i], _T("sFileURL"));
		pItem->m_sSha1Hash	= ini.GetString(saSections[i], _T("sSha1Hash"));
		pItem->m_bOptional	= ini.GetBool(saSections[i], _T("bOptional"), DEFAULT_UPDT_LISTITEM_BOPTIONAL);

		InsertItem(LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0,
			I_IMAGECALLBACK, (LPARAM) pItem);
	}
	return true;
}

void CUPDTListCtrl::SaveSetting(LPCTSTR lpFilePath)
{
	int i, iCount;
	CIni ini;
	CString sSection;
	CUPDTListItem *pItem;

	if (PathFileExists(lpFilePath))
		DeleteFile(lpFilePath);
	ini.SetPathName(lpFilePath);

	// Save hide information
	if (m_uhRegWnd)
		ini.WriteUInt(_T("General"), _T("hRegWnd"), m_uhRegWnd);
	if (!m_sWorkDir.IsEmpty())
		ini.WriteString(_T("General"), _T("sWorkDir"), m_sWorkDir);
	if (!m_sPostCmd.IsEmpty())
		ini.WriteString(_T("General"), _T("sPostCmd"), m_sPostCmd);

	// sSection for tmp
	GetParent()->GetDlgItem(IDC_UPDT_EDIT_LISTURL)->GetWindowText(sSection);
	ini.WriteString(_T("General"), _T("sListURL"), sSection);

	iCount = GetItemCount();
	for (i=0 ; i<iCount ; i++) {
		pItem = (CUPDTListItem *)GetItemData(i);
		if (!pItem)
			continue;

		sSection.Format(_T("File%d"), i+1);
		ini.WriteString(sSection, _T("sFilePath"), pItem->m_sFilePath);
		ini.WriteString(sSection, _T("sVersion"), pItem->m_sVersion);
		if (pItem->m_uFileSize)
			ini.WriteUInt(sSection, _T("uFileSize"), pItem->m_uFileSize);
		if (!pItem->m_sFileURL.IsEmpty())
			ini.WriteString(sSection, _T("sFileURL"), pItem->m_sFileURL);
		if (!pItem->m_sSha1Hash.IsEmpty())
			ini.WriteString(sSection, _T("sSha1Hash"), pItem->m_sSha1Hash);
		if (pItem->m_bOptional != DEFAULT_UPDT_LISTITEM_BOPTIONAL)
			ini.WriteBool(sSection, _T("bOptional"), pItem->m_bOptional);
	}
}

void CUPDTListCtrl::DoUpdate()
{
	if (!IsThreadRunning())
		CreateThread();
}

#define RETURN(x) { DeleteFile(sListIni); sListIni.Append(_T(".UTF8.bak")); DeleteFile(sListIni); GetParent()->GetDlgItem(IDC_UPDT_EDIT_LISTURL)->EnableWindow(TRUE); m_muxUpdate.Unlock(); return ((x)); }
bool CUPDTListCtrl::IsNeedUpdate(bool bPrepareDL/* = false*/)
{
	if (0 != GetOnInternet())
		return false;

	m_muxUpdate.Lock();
	GetParent()->GetDlgItem(IDC_UPDT_EDIT_LISTURL)->EnableWindow(FALSE);

	CString sBuf;
	CString sListIni;
	sListIni.Format(_T("%s%s"), theApp.GetAppDir(), _T("KDUpdate_List.ini.tmp"));
	if (bPrepareDL)
		m_aDLItem.RemoveAll();

	GetParent()->GetDlgItem(IDC_UPDT_EDIT_LISTURL)->GetWindowText(sBuf);
	if (sBuf.IsEmpty())
		RETURN(false);

	TRY {
		m_bDownloadFailed = false;
		DownloadFileFromHttp(sBuf, sListIni);
	} CATCH_ALL(e) {
		m_bDownloadFailed = true;
		e->ReportError();
	} END_CATCH_ALL;
	if (!PathFileExists(sListIni))
		RETURN(false);

	m_bNeedRevert = false;
	int i, iCount;
	int j, jCount;
	CIni iniList;
	CStringArray saSection;
	CStringArray saFilePath;
	CArray<CUPDTListItem> aItem;
	CArray<CUPDTListItem> aLocalItem;
	CUPDTListItem item;
	CUPDTListItem *pItem = NULL;

	iniList.SetPathName(sListIni);
	iniList.GetSectionNames(&saSection);
	iCount = saSection.GetCount();
	if (iCount == 0)
		RETURN(false);
	for (i=0 ; i<iCount ; i++) {
		if (saSection[i] == _T("General"))
			continue;

		item.m_sFilePath	= iniList.GetString(saSection[i], _T("sFilePath"));
		saFilePath.Add(item.m_sFilePath);
		item.m_sVersion		= iniList.GetString(saSection[i], _T("sVersion"));
		item.m_uFileSize	= iniList.GetUInt(saSection[i], _T("uFileSize"), 0);
		item.m_sFileURL		= iniList.GetString(saSection[i], _T("sFileURL"));
		item.m_sSha1Hash	= iniList.GetString(saSection[i], _T("sSha1Hash"));
		item.m_bOptional	= iniList.GetBool(saSection[i], _T("bOptional"), DEFAULT_UPDT_LISTITEM_BOPTIONAL);

		aItem.Add(item);
	}

	iCount = GetItemCount();
	for (i=0 ; i<iCount ; i++) {
		pItem = (CUPDTListItem *) GetItemData(i);
		if (!pItem)
			continue;

		aLocalItem.Add(*pItem);
	}

	iCount = saFilePath.GetCount();
	for (i=0 ; i<iCount ; i++) {
		pItem = NULL;
		jCount = aLocalItem.GetCount();
		for (j=0 ; j<jCount ; j++) {
			pItem = &aLocalItem[j];
			if (saFilePath[i] == pItem->m_sFilePath)
				break;
		}

		if (j<jCount) {
			// Find matched file
			if (aItem[i].m_sVersion > pItem->m_sVersion) {
				if (bPrepareDL)
					m_aDLItem.Add(aItem[i]);
				else
					RETURN(true);
			} else if (aItem[i].m_sVersion == pItem->m_sVersion) {
				if (!aItem[i].m_sSha1Hash.IsEmpty()) {
					if (pItem->m_sSha1Hash.IsEmpty()) {
						if (PathFileExists(pItem->m_sFilePath)) {
							CSHA sha;
							pItem->m_sSha1Hash = sha.GetHashStringFromFile(pItem->m_sFilePath, TRUE);
						}
					}
					if ((!pItem->m_sSha1Hash.IsEmpty()) && (aItem[i].m_sSha1Hash != pItem->m_sSha1Hash)) {
						if (bPrepareDL)
							m_aDLItem.Add(aItem[i]);
						else
							m_bNeedRevert = true;
					}
				}
			}
			aLocalItem.RemoveAt(j);
		} else if (aItem[i].m_bOptional != TRUE) {
			// The file is not in local Filelist, need to download
			if (bPrepareDL)
				m_aDLItem.Add(aItem[i]);
			else
				RETURN(true);
		}
	}

	if (bPrepareDL) {
		iCount = aLocalItem.GetCount();
		for (i=0 ; i<iCount ; i++) {
			// Delete useless files
			pItem = &aLocalItem[i];
			if (pItem) {
				sBuf.Format(_T("%s%s"), theApp.GetAppDir(), pItem->m_sFilePath);
				DeleteFile(sBuf);
			}
		}

		if (!m_aDLItem.IsEmpty())
			RETURN(true);
	}

	RETURN(m_bNeedRevert);
}
#undef RETURN

DWORD CUPDTListCtrl::ThreadProc()
{
	if (0 != GetOnInternet())
		return 0;

	// Lock Items
	GetParent()->GetDlgItem(IDC_UPDT_BTN_IMPORT)->EnableWindow(FALSE);
	GetParent()->GetDlgItem(IDC_UPDT_BTN_EXPORT)->EnableWindow(FALSE);
	GetParent()->GetDlgItem(IDC_UPDT_BTN_ADDFILE)->EnableWindow(FALSE);
	GetParent()->GetDlgItem(IDC_UPDT_BTN_REMOVE)->EnableWindow(FALSE);

	if (IsNeedUpdate(true)) {
		int i, iCount;
		CSHA sha;
		CString sSha1Hash;
		CString sBuf;
		CString sUpdatePath;
		CString sUpdateDir;
		GetCurrentDirectory(MAX_PATH, sUpdateDir.GetBuffer(MAX_PATH));
		sUpdateDir.ReleaseBuffer();

		m_muxUpdate.Lock();
		iCount = m_aDLItem.GetCount();
		for (i=0 ; i<iCount ; i++) {
			sUpdatePath.Format(_T("%s\\%s"), sUpdateDir, m_aDLItem[i].m_sFilePath);
			sUpdatePath.Replace(_T("/"), _T("\\"));

			sBuf = sUpdatePath;
			PathRemoveFileSpec(sBuf.GetBuffer());
			sBuf.ReleaseBuffer();
			if (!sBuf.IsEmpty() && !PathFileExists(sBuf))
				SHCreateDirectoryEx(NULL, sBuf, NULL);

			sBuf = sUpdatePath + _T(".tmp");
			if (DownloadFileFromHttp(m_aDLItem[i].m_sFileURL, sBuf)) {
				// Update to new file
				if (!m_aDLItem[i].m_sSha1Hash.IsEmpty()) {
					m_aDLItem[i].m_sSha1Hash.MakeUpper();
					sSha1Hash = sha.GetHashStringFromFile(sBuf, TRUE);
					if ((sSha1Hash != m_aDLItem[i].m_sSha1Hash) &&
						(IDYES == MessageBox(_T("The SHA1 checksum error!\nDo you want to download again?"),
						NULL, MB_YESNO | MB_ICONERROR))) {
						i--;
						continue;
					}
				}
				MoveFileEx(sBuf, sUpdatePath, MOVEFILE_REPLACE_EXISTING);
			} else {
				// Download failed
				sBuf.Format(_T("%s\nThe file download failed!\nDo you want to try again?"), m_aDLItem[i].m_sFileURL);
				if (IDYES == MessageBox(sBuf, NULL, MB_YESNO | MB_ICONERROR)) {
					i--;
					continue;
				}
			}
		}
		m_muxUpdate.Unlock();

		m_aDLItem.RemoveAll();
	}

	// Unlock Items
	GetParent()->GetDlgItem(IDC_UPDT_BTN_IMPORT)->EnableWindow(TRUE);
	GetParent()->GetDlgItem(IDC_UPDT_BTN_EXPORT)->EnableWindow(TRUE);
	GetParent()->GetDlgItem(IDC_UPDT_BTN_ADDFILE)->EnableWindow(TRUE);
	GetParent()->GetDlgItem(IDC_UPDT_BTN_REMOVE)->EnableWindow(TRUE);

	GetParent()->PostMessage(KDUPM_UPDATEOVER);

	return 0;
}

bool CUPDTListCtrl::AddItem()
{
	int iRes = 0;
	int iPos = GetItemCount();
	CUPDTListItem *pItem = new CUPDTListItem;
	CString sBuf;
	CWnd *pWnd = NULL;

	pWnd = GetParent()->GetDlgItem(IDC_UPDT_EDIT_LISTURL);
	pWnd->GetWindowText(sBuf);
	if (sBuf.IsEmpty()) {
		MessageBox(_T("Please enter update list URL!"), NULL, MB_OK | MB_ICONERROR);
		pWnd->SetFocus();
		delete pItem;
		return false;
	}

	pWnd = GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILEPATH);
	pWnd->GetWindowText(sBuf);
	if (sBuf.IsEmpty()) {
		MessageBox(_T("Please enter file path!"), NULL, MB_OK | MB_ICONERROR);
		pWnd->SetFocus();
		delete pItem;
		return false;
	}
	pItem->m_sFilePath = sBuf;

	pWnd = GetParent()->GetDlgItem(IDC_UPDT_EDIT_VERSION);
	pWnd->GetWindowText(sBuf);
	if (sBuf.IsEmpty()) {
		MessageBox(_T("Please enter file version!"), NULL, MB_OK | MB_ICONERROR);
		pWnd->SetFocus();
		delete pItem;
		return false;
	}
	pItem->m_sVersion = sBuf;

	pWnd = GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILESIZE);
	pWnd->GetWindowText(sBuf);
	if (!sBuf.IsEmpty())
		pItem->m_uFileSize = _ttoi(sBuf);

	pWnd = GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILEURL);
	pWnd->GetWindowText(sBuf);
	if (!sBuf.IsEmpty())
		pItem->m_sFileURL = sBuf;

	pWnd = GetParent()->GetDlgItem(IDC_UPDT_EDIT_SHA1HASH);
	pWnd->GetWindowText(sBuf);
	if (!sBuf.IsEmpty())
		pItem->m_sSha1Hash = sBuf;

	pWnd = GetParent()->GetDlgItem(IDC_UPDT_CHECK_OPTIONAL);
	if (((CButton *)pWnd)->GetCheck() == BST_CHECKED)
		pItem->m_bOptional = TRUE;

	int iTmp = FindItemByText(pItem->m_sFilePath);
	if (iTmp >= 0) {
		iPos = iTmp;
		DeleteItem(iPos);
	}
	iRes = InsertItem(LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE, iPos, LPSTR_TEXTCALLBACK, 0, 0,
		I_IMAGECALLBACK, (LPARAM) pItem);
	if (iRes == -1) {
		delete pItem;
		return false;
	}
	return true;
}

void CUPDTListCtrl::RemoveSelItem()
{
	POSITION pos = GetFirstSelectedItemPosition();
	if (pos)
		DeleteItem(GetNextSelectedItem(pos));
}

LPARAM CUPDTListCtrl::GetFirstSelectedItemLParam()
{
	if (!GetSelectedCount())
		return NULL;
	POSITION pos = GetFirstSelectedItemPosition();
	return (LPARAM)GetItemData(GetNextSelectedItem(pos));
}

int CUPDTListCtrl::FindItemByText(LPCTSTR lpText)
{
	LVFINDINFO itemInfo;
	itemInfo.flags = LVFI_STRING;
	itemInfo.psz = lpText;
	return FindItem(&itemInfo);
}

BOOL CALLBACK CUPDTListCtrl::QueryOtherKDUpdater(HWND hWnd, LPARAM lParam) {
	DWORD dwMsgResult;
	CUPDTListCtrl *pThis = (CUPDTListCtrl *)lParam;
	if (!pThis)
		return TRUE;

	if (hWnd == theApp.m_pMainWnd->GetSafeHwnd())
		return TRUE;

	LRESULT res = ::SendMessageTimeout(hWnd, WMU_KDUPDATER_REQ, (WPARAM)((CKDUpdaterDlg *)pThis->GetParent())->m_hRegWnd,
		0, SMTO_BLOCK | SMTO_ABORTIFHUNG, 1000, &dwMsgResult);

	if (res == 0)
		return TRUE;

	if (dwMsgResult == WMU_KDUPDATER_RES) {
		pThis->m_hOtherKDUpdater = hWnd; 
		return FALSE;
	}

	return TRUE; 
}

BEGIN_MESSAGE_MAP(CUPDTListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CUPDTListCtrl::OnLvnGetdispinfo)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, &CUPDTListCtrl::OnLvnDeleteitem)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CUPDTListCtrl::OnLvnItemchanged)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CUPDTListCtrl::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (pDispInfo->item.mask & LVIF_TEXT) {
		CUPDTListItem *pItem = (CUPDTListItem *) pDispInfo->item.lParam;
		pDispInfo->item.pszText = (LPTSTR)(LPCTSTR)pItem->m_sFilePath;
	}

	*pResult = 0;
}

void CUPDTListCtrl::OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	CUPDTListItem *pItem = (CUPDTListItem *) pNMLV->lParam;
	if (pItem) {
		delete pItem;
	}

	*pResult = 0;
}

void CUPDTListCtrl::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	UNUSED_ALWAYS(pNMLV);

	CUPDTListItem *pItem = (CUPDTListItem *) GetFirstSelectedItemLParam();
	if (pItem) {
		GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILEPATH)->SetWindowText(pItem->m_sFilePath);
		GetParent()->GetDlgItem(IDC_UPDT_EDIT_VERSION)->SetWindowText(pItem->m_sVersion);
		if (pItem->m_uFileSize) {
			CString sBuf;
			sBuf.Format(_T("%d"), pItem->m_uFileSize);
			GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILESIZE)->SetWindowText(sBuf);
		} else {
			GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILESIZE)->SetWindowText(_T("0"));
		}
		GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILEURL)->SetWindowText(pItem->m_sFileURL);
		GetParent()->GetDlgItem(IDC_UPDT_EDIT_SHA1HASH)->SetWindowText(pItem->m_sSha1Hash);
		((CButton *)GetParent()->GetDlgItem(IDC_UPDT_CHECK_OPTIONAL))->SetCheck(pItem->m_bOptional ? BST_CHECKED : BST_UNCHECKED);
	} else {
		GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILEPATH)->SetWindowText(_T(""));
		GetParent()->GetDlgItem(IDC_UPDT_EDIT_VERSION)->SetWindowText(_T(""));
		GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILESIZE)->SetWindowText(_T("0"));
		GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILEURL)->SetWindowText(_T(""));
		GetParent()->GetDlgItem(IDC_UPDT_EDIT_SHA1HASH)->SetWindowText(_T(""));
		((CButton *)GetParent()->GetDlgItem(IDC_UPDT_CHECK_OPTIONAL))->SetCheck(DEFAULT_UPDT_LISTITEM_BOPTIONAL ? BST_CHECKED : BST_UNCHECKED);
	}

	*pResult = 0;
}

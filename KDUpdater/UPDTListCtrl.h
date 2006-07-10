#pragma once
#include "afxcmn.h"
#include "../KDClass/KDThread.h"
#include "../KDClass/KDAppVer.h"

class CUPDTListItem
{
public:
	CUPDTListItem() : m_uFileSize(0) {}
	~CUPDTListItem() {}

	CString m_sFilePath;
	CKDAppVer m_sVersion;
	UINT m_uFileSize;
	CString m_sFileURL;
};

class CUPDTListCtrl : public CListCtrl, public CKDThread
{
	DECLARE_MESSAGE_MAP()
public:
	CUPDTListCtrl();
	~CUPDTListCtrl();

	void Init();
	bool LoadSetting(LPCTSTR lpFilePath);
	void SaveSetting(LPCTSTR lpFilePath);
	void DoUpdate();
	bool IsNeedUpdate(bool bPrepareDL = false);

	virtual DWORD ThreadProc();

	bool AddItem();
	void RemoveSelItem();
	LPARAM GetFirstSelectedItemLParam();
	int FindItemByText(LPCTSTR lpText);

private:
	static BOOL CALLBACK QueryOtherKDUpdater(HWND hWnd, LPARAM lParam);

	HWND m_hOtherKDUpdater;
	CMutex m_muxUpdate;
	CArray<CUPDTListItem> m_aDLItem;

public:
	afx_msg void OnDestroy();
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
};

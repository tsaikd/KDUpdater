#pragma once
#include "afxcmn.h"
#include "KDThread.h"
#include "KDAppVer.h"

#define DEFAULT_UPDT_LISTITEM_BOPTIONAL FALSE

class CUPDTListItem
{
public:
	CUPDTListItem() : m_uFileSize(0), m_bOptional(DEFAULT_UPDT_LISTITEM_BOPTIONAL) {}
	virtual ~CUPDTListItem() {}

	CString		m_sFilePath;
	CKDAppVer	m_sVersion;
	UINT		m_uFileSize;
	CString		m_sFileURL;
	CString		m_sSha1Hash;
	BOOL		m_bOptional;
};

class CUPDTListCtrl : public CListCtrl, public CKDThread
{
	DECLARE_MESSAGE_MAP()
public:
	CUPDTListCtrl();
	~CUPDTListCtrl();

	void	Init();
	bool	LoadSetting(LPCTSTR lpFilePath);
	void	SaveSetting(LPCTSTR lpFilePath);
	void	DoUpdate();
	bool	IsNeedUpdate(bool bPrepareDL = false);

	virtual DWORD ThreadProc();

	bool	AddItem();
	void	RemoveSelItem();
	LPARAM	GetFirstSelectedItemLParam();
	int		FindItemByText(LPCTSTR lpText);

	inline bool IsDownloadFailed() { return m_bDownloadFailed; }

	bool	m_bNeedRevert;

private:
	static BOOL CALLBACK QueryOtherKDUpdater(HWND hWnd, LPARAM lParam);

	bool	m_bDownloadFailed;
	HWND	m_hOtherKDUpdater;
	CMutex	m_muxUpdate;
	CArray<CUPDTListItem> m_aDLItem;

	// Hide information
	UINT	m_uhRegWnd;
	CString	m_sWorkDir;
	CString	m_sPostCmd;

public:
	afx_msg void OnDestroy();
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
};

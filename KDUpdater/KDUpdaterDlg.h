#pragma once
#include "UPDTListCtrl.h"
#include "UPDTEdit.h"

extern UINT WMU_KDUPDATER_REQ;
extern UINT WMU_KDUPDATER_RES;

extern UINT WMU_KDUPDATER_REQ_NEED_UPDATE;
extern UINT WMU_KDUPDATER_RES_NEED_UPDATE;
extern UINT WMU_KDUPDATER_RES_NONEED_UPDATE;

extern UINT WMU_KDUPDATER_REQ_START_UPDATE;
extern UINT WMU_KDUPDATER_REQ_CLOSE_APP;

enum {
	KDUPM_UPDATEOVER	= WM_USER + 1
};

class CKDUpdaterDlg : public CDialog
{
	DECLARE_MESSAGE_MAP()
public:
	enum { IDD = IDD_KDUPDATER_DIALOG };
	CKDUpdaterDlg(CWnd* pParent = NULL);	// standard constructor

	CUPDTListCtrl m_list_File;
	CUPDTEdit m_edit_Version;
	CUPDTEdit m_edit_FileSize;
	CUPDTEdit m_edit_SHA1Hash;
	HWND m_hRegWnd;

private:
	bool m_bVisiable;
	bool m_bArgUpdate;
	CString m_sIniPath;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	virtual void OnOK();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	HICON m_hIcon;
public:
	afx_msg void OnBnClickedUpdtBtnAddfile();
	afx_msg void OnBnClickedUpdtBtnRemove();
	afx_msg void OnBnClickedUpdtBtnImport();
	afx_msg void OnBnClickedUpdtBtnExport();
	afx_msg void OnBnClickedUpdtBtnUpdate();
	afx_msg void OnBnClickedUpdtBtnCheck();
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnDropFiles(HDROP hDropInfo);
};

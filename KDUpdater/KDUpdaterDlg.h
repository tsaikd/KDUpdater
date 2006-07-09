#pragma once
#include "UPDTListCtrl.h"

enum {
	KDUPM_UPDATEOVER	= WM_USER + 1
};

class CKDUpdaterDlg : public CDialog
{
	DECLARE_MESSAGE_MAP()
public:
	enum { IDD = IDD_KDUPDATER_DIALOG };
public:
	CKDUpdaterDlg(CWnd* pParent = NULL);	// standard constructor

	CUPDTListCtrl m_list_File;

private:
	bool m_bVisiable;
	bool m_bArgUpdate;
	CString m_sIniPath;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
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
protected:
	virtual void OnOK();
};

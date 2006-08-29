#include "stdafx.h"
#include "Ini.h"
#include "KDUpdater.h"
#include "KDUpdaterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT WMU_KDUPDATER_REQ					= RegisterWindowMessage(_T("Req") _T("KDUpdater"));
UINT WMU_KDUPDATER_RES					= RegisterWindowMessage(_T("Res") _T("KDUpdater"));

UINT WMU_KDUPDATER_REQ_NEED_UPDATE		= RegisterWindowMessage(_T("Req") _T("KDUpdater Need Update"));
UINT WMU_KDUPDATER_RES_NEED_UPDATE		= RegisterWindowMessage(_T("Res") _T("KDUpdater Need Update"));
UINT WMU_KDUPDATER_RES_NEED_REVERT		= RegisterWindowMessage(_T("Res") _T("KDUpdater Need Revert"));

UINT WMU_KDUPDATER_REQ_START_UPDATE		= RegisterWindowMessage(_T("Req") _T("KDUpdater Start Update"));
UINT WMU_KDUPDATER_REQ_CLOSE_APP		= RegisterWindowMessage(_T("Req") _T("KDUpdater Close App"));

CKDUpdaterDlg::CKDUpdaterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKDUpdaterDlg::IDD, pParent), m_bVisiable(true), m_bArgUpdate(false), m_hRegWnd(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

BOOL CKDUpdaterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_list_File.Init();

	//__argc = 2;
	//__targv[1] = _T("C:\\Documents and Settings\\tsaikd\\My Documents\\Visual Studio 2005\\Projects\\MagicKD\\trunk\\debug\\KDUpdater.exe.ini.tmp");

	// Process Command Line
	if (__argc > 1) {
		CString sBuf = __targv[1];

		if (PathFileExists(sBuf)) {
			m_bVisiable = false;
			m_bArgUpdate = true;
			Import(sBuf);
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CKDUpdaterDlg::Import(LPCTSTR lpFilePath)
{
	if (m_list_File.LoadSetting(lpFilePath)) {
		m_sIniPath = lpFilePath;
		GetDlgItem(IDC_UPDT_BTN_SAVE)->EnableWindow();
	} else {
		m_sIniPath.Empty();
		GetDlgItem(IDC_UPDT_BTN_SAVE)->EnableWindow(FALSE);
	}
}

BEGIN_MESSAGE_MAP(CKDUpdaterDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_UPDT_BTN_ADDFILE, &CKDUpdaterDlg::OnBnClickedUpdtBtnAddfile)
	ON_BN_CLICKED(IDC_UPDT_BTN_REMOVE, &CKDUpdaterDlg::OnBnClickedUpdtBtnRemove)
	ON_BN_CLICKED(IDC_UPDT_BTN_IMPORT, &CKDUpdaterDlg::OnBnClickedUpdtBtnImport)
	ON_BN_CLICKED(IDC_UPDT_BTN_EXPORT, &CKDUpdaterDlg::OnBnClickedUpdtBtnExport)
	ON_BN_CLICKED(IDC_UPDT_BTN_EXPORT, &CKDUpdaterDlg::OnBnClickedUpdtBtnSave)
	ON_BN_CLICKED(IDC_UPDT_BTN_UPDATE, &CKDUpdaterDlg::OnBnClickedUpdtBtnUpdate)
	ON_BN_CLICKED(IDC_UPDT_BTN_CHECK, &CKDUpdaterDlg::OnBnClickedUpdtBtnCheck)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

void CKDUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_UPDT_LIST_FILE, m_list_File);
	DDX_Control(pDX, IDC_UPDT_EDIT_VERSION, m_edit_Version);
	DDX_Control(pDX, IDC_UPDT_EDIT_FILESIZE, m_edit_FileSize);
	DDX_Control(pDX, IDC_UPDT_EDIT_SHA1HASH, m_edit_SHA1Hash);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CKDUpdaterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CKDUpdaterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CKDUpdaterDlg::OnBnClickedUpdtBtnAddfile()
{
	m_list_File.AddItem();
	GetDlgItem(IDC_UPDT_EDIT_FILEPATH)->SetFocus();
}

void CKDUpdaterDlg::OnBnClickedUpdtBtnRemove()
{
	m_list_File.RemoveSelItem();
	GetDlgItem(IDC_UPDT_EDIT_FILEPATH)->SetFocus();
}

void CKDUpdaterDlg::OnBnClickedUpdtBtnImport()
{
	CFileDialog fDlg(TRUE, _T("ini"), _T("Update"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Ini Files (*.ini)|*.ini|All Files (*.*)|*.*||"), this);
	if (IDOK == fDlg.DoModal())
		Import(fDlg.GetPathName());
}

void CKDUpdaterDlg::OnBnClickedUpdtBtnExport()
{
	if (!m_list_File.GetItemCount())
		return;

	CFileDialog fDlg(FALSE, _T("ini"), _T("Update"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Ini Files (*.ini)|*.ini|All Files (*.*)|*.*||"), this);
	if (IDOK == fDlg.DoModal())
		m_list_File.SaveSetting(fDlg.GetPathName());
}

void CKDUpdaterDlg::OnBnClickedUpdtBtnSave()
{
	if (!m_list_File.GetItemCount())
		return;
	if (m_sIniPath.IsEmpty())
		return;

	m_list_File.SaveSetting(m_sIniPath);
}

void CKDUpdaterDlg::OnBnClickedUpdtBtnUpdate()
{
	m_list_File.DoUpdate();
}

void CKDUpdaterDlg::OnBnClickedUpdtBtnCheck()
{
	if (m_list_File.IsNeedUpdate())
		MessageBox(_T("Need to Update"));
	else if (!m_list_File.IsDownloadFailed())
		MessageBox(_T("No Need to Update"));
}

void CKDUpdaterDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CDialog::OnWindowPosChanging(lpwndpos);

	if (!m_bVisiable)
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
}

void CKDUpdaterDlg::OnOK()
{
	int nID = GetFocus()->GetDlgCtrlID();
	switch (nID) {
	case IDC_UPDT_EDIT_LISTURL:
		GetDlgItem(IDC_UPDT_EDIT_FILEPATH)->SetFocus();
		break;
	case IDC_UPDT_EDIT_FILEPATH:
		GetDlgItem(IDC_UPDT_EDIT_VERSION)->SetFocus();
		break;
	case IDC_UPDT_EDIT_VERSION:
		GetDlgItem(IDC_UPDT_EDIT_FILESIZE)->SetFocus();
		break;
	case IDC_UPDT_EDIT_FILESIZE:
		GetDlgItem(IDC_UPDT_EDIT_FILEURL)->SetFocus();
		break;
	case IDC_UPDT_EDIT_FILEURL:
		GetDlgItem(IDC_UPDT_BTN_ADDFILE)->SetFocus();
		break;
	}

//	CDialog::OnOK();
}

void CKDUpdaterDlg::OnDropFiles(HDROP hDropInfo)
{
	TCHAR sFilePath[MAX_PATH];
	UINT i, uCount = DragQueryFile(hDropInfo, UINT_MAX, NULL, 0);

	for (i = 0;i< uCount;i++) {
		DragQueryFile(hDropInfo, i, sFilePath, MAX_PATH);

		if (PathFileExists(sFilePath)) {
			Import(sFilePath);
			break;
		}
	}

	CDialog::OnDropFiles(hDropInfo);
}

LRESULT CKDUpdaterDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WMU_KDUPDATER_REQ) {
		if (m_hRegWnd && ((HWND)wParam != m_hRegWnd))
			return 0;

		return WMU_KDUPDATER_RES;
	} else if (message == WMU_KDUPDATER_REQ_NEED_UPDATE) {
		if (m_list_File.IsNeedUpdate()) {
			if (m_list_File.m_bNeedRevert)
				return WMU_KDUPDATER_RES_NEED_REVERT;
			else
				return WMU_KDUPDATER_RES_NEED_UPDATE;
		} else {
			return 0;
		}
	} else if (message == WMU_KDUPDATER_REQ_START_UPDATE) {
		int iMaxWaitTimes = 60;
		while (IsWindow((HWND)lParam) && iMaxWaitTimes--)
			Sleep(1000);
		Sleep(wParam);
		m_list_File.DoUpdate();
		return 0;
	} else if (message == WMU_KDUPDATER_REQ_CLOSE_APP) {
		theApp.Quit();
		return 0;
	}

	switch (message) {
	case KDUPM_UPDATEOVER:
		if (m_bArgUpdate) {
			CString sPostCmd;
			CIni ini;
			ini.SetPathName(m_sIniPath);
			sPostCmd = ini.GetString(_T("General"), _T("sPostCmd"));
			if (!sPostCmd.IsEmpty())
				ShellExecute(NULL, _T("open"), sPostCmd, NULL, NULL, SW_SHOW);

			theApp.Quit();
		}
		break;
	}

	return CDialog::DefWindowProc(message, wParam, lParam);
}

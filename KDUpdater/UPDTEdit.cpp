#include "stdafx.h"
#include "SHA.h"
#include "Others.h"
#include "KDUpdater.h"
#include "UPDTEdit.h"

IMPLEMENT_DYNAMIC(CUPDTEdit, CEdit)

CUPDTEdit::CUPDTEdit()
{
}

CUPDTEdit::~CUPDTEdit()
{
}

BEGIN_MESSAGE_MAP(CUPDTEdit, CEdit)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

void CUPDTEdit::OnDropFiles(HDROP hDropInfo)
{
	TCHAR sFilePath[MAX_PATH];
	UINT i, uCount = DragQueryFile(hDropInfo, UINT_MAX, NULL, 0);

	for (i = 0;i< uCount;i++) {
		DragQueryFile(hDropInfo, i, sFilePath, MAX_PATH);

		if (PathFileExists(sFilePath)) {
			// Set SHA1 hash value
			CSHA sha;
			CString sBuf = sha.GetHashStringFromFile(sFilePath, TRUE);
			GetParent()->GetDlgItem(IDC_UPDT_EDIT_SHA1HASH)->SetWindowText(sBuf);

			// Set File Version
			GetParent()->GetDlgItem(IDC_UPDT_EDIT_VERSION)->SetWindowText(CGetFileVersion(sFilePath));

			// Set File Size
			CFile file;
			if (file.Open(sFilePath, CFile::modeRead | CFile::shareDenyWrite)) {
				sBuf.Format(_T("%d"), file.GetLength());
				GetParent()->GetDlgItem(IDC_UPDT_EDIT_FILESIZE)->SetWindowText(sBuf);
				file.Close();
			}
			break;
		}
	}

	CEdit::OnDropFiles(hDropInfo);
}

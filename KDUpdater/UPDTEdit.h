#pragma once

class CUPDTEdit : public CEdit
{
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CUPDTEdit)

public:
	CUPDTEdit();
	virtual ~CUPDTEdit();

public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
};



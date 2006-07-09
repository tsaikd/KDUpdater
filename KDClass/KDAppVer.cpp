#include "StdAfx.h"
#include "KDAppVer.h"

CKDAppVer::CKDAppVer()
{
}

CKDAppVer::~CKDAppVer()
{
}

bool CKDAppVer::operator > (const CKDAppVer& ver)
{
	CString sVer1(GetString());
	CString sVer2(ver);
	CString sBuf1, sBuf2;
	int iBuf1 = 0, iBuf2 = 0;
	int iPos1 = 0, iPos2 = 0;

	sVer1.Replace(_T(','), _T('.'));
	sVer2.Replace(_T(','), _T('.'));

	while (1) {
		sBuf1 = sVer1.Tokenize(_T("."), iPos1);
		sBuf2 = sVer2.Tokenize(_T("."), iPos2);
		if (sBuf1.IsEmpty() || sBuf2.IsEmpty())
			break;

		iBuf1 = _ttoi(sBuf1);
		iBuf2 = _ttoi(sBuf2);

		if (iBuf1 > iBuf2)
			return true;
		if (iBuf1 < iBuf2)
			return false;
	}

	if (!sBuf1.IsEmpty())
		return true;
	if (!sBuf2.IsEmpty())
		return false;

	return false;
}
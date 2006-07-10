#include "StdAfx.h"
#include "KDAppVer.h"

bool CKDAppVer::operator > (const CKDAppVer& ver)
{
	if (_CmpVer(GetString(), ver) > 0)
		return true;

	return false;
}

bool CKDAppVer::operator >= (const CKDAppVer& ver)
{
	if (_CmpVer(GetString(), ver) >= 0)
		return true;

	return false;
}

bool CKDAppVer::operator == (const CKDAppVer& ver)
{
	if (_CmpVer(GetString(), ver) == 0)
		return true;

	return false;
}

bool CKDAppVer::operator != (const CKDAppVer& ver)
{
	if (_CmpVer(GetString(), ver) != 0)
		return true;

	return false;
}

bool CKDAppVer::operator < (const CKDAppVer& ver)
{
	if (_CmpVer(GetString(), ver) < 0)
		return true;

	return false;
}

bool CKDAppVer::operator <= (const CKDAppVer& ver)
{
	if (_CmpVer(GetString(), ver) <= 0)
		return true;

	return false;
}

int CKDAppVer::_CmpVer(LPCTSTR lpVer1, LPCTSTR lpVer2)
{
	CString sVer1(lpVer1);
	CString sVer2(lpVer2);
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

		if (sBuf1.GetAt(0) >= _T('a')) {
			iBuf1 = sBuf1.GetAt(0) - _T('a') + 10;
		} else if (sBuf1.GetAt(0) >= _T('A')) {
			iBuf1 = sBuf1.GetAt(0) - _T('A') + 10;
		} else {
			iBuf1 = _ttoi(sBuf1);
		}
		if (sBuf2.GetAt(0) >= _T('a')) {
			iBuf2 = sBuf2.GetAt(0) - _T('a') + 10;
		} else if (sBuf2.GetAt(0) >= _T('A')) {
			iBuf2 = sBuf2.GetAt(0) - _T('A') + 10;
		} else {
			iBuf2 = _ttoi(sBuf2);
		}

		if (iBuf1 > iBuf2)
			return 1;
		if (iBuf1 < iBuf2)
			return -1;
	}

	if (!sBuf1.IsEmpty())
		return 1;
	if (!sBuf2.IsEmpty())
		return -1;

	return 0;
}

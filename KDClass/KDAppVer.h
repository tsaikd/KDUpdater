#pragma once

class CKDAppVer : public CString
{
public:
	inline CKDAppVer() {}
	inline CKDAppVer(LPCTSTR lpStr) { SetString(lpStr); }

	using CString::operator =;
	bool operator > (const CKDAppVer& ver);
	bool operator >= (const CKDAppVer& ver);
	bool operator == (const CKDAppVer& ver);
	bool operator != (const CKDAppVer& ver);
	bool operator < (const CKDAppVer& ver);
	bool operator <= (const CKDAppVer& ver);

private:
	int _CmpVer(LPCTSTR lpVer1, LPCTSTR lpVer2);
};

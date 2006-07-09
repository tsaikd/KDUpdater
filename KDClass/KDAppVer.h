#pragma once

class CKDAppVer : public CString
{
public:
	CKDAppVer();
	virtual ~CKDAppVer();

	using CString::operator =;
	bool operator > (const CKDAppVer& ver);
};

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "../KDClass/KDApp.h"

class CKDUpdaterApp : public CWinApp, public CKDApp
{
	DECLARE_MESSAGE_MAP()
public:
	CKDUpdaterApp();

	void Quit();

public:
	virtual BOOL InitInstance();
};

extern CKDUpdaterApp theApp;
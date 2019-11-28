
// Win2DinMFC.h : main header file for the Win2DinMFC application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CWin2DinMFCApp:
// See Win2DinMFC.cpp for the implementation of this class
//

class CWin2DinMFCApp : public CWinAppEx
{
public:
	CWin2DinMFCApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CWin2DinMFCApp theApp;

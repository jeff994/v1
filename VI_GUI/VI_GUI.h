
// VI_GUI.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CVIGUIApp:
// See VI_GUI.cpp for the implementation of this class
//

class CVIGUIApp : public CWinApp
{
public:
	CVIGUIApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CVIGUIApp theApp;

// OCF.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// C_OCFApp:
// See OCF.cpp for the implementation of this class
//

class C_OCFApp : public CWinApp
{
public:
	C_OCFApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern C_OCFApp theApp;
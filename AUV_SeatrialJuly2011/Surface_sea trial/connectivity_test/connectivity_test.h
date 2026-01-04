// connectivity_test.h : main header file for the CONNECTIVITY_TEST application
//

#if !defined(AFX_CONNECTIVITY_TEST_H__2CC28052_9844_4FC7_A8DF_737CC9ABC077__INCLUDED_)
#define AFX_CONNECTIVITY_TEST_H__2CC28052_9844_4FC7_A8DF_737CC9ABC077__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CConnectivity_testApp:
// See connectivity_test.cpp for the implementation of this class
//

class CConnectivity_testApp : public CWinApp
{
public:
	CConnectivity_testApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConnectivity_testApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CConnectivity_testApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTIVITY_TEST_H__2CC28052_9844_4FC7_A8DF_737CC9ABC077__INCLUDED_)

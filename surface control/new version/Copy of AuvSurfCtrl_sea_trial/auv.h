// auv.h : main header file for the AUV application
//

#if !defined(AFX_AUV_H__6E419EDF_09C3_4C86_8497_2771F84A953F__INCLUDED_)
#define AFX_AUV_H__6E419EDF_09C3_4C86_8497_2771F84A953F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAuvApp:
// See auv.cpp for the implementation of this class
//

class CAuvApp : public CWinApp
{
public:
	CAuvApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAuvApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAuvApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUV_H__6E419EDF_09C3_4C86_8497_2771F84A953F__INCLUDED_)

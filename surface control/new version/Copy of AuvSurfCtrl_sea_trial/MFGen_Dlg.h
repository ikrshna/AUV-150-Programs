#if !defined(AFX_MFGEN_DLG_H__A47D4435_A330_440E_960B_76532F07365C__INCLUDED_)
#define AFX_MFGEN_DLG_H__A47D4435_A330_440E_960B_76532F07365C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MFGen_Dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMFGen_Dlg dialog

class CMFGen_Dlg : public CDialog
{
// Construction
public:
	CMFGen_Dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMFGen_Dlg)
	enum { IDD = IDD_GENMF_DLG };
	CListBox	m_gen_mf;
	CString	m_mf_head;
	CString	m_mf_surge;
	CString	m_mf_time;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFGen_Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMFGen_Dlg)
	virtual void OnOK();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonDownload();
	afx_msg void OnSelchangeListMfgen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFGEN_DLG_H__A47D4435_A330_440E_960B_76532F07365C__INCLUDED_)

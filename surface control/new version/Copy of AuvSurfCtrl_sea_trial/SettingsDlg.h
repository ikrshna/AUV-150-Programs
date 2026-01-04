#if !defined(AFX_SETTINGSDLG_H__CE2CF0DF_0C1D_44E6_83C5_59B4B3A9EFA4__INCLUDED_)
#define AFX_SETTINGSDLG_H__CE2CF0DF_0C1D_44E6_83C5_59B4B3A9EFA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SettingsDlg dialog

class SettingsDlg : public CDialog
{
// Construction
public:
	SettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SettingsDlg)
	enum { IDD = IDD_SETTINGS };
	int		m_disp_depth;
	int		m_disp_alt;
	int		m_disp_surge;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SettingsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__CE2CF0DF_0C1D_44E6_83C5_59B4B3A9EFA4__INCLUDED_)

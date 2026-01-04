// connectivity_testDlg.h : header file
//

#if !defined(AFX_CONNECTIVITY_TESTDLG_H__125EF6D5_E72B_45E0_BB41_EABB53D8342E__INCLUDED_)
#define AFX_CONNECTIVITY_TESTDLG_H__125EF6D5_E72B_45E0_BB41_EABB53D8342E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CConnectivity_testDlg dialog

class CConnectivity_testDlg : public CDialog
{
// Construction
public:
	CConnectivity_testDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CConnectivity_testDlg)
	enum { IDD = IDD_CONNECTIVITY_TEST_DIALOG };
	CComboBox	m_ct2;
	CComboBox	m_mt;
	CComboBox	m_ct;
	CListBox	m_gen_mf;
	CStatic	m_static_conn;
	CButton	m_button_conn;
	CListBox	m_display;
	CComboBox m_combo_sensor;
	CButton	m_conn;
	CString	m_mf_head;
	CString	m_mf_surge;
	CString	m_mf_time;
	double	m_easting;
	double	m_northing;
	CString	m_zone;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConnectivity_testDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

private:
	CFont m_font;

	// Generated message map functions
	//{{AFX_MSG(CConnectivity_testDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnButtonAltimeter();
	afx_msg void OnButtonDepth();
	afx_msg void OnButtonPhins();
	afx_msg void OnButtonDvl();
	afx_msg void OnButtonGps();
	afx_msg void OnButtonFls();
	afx_msg void OnButtonSss();
	afx_msg void OnButtonConn();
	afx_msg void OnButtonShutdown();
	afx_msg void OnClose();
	afx_msg void OnButtonMan();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonSave();
	afx_msg void OnSelchangeListMfgen();
	afx_msg void OnButtonRelay();
	afx_msg void OnButtonLaunch();
	afx_msg void OnButtonOff();
	afx_msg void OnButtonUtm();
	afx_msg void OnButtonThread();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTIVITY_TESTDLG_H__125EF6D5_E72B_45E0_BB41_EABB53D8342E__INCLUDED_)

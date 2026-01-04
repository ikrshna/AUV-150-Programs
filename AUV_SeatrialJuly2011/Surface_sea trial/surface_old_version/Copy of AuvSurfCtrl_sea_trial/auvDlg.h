// auvDlg.h : header file
//

#include "colorBtn.h"

#if !defined(AFX_AUVDLG_H__4489CAD2_9AA6_4FC5_89EC_86D63EF7A02C__INCLUDED_)
#define AFX_AUVDLG_H__4489CAD2_9AA6_4FC5_89EC_86D63EF7A02C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAuvDlg dialog

class CAuvDlg : public CDialog
{
// Construction
public:
	
	CAuvDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAuvDlg)
	enum { IDD = IDD_AUV_DIALOG };
	CComboBox	m_refdepth;
	CComboBox	m_missdur;
	CListBox	m_genmess;
	CListBox	m_acoustic;
	CComboBox	m_combo;
	BOOL	m_rf_connected;
	BOOL	m_rf_disconnected;
	CString	m_pitch;
	CString	m_roll;
	CString	m_yaw;
	CString	m_surge;
	CString m_a_alti;
	CString m_d_alti;
	CString m_lat;
	CString m_long;
	CString m_east;
	CString m_north;
	CString m_elapsed;
	CString m_remain;
	CComboBox m_ct;
	CComboBox m_mt;
	CString m_mf_head;
	CString m_mf_surge;
	CString m_mf_time;
	CListBox m_gen_mf;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAuvDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	
	CColorButton b1_safe;
	CColorButton b1_alm;
	CColorButton b1_crt;
	CColorButton b2_safe;
	CColorButton b2_alm;
	CColorButton b2_crt;
	CColorButton b3_safe;
	CColorButton b3_alm;
	CColorButton b3_crt;
	CColorButton b4_safe;
	CColorButton b4_alm;
	CColorButton b4_crt;
	CColorButton b5_safe;
	CColorButton b5_alm;
	CColorButton b5_crt;
	CColorButton b6_safe;
	CColorButton b6_alm;
	CColorButton b6_crt;
	CColorButton btn_leak;
	CColorButton btn_rf;
	// Generated message map functions
	//{{AFX_MSG(CAuvDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnOK();
	afx_msg void OnButtonSubmit();
	afx_msg void OnButtonAuto();
	afx_msg void OnButtonMan();
	afx_msg void OnClose();
	afx_msg void OnButtonLight();
	afx_msg void OnButtonStop();
	afx_msg void OnButtonMFGen();
	

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUVDLG_H__4489CAD2_9AA6_4FC5_89EC_86D63EF7A02C__INCLUDED_)

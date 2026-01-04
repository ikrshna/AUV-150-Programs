// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "auv.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SettingsDlg dialog


SettingsDlg::SettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(SettingsDlg)
	m_disp_depth = 0;
	m_disp_alt = 0;
	m_disp_surge = 0;
	//}}AFX_DATA_INIT
}


void SettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SettingsDlg)
	DDX_Text(pDX, IDC_EDIT1, m_disp_depth);
	DDX_Text(pDX, IDC_EDIT2, m_disp_alt);
	DDX_Text(pDX, IDC_EDIT3, m_disp_surge);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SettingsDlg, CDialog)
	//{{AFX_MSG_MAP(SettingsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SettingsDlg message handlers

// MFGen_Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "auv.h"
#include "MFGen_Dlg.h"
#include "stdio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFGen_Dlg dialog

int index;

CMFGen_Dlg::CMFGen_Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMFGen_Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMFGen_Dlg)
	m_mf_head = _T("");
	m_mf_surge = _T("");
	m_mf_time = _T("");
	//}}AFX_DATA_INIT
}


void CMFGen_Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFGen_Dlg)
	DDX_Control(pDX, IDC_LIST_MFGEN, m_gen_mf);
	DDX_Text(pDX, IDC_EDIT_HEAD, m_mf_head);
	DDX_Text(pDX, IDC_EDIT_SURGE, m_mf_surge);
	DDX_Text(pDX, IDC_EDIT_TIME, m_mf_time);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMFGen_Dlg, CDialog)
	//{{AFX_MSG_MAP(CMFGen_Dlg)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD, OnButtonDownload)
	ON_LBN_SELCHANGE(IDC_LIST_MFGEN, OnSelchangeListMfgen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFGen_Dlg message handlers

void CMFGen_Dlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CMFGen_Dlg::OnButtonAdd() 
{
	// TODO: Add your control notification handler code here
	char str[15];
	UpdateData(true);
	sprintf(str,"%s %s %s",m_mf_head,m_mf_surge,m_mf_time);
	m_gen_mf.AddString(str);
}

void CMFGen_Dlg::OnButtonEdit() 
{
	// TODO: Add your control notification handler code here
	char str[15];
	UpdateData(true);
	sprintf(str,"%s %s %s",m_mf_head,m_mf_surge,m_mf_time);
	m_gen_mf.DeleteString(index);
	m_gen_mf.InsertString(index,str);
	index=9999;
	
}

void CMFGen_Dlg::OnButtonDelete() 
{
	// TODO: Add your control notification handler code here
	m_gen_mf.DeleteString(index);
	index=9999;

}

void CMFGen_Dlg::OnButtonDownload() 
{
	// TODO: Add your control notification handler code here
	int count;
	int i;
	char str[15];
	FILE *mf_fp;

	count=m_gen_mf.GetCount();
	mf_fp=fopen("miss_file.txt","a+");
	
	for(i=0;i<count;i++){
		m_gen_mf.GetText(i,str);
		if(i!=(count-1))
			fprintf(mf_fp,"%s\n",str);
		else
			fprintf(mf_fp,"%s",str);
	}

	fclose(mf_fp);
}

void CMFGen_Dlg::OnSelchangeListMfgen() 
{
	// TODO: Add your control notification handler code here
	index=m_gen_mf.GetCurSel();
}

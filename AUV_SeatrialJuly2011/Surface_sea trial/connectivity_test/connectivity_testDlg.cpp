// connectivity_testDlg.cpp : implementation file
//

#include "stdafx.h"
#include "connectivity_test.h"
#include "connectivity_testDlg.h"

#include <winsock2.h>	
#pragma comment (lib,"ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


SOCKET hSock;
sockaddr_in sa;

char recvBuffer[2600];
//char disp_buffer[2600];

int index;
char command[22];
bool suspend_man_thread=false;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectivity_testDlg dialog

CConnectivity_testDlg::CConnectivity_testDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectivity_testDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConnectivity_testDlg)
	m_mf_head = _T("");
	m_mf_surge = _T("");
	m_mf_time = _T("");
	m_easting = 0.0;
	m_northing = 0.0;
	m_zone = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CConnectivity_testDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectivity_testDlg)
	DDX_Control(pDX, IDC_COMBO_CT2, m_ct2);
	DDX_Control(pDX, IDC_COMBO_MT, m_mt);
	DDX_Control(pDX, IDC_COMBO_CT, m_ct);
	DDX_Control(pDX, IDC_LIST_MFGEN, m_gen_mf);
	DDX_Control(pDX, IDC_STATIC_CONN, m_static_conn);
	DDX_Control(pDX, IDC_BUTTON_CONN, m_button_conn);
	DDX_Control(pDX, IDC_LIST_STATUS, m_display);
	DDX_Text(pDX, IDC_EDIT_HEAD, m_mf_head);
	DDX_Text(pDX, IDC_EDIT_SURGE, m_mf_surge);
	DDX_Text(pDX, IDC_EDIT_TIME, m_mf_time);
	DDX_Text(pDX, IDC_EDIT_EASTING, m_easting);
	DDX_Text(pDX, IDC_EDIT_NORTHING, m_northing);
	DDX_Text(pDX, IDC_EDIT_ZONE, m_zone);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConnectivity_testDlg, CDialog)
	//{{AFX_MSG_MAP(CConnectivity_testDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_ALTIMETER, OnButtonAltimeter)
	ON_BN_CLICKED(IDC_BUTTON_DEPTH, OnButtonDepth)
	ON_BN_CLICKED(IDC_BUTTON_PHINS, OnButtonPhins)
	ON_BN_CLICKED(IDC_BUTTON_DVL, OnButtonDvl)
	ON_BN_CLICKED(IDC_BUTTON_GPS, OnButtonGps)
	ON_BN_CLICKED(IDC_BUTTON_FLS, OnButtonFls)
	ON_BN_CLICKED(IDC_BUTTON_SSS, OnButtonSss)
	ON_BN_CLICKED(IDC_BUTTON_CONN, OnButtonConn)
	ON_BN_CLICKED(IDC_BUTTON_SHUTDOWN, OnButtonShutdown)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_MAN, OnButtonMan)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_LBN_SELCHANGE(IDC_LIST_MFGEN, OnSelchangeListMfgen)
	ON_BN_CLICKED(IDC_BUTTON_RELAY, OnButtonRelay)
	ON_BN_CLICKED(IDC_BUTTON_LAUNCH, OnButtonLaunch)
	ON_BN_CLICKED(IDC_BUTTON_OFF, OnButtonOff)
	ON_BN_CLICKED(IDC_BUTTON_UTM, OnButtonUtm)
	ON_BN_CLICKED(IDC_BUTTON_THREAD, OnButtonThread)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectivity_testDlg message handlers
bool connected;
bool relay_on;

BOOL CConnectivity_testDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_ct.SetCurSel(3);
	m_ct2.SetCurSel(3);
	m_mt.SetCurSel(4);
	connected=false;
	relay_on=false;
	
	GetDlgItem(IDC_STATIC_CONN)->SetWindowText("Not Connected");
	GetDlgItem(IDC_STATIC_ALTIMETER)->SetWindowText("Not tested yet");
	GetDlgItem(IDC_STATIC_DEPTH)->SetWindowText("Not tested yet");
	GetDlgItem(IDC_STATIC_PHINS)->SetWindowText("Not tested yet");
	GetDlgItem(IDC_STATIC_DVL)->SetWindowText("Not tested yet");
	GetDlgItem(IDC_STATIC_GPS)->SetWindowText("Not tested yet");
	GetDlgItem(IDC_STATIC_FLS)->SetWindowText("Not tested yet");
	GetDlgItem(IDC_STATIC_SSS)->SetWindowText("Not tested yet");
	GetDlgItem(IDC_BUTTON_RELAY)->SetWindowText("Switch Relay ON");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CConnectivity_testDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CConnectivity_testDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CConnectivity_testDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}






void CConnectivity_testDlg::OnOK() 
{
	// TODO: Add extra validation here
	OnClose();
	CDialog::OnOK ();
}




void CConnectivity_testDlg::OnButtonConn() 
{
	// TODO: Add your control notification handler code here
	
	
	if(!connected){
		
		//=================================================
		//-------Initialize winsock2.2 dll ----------//
		WSADATA wsaData={0};
		WORD wVersionRequested=MAKEWORD(2,2);
		int nRet=WSAStartup(wVersionRequested,&wsaData);
		if (nRet==SOCKET_ERROR){
			GetDlgItem(IDC_STATIC_CONN)->SetWindowText("Not Connected");
		}

		//--------Open a socket-------------------------------------------------------
		hSock=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
		if (nRet==INVALID_SOCKET){
			GetDlgItem(IDC_STATIC_CONN)->SetWindowText("Invalid Socket");
		}

		//---------Name the socket--------------------------------------------------
		sa.sin_family = PF_INET;
		sa.sin_port=htons(8000);
		sa.sin_addr.s_addr=inet_addr("192.168.1.10");

		
		//-----------------connect---------------------------------------------------------------
		
		nRet = connect(hSock,(sockaddr*)&sa,sizeof(sockaddr)); 

		if( nRet == SOCKET_ERROR ) {
			 GetDlgItem(IDC_STATIC_CONN)->SetWindowText("Not Connected");
			 
		}
		else{
			connected=true;
			GetDlgItem(IDC_STATIC_CONN)->SetWindowText("Connected");
			this->SetDlgItemText (IDC_BUTTON_CONN,"Disconnect");
		}
		
			
	}
	else{
		this->SetDlgItemText (IDC_BUTTON_CONN,"&Connect");
		GetDlgItem(IDC_STATIC_CONN)->SetWindowText("Disconnected");
		send(hSock,"#",1,0);
		//close socket
		closesocket(hSock);						
		hSock=0;
		WSACleanup();	//Release Winsock dll
		connected=false;
		//m_display.AddString(disp_buffer);
		//m_static_conn.SetDlgItemText(IDC_STATIC_CONN,"Failure");
	}
}

void CConnectivity_testDlg::OnButtonAltimeter() 
{
	// TODO: Add your control notification handler code here
	char buffer[1600];
	char header;
	send(hSock,"1",1,0);
	int inDataLength=recv(hSock,recvBuffer,sizeof(recvBuffer),0);
	recvBuffer[inDataLength]='\0';
	switch(recvBuffer[0])
	{
	case 's'	:	sscanf(recvBuffer,"%c %s",&header,buffer);
					GetDlgItem(IDC_STATIC_ALTIMETER)->SetWindowText("Successful");
					m_display.AddString(buffer);
					break;
	case 'f'	:	GetDlgItem(IDC_STATIC_ALTIMETER)->SetWindowText("Failed");
					break;
	case 'n'	:	GetDlgItem(IDC_STATIC_ALTIMETER)->SetWindowText("Failed to open the port");
					break;
	}
	
}

void CConnectivity_testDlg::OnButtonDepth() 
{
	// TODO: Add your control notification handler code here
	char buffer[1600];
	char header;
	send(hSock,"2",1,0);
	int inDataLength=recv(hSock,recvBuffer,sizeof(recvBuffer),0);
	recvBuffer[inDataLength]='\0';
	switch(recvBuffer[0])
	{
	case 's'	:	sscanf(recvBuffer,"%c %s",&header,buffer);
					GetDlgItem(IDC_STATIC_DEPTH)->SetWindowText("Successful");
					m_display.AddString(buffer);
					break;
	case 'f'	:	GetDlgItem(IDC_STATIC_DEPTH)->SetWindowText("Failed");
					break;
	case 'n'	:	GetDlgItem(IDC_STATIC_DEPTH)->SetWindowText("Failed to open the port");
					break;
	}
	
}

void CConnectivity_testDlg::OnButtonPhins() 
{
	// TODO: Add your control notification handler code here
	char buffer[2500];
	char header;
	send(hSock,"3",1,0);
	int inDataLength=recv(hSock,recvBuffer,sizeof(recvBuffer),0);
	recvBuffer[inDataLength]='\0';
	switch(recvBuffer[0])
	{
	case 's'	:	sscanf(recvBuffer,"%c %s",&header,buffer);
					GetDlgItem(IDC_STATIC_PHINS)->SetWindowText("Successful");
					m_display.AddString(buffer);
					break;
	case 'f'	:	GetDlgItem(IDC_STATIC_PHINS)->SetWindowText("Failed");
					break;
	case 'n'	:	GetDlgItem(IDC_STATIC_PHINS)->SetWindowText("Failed to open the port");
					break;
	}
	
}

void CConnectivity_testDlg::OnButtonDvl() 
{
	// TODO: Add your control notification handler code here
	char buffer[1600];
	char header;
	send(hSock,"4",1,0);
	int inDataLength=recv(hSock,recvBuffer,sizeof(recvBuffer),0);
	recvBuffer[inDataLength]='\0';
	switch(recvBuffer[0])
	{
	case 's'	:	sscanf(recvBuffer,"%c %s",&header,buffer);
					GetDlgItem(IDC_STATIC_DVL)->SetWindowText("Successful");
					m_display.AddString(buffer);
					break;
	case 'f'	:	GetDlgItem(IDC_STATIC_DVL)->SetWindowText("Failed");
					break;
	case 'n'	:	GetDlgItem(IDC_STATIC_DVL)->SetWindowText("Failed to open the port");
					break;
	}
}

void CConnectivity_testDlg::OnButtonGps() 
{
	// TODO: Add your control notification handler code here
	char buffer[1600];
	char header;
	send(hSock,"5",1,0);
	int inDataLength=recv(hSock,recvBuffer,sizeof(recvBuffer),0);
	recvBuffer[inDataLength]='\0';
	switch(recvBuffer[0])
	{
	case 's'	:	sscanf(recvBuffer,"%c %s",&header,buffer);
					GetDlgItem(IDC_STATIC_GPS)->SetWindowText("Successful");
					m_display.AddString(buffer);
					break;
	case 'f'	:	GetDlgItem(IDC_STATIC_GPS)->SetWindowText("Failed");
					break;
	case 'n'	:	GetDlgItem(IDC_STATIC_GPS)->SetWindowText("Failed to open the port");
					break;
	}
}

void CConnectivity_testDlg::OnButtonFls() 
{
	// TODO: Add your control notification handler code here
	
	send(hSock,"6",1,0);
}

void CConnectivity_testDlg::OnButtonSss() 
{
	// TODO: Add your control notification handler code here
	
	send(hSock,"7",1,0);
}

void CConnectivity_testDlg::OnButtonShutdown() 
{
	// TODO: Add your control notification handler code here
	send(hSock,"t",1,0);
	GetDlgItem(IDC_STATIC_CONN)->SetWindowText("Not Connected");
	GetDlgItem(IDC_BUTTON_CONN)->SetWindowText("&Connect");
	connected=false;
	//SetDlgItemText (IDC_BUTTON_CONN,"&Connect");
}


void CConnectivity_testDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	send(hSock,"#",1,0);	//tell server to close the client connection
	closesocket(hSock);		//close socket
	hSock=0;
	WSACleanup();			//Release Winsock dll
	CDialog::OnClose();
}

void CConnectivity_testDlg::OnButtonMan() 
{
	// TODO: Add your control notification handler code here
	int idx_ct=m_ct.GetCurSel();
	CString str_ct;
	m_ct.GetLBText(idx_ct,str_ct);

	int idx_ct2=m_ct2.GetCurSel();
	CString str_ct2;
	m_ct2.GetLBText(idx_ct2,str_ct2);

	int idx_mt=m_mt.GetCurSel();
	CString str_mt;
	m_mt.GetLBText(idx_mt,str_mt);

	int bytes=sprintf(command,"%c %s %s %s",'A',str_ct,str_ct2,str_mt);
	command[bytes+1]='\0';
	send(hSock,command,strlen(command),0);
}

void CConnectivity_testDlg::OnButtonAdd() 
{
	// TODO: Add your control notification handler code here
	char str[15];
	UpdateData(true);
	sprintf(str,"%s %s %s",m_mf_head,m_mf_surge,m_mf_time);
	m_gen_mf.AddString(str);

}

void CConnectivity_testDlg::OnButtonEdit() 
{
	// TODO: Add your control notification handler code here
	char str[15];
	UpdateData(true);
	sprintf(str,"%s %s %s",m_mf_head,m_mf_surge,m_mf_time);
	m_gen_mf.DeleteString(index);
	m_gen_mf.InsertString(index,str);
	index=9999;
	
}

void CConnectivity_testDlg::OnButtonDelete() 
{
	// TODO: Add your control notification handler code here
	m_gen_mf.DeleteString(index);
	index=9999;
	
}

void CConnectivity_testDlg::OnButtonSave() 
{
	// TODO: Add your control notification handler code here
	int count;
	int i;
	char str[15];
	FILE *mf_fp;

	count=m_gen_mf.GetCount();
	mf_fp=fopen("mission_file.txt","a+");
	
	for(i=0;i<count;i++){
		m_gen_mf.GetText(i,str);
		if(i!=(count-1))
			fprintf(mf_fp,"%s\n",str);
		else
			fprintf(mf_fp,"%s",str);
	}
	fclose(mf_fp);
}


void CConnectivity_testDlg::OnSelchangeListMfgen() 
{
	// TODO: Add your control notification handler code here
	index=m_gen_mf.GetCurSel();
}

void CConnectivity_testDlg::OnButtonRelay() 
{
	// TODO: Add your control notification handler code here
	if(!relay_on){
		send(hSock,"8",1,0);
		relay_on=true;
		GetDlgItem(IDC_BUTTON_RELAY)->SetWindowText("Switch Relay OFF");
	}
	else{
		send(hSock,"9",1,0);
		relay_on=false;
		GetDlgItem(IDC_BUTTON_RELAY)->SetWindowText("Switch Relay ON");
	}
}

void CConnectivity_testDlg::OnButtonLaunch() 
{
	// TODO: Add your control notification handler code here
	send(hSock,"B",1,0);
}

void CConnectivity_testDlg::OnButtonOff() 
{
	// TODO: Add your control notification handler code here
	send(hSock,"C",1,0);
	
}

void CConnectivity_testDlg::OnButtonUtm() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	char utm_buffer[200];
	sprintf(utm_buffer,"%c %s %lf %lf",'D',m_zone,m_northing,m_easting);
	send(hSock,utm_buffer,strlen(utm_buffer),0);
}


void CConnectivity_testDlg::OnButtonThread() 
{
	// TODO: Add your control notification handler code here
	if(!suspend_man_thread){
		send(hSock,"E",1,0);
		suspend_man_thread=true;
		GetDlgItem(IDC_BUTTON_THREAD)->SetWindowText("Resume Manual Operation");
	}
	else{
		send(hSock,"F",1,0);
		suspend_man_thread=false;
		GetDlgItem(IDC_BUTTON_THREAD)->SetWindowText("Suspend Manual Operation");
	}
}

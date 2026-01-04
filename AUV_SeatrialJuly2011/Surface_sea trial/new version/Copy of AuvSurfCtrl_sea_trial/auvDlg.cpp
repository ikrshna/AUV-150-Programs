// auvDlg.cpp : implementation file
//
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "stdafx.h"
#include "auv.h"
#include "auvDlg.h"
#include "MFGen_Dlg.h"
#include "colorBtn.h"

#include <winsock2.h>
#include <process.h> 
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib")

SYSTEMTIME st; 


HANDLE comm_hwnd;
LPDWORD id_comm;
DWORD comm_ext;

HANDLE scomm_hwnd;
LPDWORD sid_comm;
DWORD scomm_ext;

unsigned long _stdcall comm(void*);
unsigned long _stdcall scomm(void*);

bool threadrunning=false;
bool server_connected=false;
bool serial_started=false;
bool data_recvd=false;

void client_func(void);
 	
char ac_depth[10];
bool disp_acoustic_depth=false;

char command[22];

FILE *fp;

char buffer[200]="";
char serial_buffer[80]; //buffer for scomm
char time_buffer[12];
//char display_listbox[100];

SOCKET hSock;
HANDLE hPort;
int count;
bool submit=false;
CString str_refdepth;
CString str_missdur;
bool light;
DCB PortDCB;
COMMTIMEOUTS timeout;
DWORD bytesRead;
DWORD bytesWritten;
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
// CAuvDlg dialog

CAuvDlg::CAuvDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAuvDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAuvDlg)
	m_pitch = _T("");
	m_roll = _T("");
	m_yaw = _T("");
	m_surge = _T("");
	m_a_alti = _T("");
	m_d_alti = _T("");
	m_lat = _T("");
	m_long = _T("");
	m_east = _T("");
	m_north = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAuvDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAuvDlg)
	DDX_Control(pDX, IDC_COMBO_REFDEPTH, m_refdepth);
	DDX_Control(pDX, IDC_COMBO_MISSDUR, m_missdur);
	DDX_Control(pDX, IDC_LIST_GENMESS, m_genmess);
	DDX_Control(pDX, IDC_LIST_ACOUSTIC, m_acoustic);
	DDX_Control(pDX, IDC_COMBO_COMMAND, m_combo);
	DDX_Control(pDX,IDC_LIST_ACOUSTICD,m_acoustic_d);
	DDX_Text(pDX, IDC_STATIC_PITCH, m_pitch);
	DDX_Text(pDX, IDC_STATIC_ROLL, m_roll);
	DDX_Text(pDX, IDC_STATIC_YAW, m_yaw);
	DDX_Text(pDX, IDC_STATIC_SURGE, m_surge);
	DDX_Text(pDX, IDC_STATIC_ELAPSED, m_elapsed);
	DDX_Text(pDX, IDC_STATIC_REM, m_remain);
	DDX_Control(pDX, IDC_COMBO_CT, m_ct);
	DDX_Control(pDX, IDC_COMBO_MT, m_mt);
	DDX_Text(pDX, IDC_STATIC_LAT, m_lat);
	DDX_Text(pDX, IDC_STATIC_LON, m_long);
	DDX_Text(pDX, IDC_STATIC_EAST, m_east);
	DDX_Text(pDX, IDC_STATIC_NORTH, m_north);
	DDX_Text(pDX, IDC_STATIC_A_ALTI, m_a_alti);
	DDX_Text(pDX, IDC_STATIC_D_ALTI, m_d_alti);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAuvDlg, CDialog)
	//{{AFX_MSG_MAP(CAuvDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SUBMIT, OnButtonSubmit)
	ON_BN_CLICKED(IDC_BUTTON_AUTO, OnButtonAuto)
	ON_BN_CLICKED(IDC_BUTTON_MAN, OnButtonMan)
	ON_BN_CLICKED(IDC_BUTTON_LIGHT, OnButtonLight)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_MFGEN, OnButtonMFGen)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAuvDlg message handlers

BOOL CAuvDlg::OnInitDialog()
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
	

	comm_hwnd=CreateThread(NULL,0,comm,(void *)0,NULL,id_comm);
	scomm_hwnd=CreateThread(NULL,0,scomm,(void *)0,NULL,sid_comm);
	
	
	b1_safe.Attach(IDC_BUTTON_BAT1_SAFE,this);
	b2_safe.Attach(IDC_BUTTON_BAT2_SAFE,this);
	b3_safe.Attach(IDC_BUTTON_BAT3_SAFE,this);
	b4_safe.Attach(IDC_BUTTON_BAT4_SAFE,this);
	b5_safe.Attach(IDC_BUTTON_BAT5_SAFE,this);
	b6_safe.Attach(IDC_BUTTON_BAT6_SAFE,this);
	b1_alm.Attach(IDC_BUTTON_BAT1_ALM,this);
	b2_alm.Attach(IDC_BUTTON_BAT2_ALM,this);
	b3_alm.Attach(IDC_BUTTON_BAT3_ALM,this);
	b4_alm.Attach(IDC_BUTTON_BAT4_ALM,this);
	b5_alm.Attach(IDC_BUTTON_BAT5_ALM,this);
	b6_alm.Attach(IDC_BUTTON_BAT6_ALM,this);
	b1_crt.Attach(IDC_BUTTON_BAT1_CRT,this);
	b2_crt.Attach(IDC_BUTTON_BAT2_CRT,this);
	b3_crt.Attach(IDC_BUTTON_BAT3_CRT,this);
	b4_crt.Attach(IDC_BUTTON_BAT4_CRT,this);
	b5_crt.Attach(IDC_BUTTON_BAT5_CRT,this);
	b6_crt.Attach(IDC_BUTTON_BAT6_CRT,this);
	//btn_am.Attach(IDC_BUTTON_AM,this);
	btn_rf.Attach(IDC_BUTTON_RF,this);
	btn_leak.Attach(IDC_BUTTON_LEAK,this);
	
	m_combo.SetCurSel(0);	//initialize the combobox item as start
	m_missdur.SetCurSel(0);
	m_refdepth.SetCurSel(0);
	m_ct.SetCurSel(3);
	m_mt.SetCurSel(4);
	
	SetTimer(ID_TIMER1,1000,NULL);

	light=false;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAuvDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAuvDlg::OnPaint() 
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
HCURSOR CAuvDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}




////////////////////////////////////  C O M M  ////////////////////////////////////

unsigned long _stdcall comm(void *data){
	//bool repeat=true;
	
	while(1){
		client_func();
	}
	return 0;
}




/////////////////////////////////////// On Timer //////////////////////////////////////////

void CAuvDlg::OnTimer(UINT nIDEvent){
	// TODO: Add your message handler code here and/or call default
	switch(nIDEvent)
	{
	case ID_TIMER1 :					
			if(server_connected){
				btn_rf.SetColor(BLACK,GREEN);
			}
			else{
				btn_rf.SetColor(BLACK,RED);
			}
				
			if(serial_started)
			{
				m_acoustic.AddString(serial_buffer);
				serial_started=false;
				//m_acoustic.SetCurSel(ind);
			}
			else{	
				/*GetTimeFormat(LOCALE_SYSTEM_DEFAULT,TIME_FORCE24HOURFORMAT,NULL,"HH':'mm':'ss",time_buffer,20);
				sprintf(display_listbox,"AM disconnected ! ! !             %s",time_buffer);
				int idx_genmess=m_genmess.AddString(display_listbox);
				m_genmess.SetCurSel(idx_genmess);*/
			}
			if(disp_acoustic_depth){
				m_acoustic_d.AddString(ac_depth);
				disp_acoustic_depth=false;
			}
				

			break;

	case ID_TIMER_ELAPSED:
		if(count<=(atoi(str_missdur)*60)){
			int ss=0;
			int mm=0;
			int hh=0;
			hh=count/3600;
			mm=(count%3600)/60;
			ss=(count%3600)%60;
			sprintf(time_buffer,"%02d:%02d:%02d",hh,mm,ss);
			m_elapsed.Format("%s",time_buffer);
			
			hh=((atoi(str_missdur)*60)-count)/3600;
			mm=(((atoi(str_missdur)*60)-count)%3600)/60;
			ss=(((atoi(str_missdur)*60)-count)%3600)%60;
			sprintf(time_buffer,"%02d:%02d:%02d",hh,mm,ss);
			m_remain.Format("%s",time_buffer);
			count++;
		}
		UpdateData(false);
		break;	
	}
	
	CDialog::OnTimer(nIDEvent);
}



/////////////////////////////////////////////////////////////////////////////////////

void CAuvDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	OnClose();
	
	CDialog::OnOK();
}





void CAuvDlg::OnButtonAuto()
{
	int index=m_combo.GetCurSel();
	int bytes;
	int rcv_len;
	char msg_buffer[100];
	char data_buffer[500];
	//char temp[20];
	float a_alti;	// variables to store 
	float d_alti;		//data from server.....
	float roll;
	float pitch;
	float yaw;
	float lat;
	float lon;
	float surge;
	float east_pos;
	float north_pos;
	float bank1;
	float bank2;
	float bank3;
	float bank4;
	float bank5;
	float bank6;
	int leak;

	switch(index)
	{
	case 0:	///### Mission START Command ###///
			bytes=sprintf(command,"%c",'S');
			command[bytes+1]='\0';
			/// Disable Mission Particulars ///
			m_refdepth.EnableWindow(false);
			m_missdur.EnableWindow(false);
			GetDlgItem(IDC_BUTTON_SUBMIT)->EnableWindow(false);
			//idx_genmess=m_genmess.AddString(command);
			//m_genmess.SetCurSel(idx_genmess);
			/// Start Timer for the elapsed time of the mission ///
			if(!submit){
				count=1;
				SetTimer(ID_TIMER_ELAPSED,1000,NULL);
				submit=true;
			}
			send(hSock,command,strlen(command),0);
			break;
	case 1:	///### Mission ABORT Command ###///
			bytes=sprintf(command,"%c",'A');
			command[bytes+1]='\0';
			send(hSock,command,strlen(command),0);
			break;
	case 2:	///### System HALT Command ###///
			bytes=sprintf(command,"%c",'H');
			command[bytes+1]='\0';
			send(hSock,command,strlen(command),0);
			break;
	case 3:	///### System REBOOT Command ###///
			bytes=sprintf(command,"%c",'R'); 
			command[bytes+1]='\0';
			send(hSock,command,strlen(command),0);
			break;
	case 4:	///### Request for STATUS Command ###///
			bytes=sprintf(command,"%c",'G'); 
			command[bytes+1]='\0';
			send(hSock,command,strlen(command),0);
			rcv_len=recv(hSock,msg_buffer,strlen(msg_buffer),0);
			msg_buffer[rcv_len]='\0';
			m_genmess.AddString(msg_buffer);
			break;
	case 5:///### Request for DATA Command ###///
			bytes=sprintf(command,"%c",'D'); 
			command[bytes+1]='\0';
			send(hSock,"D",1,0);
			rcv_len=recv(hSock,data_buffer,strlen(data_buffer),0);
			data_buffer[rcv_len]='\0';
			m_genmess.AddString(data_buffer);
			sscanf(data_buffer,"%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d",
						&a_alti,&d_alti, &roll, &pitch, &yaw, &surge, &bank1, &bank2, &bank3, &bank4,
						&bank5, &bank6, &lat, &lon, &east_pos, &north_pos, &leak);
			//sprintf(temp,"%f %f",a_alti,d_alti);
			//m_genmess.AddString(temp);
							////////leak button//////////////
			if(leak==0)		
				btn_leak.SetColor(BLACK,GREEN);
			else if(leak==1)
				btn_leak.SetColor(BLACK,RED);

			/////////battery1 /////////////
			if(bank1>=27 && bank1<=29){
				b1_safe.SetColor(BLACK,GREEN);
				b1_alm.SetColor(BLACK,IVORY2);
				b1_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank1>=25 && bank1<27){
				b1_alm.SetColor(BLACK,YELLOW);
				b1_safe.SetColor(BLACK,IVORY2);
				b1_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank1<25){
				b1_crt.SetColor(BLACK,RED);
				b1_alm.SetColor(BLACK,IVORY2);
				b1_safe.SetColor(BLACK,IVORY2);
			}
			/////////battery2 /////////////
			if(bank2>=27 && bank2<=29){
				b2_safe.SetColor(BLACK,GREEN);
				b2_alm.SetColor(BLACK,IVORY2);
				b2_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank2>=25 && bank2<27){
				b2_alm.SetColor(BLACK,YELLOW);
				b2_safe.SetColor(BLACK,IVORY2);
				b2_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank2<25){
				b2_crt.SetColor(BLACK,RED);
				b2_alm.SetColor(BLACK,IVORY2);
				b2_safe.SetColor(BLACK,IVORY2);
			}
			/////////battery3 /////////////
			if(bank3>=27 && bank3<=29){
				b3_safe.SetColor(BLACK,GREEN);
				b3_alm.SetColor(BLACK,IVORY2);
				b3_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank3>=25 && bank3<27){
				b3_alm.SetColor(BLACK,YELLOW);
				b3_safe.SetColor(BLACK,IVORY2);
				b3_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank3<25){
				b3_crt.SetColor(BLACK,RED);
				b3_alm.SetColor(BLACK,IVORY2);
				b3_safe.SetColor(BLACK,IVORY2);
			}
			/////////battery4 /////////////
			if(bank4>=27 && bank4<=29){
				b4_safe.SetColor(BLACK,GREEN);
				b4_alm.SetColor(BLACK,IVORY2);
				b4_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank4>=25 && bank4<27){
				b4_alm.SetColor(BLACK,YELLOW);
				b4_safe.SetColor(BLACK,IVORY2);
				b4_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank4<25){
				b4_crt.SetColor(BLACK,RED);
				b4_alm.SetColor(BLACK,IVORY2);
				b4_safe.SetColor(BLACK,IVORY2);
			}
			/////////battery5 /////////////
			if(bank5>=27 && bank5<=29){
				b5_safe.SetColor(BLACK,GREEN);
				b5_alm.SetColor(BLACK,IVORY2);
				b5_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank5>=25 && bank5<27){
				b5_alm.SetColor(BLACK,YELLOW);
				b5_safe.SetColor(BLACK,IVORY2);
				b5_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank5<25){
				b5_crt.SetColor(BLACK,RED);
				b5_alm.SetColor(BLACK,IVORY2);
				b5_safe.SetColor(BLACK,IVORY2);
			}
			/////////battery6 /////////////
			if(bank6>=27 && bank6<=29){
				b6_safe.SetColor(BLACK,GREEN);
				b6_alm.SetColor(BLACK,IVORY2);
				b6_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank6>=25 && bank6<27){
				b6_alm.SetColor(BLACK,YELLOW);
				b6_safe.SetColor(BLACK,IVORY2);
				b6_crt.SetColor(BLACK,IVORY2);
			}
			else if(bank6<25){
				b6_crt.SetColor(BLACK,RED);
				b6_alm.SetColor(BLACK,IVORY2);
				b6_safe.SetColor(BLACK,IVORY2);
			}
				

			m_a_alti.Format("%.2f",a_alti);
			m_d_alti.Format("%.2f",d_alti);
			m_roll.Format("%.2f",roll);
			m_pitch.Format("%.2f",pitch);
			m_yaw.Format("%.2f",yaw);
			m_surge.Format("%.2f",surge);
			m_lat.Format("%.2f",lat);
			m_long.Format("%.2f",lon);
			m_east.Format("%.2f",east_pos);
			m_north.Format("%.2f",north_pos);


			UpdateData(false);
			break;
	}
}

void CAuvDlg::OnButtonSubmit() 
{
	// TODO: Add your control notification handler code here
	
	int idx_refdepth=m_refdepth.GetCurSel();
	m_refdepth.GetLBText(idx_refdepth,str_refdepth);
	int idx_missdur=m_missdur.GetCurSel();
	m_missdur.GetLBText(idx_missdur,str_missdur);
	
	int bytes;
	//int idx_genmess;
	
	bytes=sprintf(command,"%c %s %s",'P',str_refdepth,str_missdur);
	command[bytes+1]='\0';
	send(hSock,command,strlen(command),0);
	
	//idx_genmess=m_genmess.AddString(command);
	//m_genmess.SetCurSel(idx_genmess);
		
}

void CAuvDlg::OnButtonMan()
{
	int idx_ct=m_ct.GetCurSel();
	CString str_ct;
	m_ct.GetLBText(idx_ct,str_ct);

	int idx_mt=m_mt.GetCurSel();
	CString str_mt;
	m_mt.GetLBText(idx_mt,str_mt);

	int bytes=sprintf(command,"%c %s %s",'M',str_ct,str_mt);
	command[bytes+1]='\0';

	/*========================================================================================*/
	send(hSock,command,strlen(command),0);
	*command=NULL;
	//int inDataLength=recv(hSock,buffer,112,0);
	//buffer[inDataLength]='\0';
	/*========================================================================================*/

}

void CAuvDlg::OnButtonLight()
{
	//int bytes;
	if(!light){
		GetDlgItem(IDC_BUTTON_LIGHT)->SetWindowText("Switch Light OFF");
		//bytes=sprintf(command,"%c",'N');
		//command[bytes+1]='\0';
		send(hSock,"N",1,0);
		light=true;
	}
	else{
		GetDlgItem(IDC_BUTTON_LIGHT)->SetWindowText("Switch Light ON");
		//bytes=sprintf(command,"%c",'F');
		//command[bytes+1]='\0';
		send(hSock,"F",1,0);
		light=false;
	}
}



void client_func(void)
{
	//----------Initialize winsock2.2 dll---------------------------------------
	WSADATA wsaData={0};
	WORD wVersionRequested=MAKEWORD(2,2);
	int nRet=WSAStartup(wVersionRequested,&wsaData);
	if (nRet==SOCKET_ERROR){
		return;
	}

	//--------Open a socket-------------------------------------------------------
	hSock=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	if (hSock==INVALID_SOCKET){
		return;
	}

	//---------Name the socket--------------------------------------------------
	sockaddr_in sa;

	sa.sin_family = PF_INET;
	sa.sin_port=htons(8888);
	sa.sin_addr.s_addr=inet_addr("192.168.1.10");

	//-----------------connect---------------------------------------------------------------
	nRet = connect( hSock, (sockaddr*)&sa, sizeof( sockaddr ) ); 

	if( nRet == SOCKET_ERROR ) {
		server_connected=false;
		closesocket(hSock);
		return;
	}
	else{
		server_connected=true;
	}
	
	while(1){

	}

}


unsigned long _stdcall scomm(void*)
{
		
	FILE *log;
	bool com_port_open;
	char time_buffer[30];
	bool found_d=false;
	int index=0;
	
_PortOpen:	hPort=CreateFile("COM1",GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
			if(hPort!=INVALID_HANDLE_VALUE){
				PortDCB.DCBlength = sizeof(DCB); 
				GetCommState(hPort,&PortDCB); 
				PortDCB.BaudRate = 9600;      
				PortDCB.ByteSize = 8;              
				PortDCB.Parity = NOPARITY;         
				PortDCB.StopBits = ONESTOPBIT;    
				SetCommState(hPort,&PortDCB);

				timeout.ReadTotalTimeoutConstant =5000;
				timeout.ReadTotalTimeoutMultiplier =0;
				timeout.WriteTotalTimeoutConstant =0;
				timeout.WriteTotalTimeoutMultiplier =0;
				SetCommTimeouts(hPort,&timeout);

				com_port_open=true;
			}
			else
				com_port_open=false;

	
	while(1){
		if(com_port_open){
			struct _timeb start;
			struct _timeb stop;
			int timeelapsed;

			_ftime(&start);
			ReadFile(hPort,serial_buffer,37,&bytesRead,NULL);
			_ftime(&stop);
			timeelapsed=stop.time-start.time;
			
			if(timeelapsed>=5){
				sprintf(serial_buffer,"Read Timed Out");
				serial_started=true;
			}
			else{
				serial_buffer[bytesRead]='\0';
				GetTimeFormat(LOCALE_SYSTEM_DEFAULT,TIME_FORCE24HOURFORMAT,NULL,"HH':'mm':'ss",time_buffer,30);
				log=fopen("Data_Acoustic.txt","a+");
				fprintf(log,"%s %s\n",time_buffer,serial_buffer);
				fclose(log);
				for(int m=0;m<bytesRead;m++){
					if(serial_buffer[m]=='D' && serial_buffer[m+1]==','){
						m=m+2;
						found_d=true;
					}
					if(found_d && serial_buffer[m]!=','){
						ac_depth[index++]=serial_buffer[m];
					}
					else{
						ac_depth[index]='\0';
						break;
					}
				}
				index=0;
				found_d=false;
				if(m<bytesRead){
					disp_acoustic_depth=true;
				}
				serial_started=true;
			}
		}
		else{
			_sleep(1000);
			goto _PortOpen;
		}

	}
	
}


void CAuvDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	//CloseHandle(hPort);
	send(hSock,"T",1,0);
	closesocket(hSock);
	hSock=0;
	WSACleanup();
	TerminateThread(comm_hwnd,comm_ext);
	TerminateThread(scomm_hwnd,scomm_ext);
	KillTimer(ID_TIMER1);
	KillTimer(ID_TIMER_ELAPSED);
	
	CDialog::OnClose();
}

void CAuvDlg::OnButtonMFGen(){
	CMFGen_Dlg mf;
	mf.DoModal();
}


void CAuvDlg::OnButtonStop()
{
	WriteFile(hPort,"X",1,&bytesWritten,NULL);
}




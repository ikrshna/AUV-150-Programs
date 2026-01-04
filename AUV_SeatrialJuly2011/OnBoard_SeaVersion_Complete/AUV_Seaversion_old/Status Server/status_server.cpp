#include <winsock2.h>

#pragma comment (lib,"ws2_32.lib")

#include <iostream>

#include "C:\Program Files\Advantech\Adsapi\Include\driver.h" 
#include "ComConfig.h"
#include "winbase.h"
#include "process.h"

using namespace std;


char recvBuffer[200];
char buffer[2600];
int ctn;
int ctt;
bool repeat;
CComConfig hor;

int altimeter();
int depth();
int phins();
int DVL();
int GPS();

unsigned long _stdcall man_orient(void *);
HANDLE hwnd_orient;
LPDWORD id_orient;
DWORD exit_th;
///// DA variables /////
#define     MAX_DEVICES     100 

static PT_AOVoltageOut	ptAOVoltageOut;
static PT_AOConfig		ptAOConfig;
static PT_DioWriteBit	ptDioWriteBit;


	DEVLIST     DeviceList[MAX_DEVICES];
	LONG        DriverHandle = (LONG)NULL;          // driver handle
	BOOL        bRun = FALSE;                       // flag for running
	float       fBuffer[4] = {5.0f, 2.5f, 1.25f, 0.0f};
	USHORT      gwIndex = 0;
	USHORT      gwDevice = 0, gwSubDevice = 0;      // device index
	USHORT      gwChannel = 0;                      // input channel
	SHORT       gnNumOfDevices;  // number of installed devices
	int         nOutEntries;
	LONG ErrCde;				// error code
	float A_OUT=0.00;

	char utm_data[100];

//////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	SOCKET hSock;
	SOCKET hClient;
	sockaddr_in sa;
	sockaddr_in saClient;
	bool connected=false;
	char data[2600];
	char header;
	char ct_cmd[10];
	char ct_cmd2[10];
	char mt_cmd[10];
	float mt;
	char zone[10];
	double northing;
	double easting;
	FILE *utm_fp;
	


	//--------------  initialize DA registers ---------------//

		/// getting the list of devices installed
	DRV_DeviceGetNumOfList(&gnNumOfDevices);
	//cout<<gnNumOfDevices<<endl;

	/// getting info regarding the installed devices
	DRV_DeviceGetList((DEVLIST far *)&DeviceList[0],MAX_DEVICES,(SHORT far *)&nOutEntries);
	//cout<<DeviceList[0].dwDeviceNum<<DeviceList[0].szDeviceName<<endl;
	/// initialising the installed device
	ErrCde=DRV_DeviceOpen(DeviceList[0].dwDeviceNum ,(LONG far *)&DriverHandle);
	if(ErrCde!=SUCCESS)
		cout<<"Error in opening the installed device"<<endl;
	else


		cout<<"Device opened successfully"<<endl;
	/// setting AO parameters
	ptAOConfig.chan =gwChannel;
	ptAOConfig.MaxValue =5.00;
	ptAOConfig.MinValue =-5.00;
	ErrCde=DRV_AOConfig(DriverHandle,&ptAOConfig);
	if(ErrCde!=SUCCESS)
		cout<<"cannot be configured"<<endl;
	else
		cout<<"configured successfully"<<endl;
	
	/// output data
	ptAOVoltageOut.chan =gwChannel;

	//------------  DA Initialization Complete -----------------//

	// -----------  Create thread for manual orientation   --------//
	hwnd_orient=CreateThread(NULL,0,man_orient,(void *)0,NULL,id_orient);

	//------------- Initialize winsock2.2 dll ---------------//
	WSADATA wsaData={0};
	WORD wVersionRequested=MAKEWORD(2,2);
	int nRet=WSAStartup(wVersionRequested,&wsaData);
	if(nRet==SOCKET_ERROR){
		cout<<"ERROR : "<<WSAGetLastError()<<endl;
	}

	//------------- Open a socket --------------------------//
	hSock=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	if(hSock==INVALID_SOCKET){
		cout<<"Invalid socket,failed to create the socket..."<<endl;
	}

	//-------------- Name the socket -----------------------//
	sa.sin_family=PF_INET;
	sa.sin_port=htons(8000);
	sa.sin_addr.S_un.S_addr=htonl(INADDR_ANY);

	//-----------bind the socket's name-----------------------------------------
	nRet=bind(hSock,(sockaddr*)&sa,sizeof(sa));
	if (nRet==SOCKET_ERROR){
		cout<<"ERROR:"<<WSAGetLastError()<<endl;
	}
		
	
	while(1){
		
		//--------------- Listen -----------------------------//
		cout<<"Listening for connections..."<<endl;
		nRet=listen(hSock,5);	//connection backlog queue set to 5

		if (nRet==SOCKET_ERROR){
			cout<<"ERROR:"<<WSAGetLastError()<<endl;
			closesocket(hSock);
		}
		//--------------- Accept -----------------------------//
		int nSALen=sizeof(sockaddr);
		hClient=accept(hSock,(sockaddr*)&saClient,&nSALen);
		if (hClient==INVALID_SOCKET){
			cout<<"Invalid client socket,connection failed!"<<endl;
			closesocket(hSock);
		}
		else{
			cout<<"Connection established"<<endl;
		}
		
		for(;;){
			//--------------- Receive & Send ----------------------------//
			int inDataLength=recv(hClient,recvBuffer,sizeof(recvBuffer),0);
			cout<<"Data received : "<<recvBuffer<<endl;
			recvBuffer[inDataLength]='\0';
			
			switch(recvBuffer[0])
			{
			case '1'	:	switch(altimeter())
							{
							case 0	:	sprintf(data,"s %s",buffer);
										send(hClient,data,strlen(data),0);
										break;

							case -1	:	send(hClient,"n",1,0);
										break;
						
							case -2	:	send(hClient,"f",1,0);
										break;
							}
							break;
						
			case '2'	:	switch(depth())
							{
							case 0	:	sprintf(data,"s %s",buffer);
										send(hClient,data,strlen(data),0);
										*buffer=NULL;
										*data=NULL;
										break;

							case -1	:	send(hClient,"n",1,0);
										break;
						
							case -2	:	send(hClient,"f",1,0);
										break;
							}
							break;
			
			case '3'	:	switch(phins())
							{
							case 0	:	sprintf(data,"s %s",utm_data);
										send(hClient,data,strlen(data),0);
										*utm_data=NULL;
										*buffer=NULL;
										*data=NULL;
										break;

							case -1	:	send(hClient,"n",1,0);
										break;
						
							case -2	:	send(hClient,"f",1,0);
										break;
							}
							break;

			case '4'	:	switch(DVL())
							{
							case 0	:	sprintf(data,"s %s",buffer);
										send(hClient,data,strlen(data),0);
										*buffer=NULL;
										*data=NULL;
										break;

							case -1	:	send(hClient,"n",1,0);
										break;
						
							case -2	:	send(hClient,"f",1,0);
										break;
							}
							break;

			case '5'	:	switch(GPS())
							{
							case 0	:	sprintf(data,"s %s",buffer);
										send(hClient,data,strlen(data),0);
										*buffer=NULL;
										*data=NULL;
										break;

							case -1	:	send(hClient,"n",1,0);
										break;
						
							case -2	:	send(hClient,"f",1,0);
										break;
							}
							break;

			case '6'	:	break;
			case '7'	:	break;
			case '8'	:	ptDioWriteBit.port  = 0; // output port: 0
							ptDioWriteBit.bit   = 1; // output channel
							ptDioWriteBit.state = 1; // output state
							DRV_DioWriteBit(DriverHandle,&ptDioWriteBit);
							break;
			case '9'	:	ptDioWriteBit.port  = 0; // output port: 0
							ptDioWriteBit.bit   = 1; // output channel
							ptDioWriteBit.state = 0; // output state
							DRV_DioWriteBit(DriverHandle,&ptDioWriteBit);
							break;
			case 'A'	:	sscanf(recvBuffer,"%c %s %s %s",&header,ct_cmd,ct_cmd2,mt_cmd);
							ctn=atoi(ct_cmd);
							ctt=atoi(ct_cmd2);
							mt=atof(mt_cmd);
							ptAOVoltageOut.OutputValue=mt;
							DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
							_sleep(3000);
							ptAOVoltageOut.OutputValue=0.00;
							DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
							break;
			case 'B'	:	WinExec("AUV_onboard_dummy.exe",SW_SHOW);
							break;
			case 'C'	:	ptDioWriteBit.port  = 0; // output port: 0
							ptDioWriteBit.bit   = 2; // output channel
							ptDioWriteBit.state = 1; // output state
							DRV_DioWriteBit(DriverHandle,&ptDioWriteBit);
							break;
			case 'D'	:	sscanf(recvBuffer,"%c %s %lf %lf",&header,&zone,&northing,&easting);
							utm_fp=fopen("utm_coord.txt","w+");
							fprintf(utm_fp,"%s %.2lf %.2lf",zone,northing,easting);
							fclose(utm_fp);
							break;
			case 'E'	:	TerminateThread(hwnd_orient,exit_th);
							hor.EndCom();
							break;
			case 'F'	:	hwnd_orient=CreateThread(NULL,0,man_orient,(void *)0,NULL,id_orient);
							break;
			}

			//----- close client connection ---------------//
			if(strcmp(recvBuffer,"#")==0){
				closesocket(hClient);
				hClient=0;
				break;
			}

			//------ Close Status Server --------------//
			if(strcmp(recvBuffer,"t")==0){
				closesocket(hSock);
				hSock=0;
				WSACleanup();
				repeat=false;
				TerminateThread(hwnd_orient,exit_th);
				CloseHandle(hwnd_orient);
				return 0;
			}
		}
	
	}
	closesocket(hSock);						//close socket
	hSock=0;
	WSACleanup();							//Release Winsock dll
	return 0;
}




int altimeter()
{
	HANDLE hSerial;
	
	hSerial=CreateFile("COM8",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hSerial==INVALID_HANDLE_VALUE){
		cout<<"COM8 could not be opened."<<endl;
		return -1;
	}
	
	DCB dcbSerialParams={0};
	
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	
	if(!GetCommState(hSerial,&dcbSerialParams))
		cout<<"Error getting COM8 port state."<<endl;

	dcbSerialParams.BaudRate=9600;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=ONESTOPBIT;
	dcbSerialParams.Parity=NOPARITY;
	
	if(!SetCommState(hSerial,&dcbSerialParams))
		cout<<"Error setting COM8 port state."<<endl;
	
	COMMTIMEOUTS timeouts={0};

	timeouts.ReadIntervalTimeout=50;
	timeouts.ReadTotalTimeoutConstant=50;
	timeouts.ReadTotalTimeoutMultiplier=10;
	timeouts.WriteTotalTimeoutConstant=50;
	timeouts.WriteTotalTimeoutMultiplier=10;

	if(!SetCommTimeouts(hSerial,&timeouts))
		cout<<"Error in setting timeouts for COM8."<<endl;
	
	DWORD bytesRead;
	
	ReadFile(hSerial,buffer,50,&bytesRead,NULL);
	buffer[bytesRead]='\0';
	cout<<buffer<<endl;
	if(buffer!=NULL){
		CloseHandle(hSerial);
		return 0;
	}
	else{
		CloseHandle(hSerial);
		return -2;
	}
	
	
}


int depth()
{
	HANDLE hSerial;
	
	hSerial=CreateFile("COM9",GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hSerial==INVALID_HANDLE_VALUE){
		cout<<"COM9 could not be opened."<<endl;
		return -1;
	}
	
	DCB dcbSerialParams={0};
	
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	
	if(!GetCommState(hSerial,&dcbSerialParams))
		cout<<"Error getting COM9 port state."<<endl;

	dcbSerialParams.BaudRate=9600;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=ONESTOPBIT;
	dcbSerialParams.Parity=NOPARITY;
	
	if(!SetCommState(hSerial,&dcbSerialParams))
		cout<<"Error setting COM9 port state."<<endl;
	
	COMMTIMEOUTS timeouts={0};

	timeouts.ReadIntervalTimeout=50;
	timeouts.ReadTotalTimeoutConstant=50;
	timeouts.ReadTotalTimeoutMultiplier=10;
	timeouts.WriteTotalTimeoutConstant=50;
	timeouts.WriteTotalTimeoutMultiplier=10;

	if(!SetCommTimeouts(hSerial,&timeouts))
		cout<<"Error in setting timeouts for COM9."<<endl;
	
	DWORD bytesRead;
	DWORD bytesWritten;
	WriteFile(hSerial,"\r",1,&bytesWritten,NULL);
	ReadFile(hSerial,buffer,100,&bytesRead,NULL);
	buffer[bytesRead]='\0';
	cout<<buffer<<endl;
	if(buffer!=NULL){
		CloseHandle(hSerial);
		return 0;
	}
	else{
		CloseHandle(hSerial);
		return -2;
	}
}


int phins()
{
	HANDLE hSerial;
	
	hSerial=CreateFile("COM1",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hSerial==INVALID_HANDLE_VALUE){
		cout<<"COM1 could not be opened."<<endl;
		return -1;
	}
	
	DCB dcbSerialParams={0};
	
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	
	if(!GetCommState(hSerial,&dcbSerialParams))
		cout<<"Error getting COM1 port state."<<endl;

	dcbSerialParams.BaudRate=57600;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=TWOSTOPBITS;
	dcbSerialParams.Parity=ODDPARITY;
	
	if(!SetCommState(hSerial,&dcbSerialParams))
		cout<<"Error setting COM1 port state."<<endl;
	
	COMMTIMEOUTS timeouts={0};

	timeouts.ReadIntervalTimeout=50;
	timeouts.ReadTotalTimeoutConstant=50;
	timeouts.ReadTotalTimeoutMultiplier=10;
	timeouts.WriteTotalTimeoutConstant=50;
	timeouts.WriteTotalTimeoutMultiplier=10;

	if(!SetCommTimeouts(hSerial,&timeouts))
		cout<<"Error in setting timeouts for COM1."<<endl;
	
	DWORD bytesRead;
	DWORD bytesWritten;
	char p_cmd[30];
	int index=0;
	bool found=false;

	sprintf(p_cmd,"$PIXSE,CONFIG,WAKEUP*40%x%x",0x0D,0x0A);
	WriteFile(hSerial,p_cmd,strlen(p_cmd),&bytesWritten,NULL);
	_sleep(2000);
	
	ReadFile(hSerial,buffer,1500,&bytesRead,NULL);
	buffer[bytesRead]='\0';

	for(int i=0;i<bytesRead;i++){
		if(buffer[i]=='U' && buffer[i+1]=='T' && buffer[i+2]=='M' && buffer[i+3]=='W' && buffer[i+4]=='G' && buffer[i+5]=='S' && buffer[i+6]==','){
			i=i+7;
			found=true;
		}
		if(found){
			if(buffer[i]=='$'){
				utm_data[index]='\0';
				break;
			}
			else{
				utm_data[index]=buffer[i];
				index=index+1;
			}
		}
	}

	cout<<buffer<<endl;
	if(buffer!=NULL){
		CloseHandle(hSerial);
		return 0;
	}
	else{
		CloseHandle(hSerial);
		return -2;
	}
}

int DVL()
{
	HANDLE hSerial;
	
	hSerial=CreateFile("COM7",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hSerial==INVALID_HANDLE_VALUE){
		cout<<"COM7 could not be opened."<<endl;
		return -1;
	}
	
	DCB dcbSerialParams={0};
	
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	
	if(!GetCommState(hSerial,&dcbSerialParams))
		cout<<"Error getting COM7 port state."<<endl;

	dcbSerialParams.BaudRate=9600;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=ONESTOPBIT;
	dcbSerialParams.Parity=NOPARITY;
	
	if(!SetCommState(hSerial,&dcbSerialParams))
		cout<<"Error setting COM7 port state."<<endl;
	
	COMMTIMEOUTS timeouts={0};

	timeouts.ReadIntervalTimeout=50;
	timeouts.ReadTotalTimeoutConstant=50;
	timeouts.ReadTotalTimeoutMultiplier=10;
	timeouts.WriteTotalTimeoutConstant=50;
	timeouts.WriteTotalTimeoutMultiplier=10;

	if(!SetCommTimeouts(hSerial,&timeouts))
		cout<<"Error in setting timeouts for COM7."<<endl;
	
	DWORD bytesRead;
	
	ReadFile(hSerial,buffer,2000,&bytesRead,NULL);
	buffer[bytesRead]='\0';
	cout<<buffer<<endl;
	if(buffer!=NULL){
		CloseHandle(hSerial);
		return 0;
	}
	else{
		CloseHandle(hSerial);
		return -2;
	}
}

int GPS()
{
	HANDLE hSerial;
	
	hSerial=CreateFile("\\\\.\\COM10",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hSerial==INVALID_HANDLE_VALUE){
		cout<<"COM10 could not be opened."<<endl;
		return -1;
	}
	
	DCB dcbSerialParams={0};
	
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	
	if(!GetCommState(hSerial,&dcbSerialParams))
		cout<<"Error getting COM10 port state."<<endl;

	dcbSerialParams.BaudRate=9600;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=ONESTOPBIT;
	dcbSerialParams.Parity=NOPARITY;
	
	if(!SetCommState(hSerial,&dcbSerialParams))
		cout<<"Error setting COM10 port state."<<endl;
	
	COMMTIMEOUTS timeouts={0};

	timeouts.ReadIntervalTimeout=50;
	timeouts.ReadTotalTimeoutConstant=50;
	timeouts.ReadTotalTimeoutMultiplier=10;
	timeouts.WriteTotalTimeoutConstant=50;
	timeouts.WriteTotalTimeoutMultiplier=10;

	if(!SetCommTimeouts(hSerial,&timeouts))
		cout<<"Error in setting timeouts for COM10."<<endl;
	
	DWORD bytesRead;
	
	ReadFile(hSerial,buffer,500,&bytesRead,NULL);
	buffer[bytesRead]='\0';
	cout<<buffer<<endl;
	if(buffer!=NULL){
		CloseHandle(hSerial);
		return 0;
	}
	else{
		CloseHandle(hSerial);
		return -2;
	}
}

unsigned long _stdcall man_orient(void *data){
	int res_tailtorque;
	int res_nosetorque;
	char command_n[10];
	char command_t[10];

	repeat=true;
	while(repeat){
		hor.ComInit ("COM6",1);
		hor.SetConfigParam (57600,8,NOPARITY,ONESTOPBIT);
		res_tailtorque=ctt;
		res_nosetorque=ctn;
		if(res_tailtorque<0){
			if(res_tailtorque<-60)
				res_tailtorque=-60;
			sprintf(command_t,"u37-%d\r",abs(res_tailtorque));
			hor.DeviceWrite (command_t);
		}
		if(res_tailtorque>0){
			if(res_tailtorque>60)
				res_tailtorque=60;
			sprintf(command_t,"u37+%d\r",abs(res_tailtorque));
			hor.DeviceWrite (command_t);
		}
		_sleep(100);
		if(res_nosetorque<0){
			if(res_nosetorque<-60)
				res_nosetorque=-60;
			sprintf(command_n,"u17-%d\r",abs(res_nosetorque));
			hor.DeviceWrite (command_n);
		}
		if(res_nosetorque>0){
			if(res_nosetorque>60)
				res_nosetorque=60;
			sprintf(command_n,"u17+%d\r",abs(res_nosetorque));
			hor.DeviceWrite (command_n);
		}
		hor.EndCom ();
	}
	return 0;
}
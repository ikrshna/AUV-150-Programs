// ComConfig.cpp: implementation of the CComConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "ComConfig.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CComConfig::CComConfig()
{
	hPort=NULL;
	sBuffer=NULL;
	iBytesRead=0;
	iBytesWritten=0;
}

CComConfig::~CComConfig()
{
	delete sBuffer;

}

int CComConfig::ComInit(char* device_obj,int io_opts)//accepting device ID or a filename along with read write options
{
	//The access options are as follows:
	//	1. READ_ONLY	0
	//	2. WRITE_ONLY	1
	//	3. READ_WRITE	2
	
	switch(io_opts){

		case READ_ONLY:
			hPort=CreateFile(device_obj,GENERIC_READ,0,NULL,OPEN_ALWAYS,0,NULL);
			if(hPort==INVALID_HANDLE_VALUE)
				return 1;
			else
				break;
		case WRITE_ONLY:
			hPort=CreateFile(device_obj,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,0,NULL);
			if(hPort==INVALID_HANDLE_VALUE)
				return 1;
			else
				break;
		case READ_WRITE:
			hPort=CreateFile(device_obj,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,0,NULL);
			if(hPort==INVALID_HANDLE_VALUE){
				printf("port could not be opened\n");
				return 1;
			}

			else
				break;
		default:
			printf("Not a registered access option");
			return 1;
	}
	return 0;
}

void CComConfig::SetConfigParam(int baud,int bytesz,int par,int stopb)
{
	//setting up configuration parameters for the device

	//PortDCB.DCBlength = sizeof (DCB); 
	if(!GetCommState(hPort,&PortDCB))
		printf("GetCommStateError\n");
	PortDCB.BaudRate = baud;      
	PortDCB.ByteSize = bytesz;              
	PortDCB.Parity = par;         
	PortDCB.StopBits = stopb;    
	if (!SetCommState(hPort,&PortDCB))
		printf("SetCommStateError\n");
	
}

int CComConfig::DeviceWrite(char *data)	//writing data to the device
{	
	
	int result;
	result=WriteFile(hPort,data,strlen(data),&iBytesWritten,NULL);
	if(result)
		return iBytesWritten;
	else
		return -1;
}

char* CComConfig::DeviceRead()  //reading data from the device
{
	int result;
	char buff[1500];
	char *sBuffer;
	sBuffer=buff;
	//char *sBuffer=new char;
	result=ReadFile(hPort,&buff,1500,&iBytesRead,NULL);
	buff[iBytesRead]='\0';

	/*if(result){
		sBuffer=this->sBuffer;
		return iBytesRead;
	}
	else
		return -1;*/
	//if(iBytesRead>0)
		return sBuffer;
	

}

int CComConfig::EndCom()	//terminating the device
{
	CloseHandle(hPort);
	return 0;
}
// ComConfig.h: interface for the CComConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMCONFIG_H__FFF4301A_0DBE_4C4B_8A7B_CB3DB9746533__INCLUDED_)
#define AFX_COMCONFIG_H__FFF4301A_0DBE_4C4B_8A7B_CB3DB9746533__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <stdio.h>

#define READ_ONLY	0
#define WRITE_ONLY	1
#define READ_WRITE	2


class CComConfig  
{
public:
	int EndCom();
	char* DeviceRead();
	int DeviceWrite(char *);
	void SetConfigParam(int,int,int,int);
	int ComInit(char *,int);
	CComConfig();
	virtual ~CComConfig();

private:
	HANDLE hPort;
	DCB PortDCB;
	char *sBuffer;
	DWORD iBytesRead;
	DWORD iBytesWritten;
};
#endif // !defined(AFX_COMCONFIG_H__FFF4301A_0DBE_4C4B_8A7B_CB3DB9746533__INCLUDED_)

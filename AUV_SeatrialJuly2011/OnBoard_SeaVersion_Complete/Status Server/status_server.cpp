#include <winsock2.h>
//#include <windows.h>

#pragma comment (lib,"ws2_32.lib")

#include <iostream>

using namespace std;



SOCKET hSock;
SOCKET hClient;
sockaddr_in sa;
sockaddr_in saClient;

void socket_server(void);

char recvBuffer[1];
char sendBuffer[10];

/*unsigned long _stdcall thread(void*);
HANDLE hThread;
LPDWORD id_thread;
DWORD thread_exit;*/
void server_socket(void);







//////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
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
	sa.sin_port=htons(8888);
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
		else
			cout<<"Connection established"<<endl;

		//--------------- Receive ----------------------------//
		int inDataLength=recv(hClient,recvBuffer,sizeof(recvBuffer),0);
		cout<<"Data received : "<<recvBuffer<<endl;
		recvBuffer[inDataLength]='\0';
		

		//---------------- Send -------------------------------//
		/*send(hClient,sendBuffer,sizeof(sendBuffer),0);
		cout<<"Data sent : "<<sendBuffer<<endl;
		*sendBuffer=NULL;*/
		//Sleep(1000);
	}
	closesocket(hSock);						//close socket
	hSock=0;
	WSACleanup();							//Release Winsock dll
	return 0;
}


unsigned long _stdcall thread(void* data)
{
	while(1){
		server_socket();
	}
	return 0;
}


void server_socket(void)
{
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
	sa.sin_port=htons(8888);
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
		else
			cout<<"Connection established"<<endl;

		//--------------- Receive ----------------------------//
		int inDataLength=recv(hClient,recvBuffer,sizeof(recvBuffer),0);
		cout<<"Data received : "<<recvBuffer<<endl;
		recvBuffer[inDataLength]='\0';
		

		//---------------- Send -------------------------------//
		/*send(hClient,sendBuffer,sizeof(sendBuffer),0);
		cout<<"Data sent : "<<sendBuffer<<endl;
		*sendBuffer=NULL;*/
		Sleep(1000);
	}
	closesocket(hSock);						//close socket
	hSock=0;
	WSACleanup();							//Release Winsock dll
	
}
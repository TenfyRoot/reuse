// client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock.h>
#include <string.h>
#include <time.h>
#include <map>

#pragma comment(lib,"ws2_32.lib")

char *serveraddr = "127.0.0.1";

typedef struct{
	int sockfrom;
	int sockto;
} tagSock;
int sock80;

unsigned long __stdcall ThreadAccept(void* param);
unsigned long __stdcall ThreadRecv(void* param);
int ConnectHost(unsigned long dwIP, int nPort, bool reuse=false);
int DataSend(SOCKET s, char *DataBuf, int DataLen);

int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc >= 2 )
	{
		serveraddr = argv[1];
	}

	WSAData wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);

	//sock80 = ConnectHost(inet_addr("127.0.0.1"), 80);

	int sockfd = ConnectHost(inet_addr(serveraddr), 9999, true);
	if (sockfd <= 0) goto end;

	struct sockaddr_in addrhole;
	int len = sizeof(struct sockaddr);
	getsockname(sockfd,(struct sockaddr*)&addrhole,&len);

	int socklisten = socket(2,1,0);
	bool breuse = true;
	setsockopt(socklisten,-1,4,(const char*)&breuse,sizeof(bool));
	unsigned long dwArg = 1;
	ioctlsocket(socklisten,FIONBIO, &dwArg);
	bind(socklisten,(struct sockaddr*)&addrhole,sizeof(struct sockaddr));
	printf("client>>bind %s:%d\r\n", inet_ntoa(addrhole.sin_addr),ntohs(addrhole.sin_port));
	listen(socklisten, 100);
	unsigned long threadId;
	::CreateThread(0,0,ThreadAccept,(void*)socklisten,0,&threadId);

	while(true) {
		Sleep(1000);
	}

end:
	WSACleanup();
	getchar();
	return 0;
}

unsigned long __stdcall ThreadAccept(void* param)
{
	int socklisten = (int)param;
	
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr);
	while(true) {
		int newsock = accept(socklisten, (struct sockaddr *)&addr, &len);
		if (newsock > 0) {
			tagSock* stSock = new tagSock();
			stSock->sockfrom = newsock;
			printf("client>>accept %s:%d\r\n", inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
			stSock->sockto = ConnectHost(inet_addr("127.0.0.1"), 80);
			unsigned long threadRecvId;
			::CreateThread(0,0,ThreadRecv,(void*)stSock,0,&threadRecvId);
		}
		Sleep(100);
	}
	return 0;
}

#define CLOCKS_PER_SEC ((clock_t)1000)
unsigned long __stdcall ThreadRecv(void* param)
{
	tagSock* stSock = (tagSock*)param;
	//printf("sock:%d,%d,%d\r\n", stSock->socklisten,stSock->sockto,stSock->sockfrom);
	char RecvBuf[1024*100] = {0};
	fd_set fdset;
	int ret, nRecv;
	clock_t start, finish;
	finish = start = clock();
	while(1)
	{
		FD_ZERO(&fdset);
		FD_SET(stSock->sockfrom, &fdset);
		FD_SET(stSock->sockto, &fdset);
		ret = select(0, &fdset, 0, 0, 0);
		if(ret <= 0)
			goto error;
		if(FD_ISSET(stSock->sockfrom, &fdset))
		{
			nRecv = recv(stSock->sockfrom, RecvBuf, sizeof(RecvBuf), 0);
			if(nRecv <= 0)
				goto error;
			ret = DataSend(stSock->sockto, RecvBuf, nRecv);
			if(ret == 0 || ret != nRecv)
				goto error;
		}
		if(FD_ISSET(stSock->sockto, &fdset))
		{
			nRecv = recv(stSock->sockto, RecvBuf, sizeof(RecvBuf), 0);
			if(nRecv <= 0)
				goto error;
			ret = DataSend(stSock->sockfrom, RecvBuf, nRecv);
			if(ret == 0 || ret != nRecv)
				goto error;
		}
	}
error:
	if (ret <= 0 || nRecv <= 0)
		printf( "[error-%d]", WSAGetLastError() );
	closesocket(stSock->sockfrom);
	//closesocket(stSock->sockto);
	double duration = (double)(clock() - start) / CLOCKS_PER_SEC;
	printf( "-->%f seconds\n", duration );
	delete stSock;
	return 0;
}

int ConnectHost(unsigned long dwIP, int nPort, bool breuse)
{
	int sockid;
	if ((sockid = socket(2,1,0)) == INVALID_SOCKET)
		return 0;
	setsockopt(sockid,-1,4,(const char*)&breuse,sizeof(bool));
	struct sockaddr_in srv_addr;
	srv_addr.sin_family = 2;
	srv_addr.sin_addr.S_un.S_addr = dwIP;
	srv_addr.sin_port = htons(nPort);
	if (connect(sockid,(struct sockaddr*)&srv_addr,sizeof(struct sockaddr_in)) == -1)
	{
		printf("connect[%d] errno:%d",nPort, WSAGetLastError());
		goto error;
	}
	return sockid;
error:
	closesocket(sockid);
	return 0;
}

int nTimes = 0;
int DataSend(SOCKET s, char *DataBuf, int DataLen)//将DataBuf中的DataLen个字节发到s去
{
	int nBytesLeft = DataLen;
	int nBytesSent = 0;
	int ret;
	//set socket to blocking mode
	int iMode = 0;
	ioctlsocket(s, FIONBIO, (u_long FAR*) &iMode);
	while(nBytesLeft > 0)
	{
		ret = send(s, DataBuf + nBytesSent, nBytesLeft, 0);
		if(ret <= 0)
			break;
		nBytesSent += ret;
		nBytesLeft -= ret;
	}
	return nBytesSent;
}

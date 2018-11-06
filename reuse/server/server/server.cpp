// server.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

int sockfd = 0;

unsigned long __stdcall ThreadFunc(void* param);

int _tmain(int argc, _TCHAR* argv[])
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);

	sockfd = socket(2,1,0);
	bool breuse = true;
	setsockopt(sockfd,-1,4,(const char*)&breuse,sizeof(bool));

	struct sockaddr_in addr;
	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = 2;
	addr.sin_addr.s_addr = htonl(0);
	addr.sin_port = htons(9999);

	bind(sockfd,(struct sockaddr*)&addr,sizeof(struct sockaddr));

	listen(sockfd, 1000);

	unsigned long threadId;
	::CreateThread(0,0,ThreadFunc,0,0,&threadId);
	
	while(true) {
		Sleep(1000);
	}
	WSACleanup();
	return 0;
}

unsigned long __stdcall ThreadFunc(void* param)
{
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr);
	while(true) {
		int sock = accept(sockfd, (struct sockaddr *)&addr, &len);
		printf("server>>accept %s:%d\r\n", inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
		Sleep(100);
	}
}


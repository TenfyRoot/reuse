// client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

int sockfd = 0;
int sockfd1 = 0;

char *serveraddr = "127.0.0.1";

unsigned long __stdcall ThreadFunc(void* param);

int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc >= 2 )
	{
		serveraddr = argv[1];
	}

	WSAData wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);

	sockfd = socket(2,1,0);
	sockfd1 = socket(2,1,0);
	bool breuse = true;
	setsockopt(sockfd,0xffff,0x0004,(const char*)&breuse,sizeof(bool));
	setsockopt(sockfd1,0xffff,0x0004,(const char*)&breuse,sizeof(bool));

	struct sockaddr_in addr;
	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = 2;
	addr.sin_addr.s_addr = inet_addr(serveraddr);
	addr.sin_port = htons(9999);

	connect(sockfd,(struct sockaddr*)&addr,sizeof(struct sockaddr));
	struct sockaddr_in addrhole;
	int len = sizeof(struct sockaddr);
	getsockname(sockfd,(struct sockaddr*)&addrhole,&len);

	printf("client>>bind %s:%d\r\n", inet_ntoa(addrhole.sin_addr),ntohs(addrhole.sin_port));
	bind(sockfd1,(struct sockaddr*)&addrhole,sizeof(struct sockaddr));
	listen(sockfd1,5);

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
		int sock = accept(sockfd1, (struct sockaddr *)&addr, &len);
		if (sock > 0) {
			printf("client>>accept %s:%d\r\n", inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
		}
		Sleep(1000);
	}
}


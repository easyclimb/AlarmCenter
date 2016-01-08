// TestSockOpt.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")


int test_sock_opt() {
	

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// , NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == s) {
		//throw L"server socket creation failed.";
		int errnono = WSAGetLastError();
		printf("%x", errnono);
		return -1;
	}
	int timeout = 0;
	int optlen = sizeof(struct timeval);
	int ret = 0;


	printf("sys info:\n");
	ret = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		printf("SO_RCVTIMEO: %d\n", timeout);
	} else {
		printf("get SO_RCVTIMEO failed\n");
		return -1;
	}

	ret = getsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		printf("SO_SNDTIMEO: %d\n", timeout);
	} else {
		printf("get SO_SNDTIMEO failed\n");
		return -1;
	}

	timeout = 2000;
	printf("user set timeout to %ds:\n", timeout);
	optlen = sizeof(struct timeval);
	ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, optlen);
	if (ret == 0) {
		printf("set SO_RCVTIMEO ok\n");
	} else {
		printf("set SO_RCVTIMEO failed\n");
		return -1;
	}

	ret = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, optlen);
	if (ret == 0) {
		printf("set SO_SNDTIMEO ok\n");
	} else {
		printf("set SO_SNDTIMEO failed\n");
		return -1;
	}
	printf("set ok\n");

	printf("sys info:\n");
	ret = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		printf("SO_RCVTIMEO: %d\n", timeout);
	} else {
		printf("get SO_RCVTIMEO failed\n");
		return -1;
	}

	ret = getsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		printf("SO_SNDTIMEO: %d\n", timeout);
	} else {
		printf("get SO_SNDTIMEO failed\n");
		return -1;
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	int	nRet;
	WSAData	wsData;
	nRet = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (nRet < 0) {
		printf("Can't load winsock.dll.\n");
		return -1;
	}

	hostent* h = gethostbyaddr()
	

	system("pause");
	return 0;
}


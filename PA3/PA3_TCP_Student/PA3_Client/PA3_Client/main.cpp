#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#include "Trace.h"
#include "TestData.h"

// Need to link with Ws2_32.lib
#pragma comment(lib,"ws2_32.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT 8888
#define DEFAULT_ACK_MSG "Client: Got the MSG."

int main(void)
{
	//--------------------------------------------------------
	// Get initialize data
	//--------------------------------------------------------
	TestData  data;
	std::list<int> &rNodeList = data.getNodeList();

	//--------------------------------------------------------
	// Print the original data
	//--------------------------------------------------------
	Trace::out("\n\nClient: Original List\n\n");
	int j(0);
	for (std::list<int>::iterator it=rNodeList.begin(); it!=rNodeList.end(); ++it)
	{
		Trace::out("    i:%d  %d\n",j++,*it);
	}

	//--------------------------------------------------------
	// Add your code and hooks here:
	//--------------------------------------------------------

	// Initialise Winsock
	WSADATA WsaDat;
	int iRecvResult;
	int iSendResult;
	
	iSendResult = WSAStartup(MAKEWORD(2, 2), &WsaDat);
	if (iSendResult != 0)
	{
		Trace::out("Winsock error - Winsock initialization failed\r\n");
		WSACleanup();
		system("PAUSE");
		return 1;
	}

	// Resolve IP address for hostname
	struct hostent *host;

	if ((host = gethostbyname("localhost")) == NULL)
	{
		Trace::out("Failed to resolve hostname.\r\n");
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	// Create our socket
	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		Trace::out("Winsock error - Socket creation Failed!\r\n");
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	// Setup our socket address structure
	SOCKADDR_IN SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
	SockAddr.sin_port = htons(DEFAULT_PORT);

	// Attempt to connect to server
	if (connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
	{
		Trace::out("Failed to establish connection with server\r\n");
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	// Send an initial buffer
	char sendBuffer[DEFAULT_BUFLEN];
	char recvBuffer[DEFAULT_BUFLEN];
	memset(sendBuffer, 0, DEFAULT_BUFLEN - 1);
	memset(recvBuffer, 0, DEFAULT_BUFLEN - 1);

	for (std::list<int>::iterator it = rNodeList.begin(); it != rNodeList.end(); ++it)
	{
		iSendResult = send(Socket, (char *)&(*it), sizeof(int), 0);
		if (iSendResult == SOCKET_ERROR)
		{
			Trace::out("Send failed with error: %d\n", WSAGetLastError());
			closesocket(Socket);
			WSACleanup();
			return 1;
		}
		Trace::out("Sent: %dB \t---\t Msg: %d\n", iSendResult, *it);
		iRecvResult = recv(Socket, recvBuffer, DEFAULT_BUFLEN, 0);
		if (iRecvResult > 0)
		{
			Trace::out("Recv: %dB \t---\t Msg: %s\n", iRecvResult, recvBuffer);
		}
		else if (iRecvResult == 0)
		{
			Trace::out("Connection closed\n");
		}
		else
		{
			Trace::out("recv failed with error: %d\n", WSAGetLastError());
		}
	}
	
	// shutdown the connection since no more data will be sent
	iSendResult = shutdown(Socket, SD_SEND);
	if (iSendResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(Socket);
		WSACleanup();
		return 1;
	}

	// Shutdown our socket
	shutdown(Socket, SD_SEND);

	// Close our socket entirely
	closesocket(Socket);

	// Cleanup Winsock
	WSACleanup();
	system("PAUSE");
	return 0;


	//--------------------------------------------------------
	// Print the sorted data
	//--------------------------------------------------------
	Trace::out("\n\nClient: Sorted List from the server\n\n");
	int k(0);
	for (std::list<int>::iterator it=rNodeList.begin(); it!=rNodeList.end(); ++it)
	{
		Trace::out("    i:%d  %d\n",k++,*it);
	}
	Trace::out("Client Done\n\n");
	system("PAUSE");

	return 0;
}
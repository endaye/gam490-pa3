#include <iostream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
		
#include "Trace.h"
#include <vector>
#include <algorithm>
#include <string>     // std::string, std::stoi

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT 8888
#define DEFAULT_ACK_MSG "Server: Got the MSG."

int main()
{
	//----------------------------------------------------
	// Add your magic here
	//----------------------------------------------------

	// Initialize Winsock
	WSADATA WsaDat;
	int iRecvResult;
	int iSendResult;

	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		Trace::out("WSA Initialzation failed.\r\n");
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	// Create our socket
	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		Trace::out("Socket creation failed.\r\n");
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	// Resolve IP address for hostname
	

	// Setup our socket address structure
	SOCKADDR_IN serverInf;
	serverInf.sin_family = AF_INET;
	serverInf.sin_addr.s_addr = INADDR_ANY;
	serverInf.sin_port = htons(DEFAULT_PORT);

	// Bind
	if (bind(Socket, (SOCKADDR*)(&serverInf), sizeof(serverInf)) == SOCKET_ERROR)
	{
		Trace::out("Unable to bind socket!\r\n");
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	// Listen
	listen(Socket, 1);

	// Server accepts a connection request from a client
	SOCKET TempSock = SOCKET_ERROR;
	while (TempSock == SOCKET_ERROR) 
	{
		Trace::out("Waiting for incoming connections...\r\n");
		TempSock = accept(Socket, NULL, NULL);
	}

	Trace::out("Client connected!\r\n\r\n");

	// No longer need server socket
	closesocket(Socket);

	// Receive until the peer shuts down the connection
	char sendBuffer[DEFAULT_BUFLEN];
	char recvBuffer[DEFAULT_BUFLEN];
	memset(sendBuffer, 0, DEFAULT_BUFLEN - 1);
	memset(recvBuffer, 0, DEFAULT_BUFLEN - 1);
	int recvInt;

	std::vector<int> v;
	do {
		iRecvResult = recv(TempSock, (char *)&recvInt, sizeof(int), 0);
		if (iRecvResult > 0) {
			v.push_back(recvInt);
			Trace::out("Recv: %dB \t---\t Msg: %d\n", iRecvResult, recvInt);
			
			// send ack message to client
			iSendResult = send(TempSock, DEFAULT_ACK_MSG, (int)strlen(DEFAULT_ACK_MSG), 0);
			if (iSendResult == SOCKET_ERROR) {
				Trace::out("Send failed with error: %d\n", WSAGetLastError());
				closesocket(TempSock);
				WSACleanup();
				system("PAUSE");
				return 1;
			}
			Trace::out("Sent: %dB \t---\t Msg: %s\n", iSendResult, DEFAULT_ACK_MSG);
		}
		else if (iRecvResult == 0)
			Trace::out("Connection closing...\n");
		else  {
			Trace::out("recv failed with error: %d\n", WSAGetLastError());
			closesocket(TempSock);
			WSACleanup();
			system("PAUSE");
			return 1;
		}

	} while (iRecvResult > 0);
	Trace::out("Received all numbers\n\n");

	// sort the list
	std::sort(v.begin(), v.end());
	Trace::out("Server: Sorted List\n");
	int j(0);
	for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it)
	{
		Trace::out("    i:%d  %d\n", j++, *it);
	}
	

	// shutdown the connection since we're done
	iRecvResult = shutdown(TempSock, SD_SEND);
	if (iRecvResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(TempSock);
		WSACleanup();
		system("PAUSE");
		return 1;
	}

	// Shutdown our socket
	shutdown(TempSock, SD_BOTH);
	shutdown(Socket, SD_BOTH);

	// Close our socket entirely
	closesocket(TempSock);
	closesocket(Socket);

	// Cleanup Winsock
	WSACleanup();
	
	//----------------------------------------------------
	// print and exit
	//----------------------------------------------------
	Trace::out("Server: Done\n\n");
	system("PAUSE");
	return 0;
}
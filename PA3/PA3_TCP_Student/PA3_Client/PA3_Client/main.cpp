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

enum Type { 
	Start = 1, 
	Stop  = 2, 
	Value = 3, 
	Ack   = 4
};

struct Packet {
	Type		type;
	int			a;		// totalNum or Rank
	int			b;		// packetSize of Data
} info;

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

	// Send a start
	Packet packet;
	packet.type = Type::Start;
	packet.a = rNodeList.size();
	packet.b = sizeof(int);

	char recvBuffer[DEFAULT_BUFLEN];
	memset(recvBuffer, 0, DEFAULT_BUFLEN - 1);

	iSendResult = send(Socket, (char *)&packet, sizeof(packet), 0);
	if (iSendResult == SOCKET_ERROR)
	{
		Trace::out("Send failed with error: %d\n", WSAGetLastError());
		closesocket(Socket);
		WSACleanup();
		system("PAUSE");
		return 1;
	}
	else
	{
		Trace::out("Sent: %dB \t---\t Type: %d TotalNum: %d PacketSize: %d\n", iSendResult, packet.type, packet.a, packet.b);
		iRecvResult = recv(Socket, recvBuffer, sizeof(recvBuffer), 0);
		if (iRecvResult > 0) {
			// parse the packet
			memcpy(&packet, recvBuffer, sizeof(&packet));
			if (packet.type == Type::Ack)
			{
				Trace::out("Recv: %dB \t---\t Type: %d\n", iRecvResult, packet.type);
			}
		}
		else if (iRecvResult == 0)
		{
			Trace::out("Connection closing...\n");
		}	
		else  
		{
			Trace::out("recv failed with error: %d\n", WSAGetLastError());
			closesocket(Socket);
			WSACleanup();
			system("PAUSE");
			return 1;
		}
	}
	
	// Send number data
	int i(0);
	std::list<int>::iterator it = rNodeList.begin();
	do 
	{
		packet.type = Type::Value;	// Header
		packet.a = i;				// Rank
		packet.b = *it;				// Value
		iSendResult = send(Socket, (char *)&packet, sizeof(packet), 0);
		if (iSendResult == SOCKET_ERROR)
		{
			Trace::out("Send failed with error: %d\n", WSAGetLastError());
			Trace::out("Sent out %d numbers\n", i);
			closesocket(Socket);
			WSACleanup();
			system("PAUSE");
			return 1;
		} 
		else
		{
			i++;
			++it;
			Trace::out("Sent: %dB \t---\t Type: %d Rank: %d Value: %d\n", iSendResult, packet.type, packet.a, packet.b);
			iRecvResult = recv(Socket, recvBuffer, sizeof(recvBuffer), 0);
			if (iRecvResult > 0) {
				// parse the packet
				memcpy(&packet, recvBuffer, sizeof(&packet));
				if (packet.type == Type::Ack)
				{
					Trace::out("Recv: %dB \t---\t Type: %d\n", iRecvResult, packet.type);
				}
			}
			else if (iRecvResult == 0)
			{
				Trace::out("Connection closing...\n");
			}
			else
			{
				Trace::out("recv failed with error: %d\n", WSAGetLastError());
				closesocket(Socket);
				WSACleanup();
				system("PAUSE");
				return 1;
			}
		}
	} while (it != rNodeList.end());

	// Send a stop 
	packet.type = Type::Stop;
	packet.a = -1;
	packet.b = -1;

	iSendResult = send(Socket, (char *)&packet, sizeof(packet), 0);
	if (iSendResult == SOCKET_ERROR)
	{
		Trace::out("Send failed with error: %d\n", WSAGetLastError());
		closesocket(Socket);
		WSACleanup();
		system("PAUSE");
		return 1;
	}
	else
	{
		Trace::out("Sent: %dB \t---\t Type: %d\n Sent a stop msg.", iSendResult, packet.type);
		iRecvResult = recv(Socket, recvBuffer, sizeof(recvBuffer), 0);
		if (iRecvResult > 0) {
			// parse the packet
			memcpy(&packet, recvBuffer, sizeof(&packet));
			if (packet.type == Type::Ack)
			{
				Trace::out("Recv: %dB \t---\t Type: %d\n", iRecvResult, packet.type);
			}
		}
		else if (iRecvResult == 0)
		{
			Trace::out("Connection closing...\n");
		}
		else
		{
			Trace::out("recv failed with error: %d\n", WSAGetLastError());
			closesocket(Socket);
			WSACleanup();
			system("PAUSE");
			return 1;
		}
	}
	
	// Receive sorted numbers
	Trace::out("Client: Receiving sorted list...\n");
	it = rNodeList.begin();
	do {
		iRecvResult = recv(Socket, recvBuffer, sizeof(recvBuffer), 0);
		if (iRecvResult > 0) 
		{
			// parse the packet
			memcpy(&packet, recvBuffer, sizeof(packet));
			if (packet.type == Type::Value) 
			{
				(*it) = packet.b;
				++it;
				Trace::out("Recv: %dB \t---\t Type: %d Rank: %d Value: %d\n", iRecvResult, packet.type, packet.a, packet.b);

				// send ack message to client
				packet.type = Type::Ack;
				packet.a = -1;
				packet.b = -1;
				iSendResult = send(Socket, (char *)&packet, sizeof(packet), 0);
				if (iSendResult == SOCKET_ERROR) {
					Trace::out("Send failed with error: %d\n", WSAGetLastError());
					closesocket(Socket);
					WSACleanup();
					system("PAUSE");
					return 1;
				}
				else
				{
					Trace::out("Sent: %dB \t---\t Type: %d\n", iSendResult, packet.type);
				}
			}
		}
		else if (iRecvResult == 0)
		{
			Trace::out("Connection closing...\n");
		}
		else  {
			Trace::out("Recv failed with error: %d\n", WSAGetLastError());
			closesocket(Socket);
			WSACleanup();
			system("PAUSE");
			return 1;
		}
	} while (iRecvResult > 0 && it != rNodeList.end());


	if (it == rNodeList.end())
	{
		Trace::out("Client: Received all sorted numbers.\n");
	}
	else 
	{
		Trace::out("Client: Received WRONG numbers!\n");
	}
	

	// shutdown the connection since no more data will be sent
	iSendResult = shutdown(Socket, SD_SEND);
	if (iSendResult == SOCKET_ERROR) {
		Trace::out("Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(Socket);
		WSACleanup();
		system("PAUSE");
		return 1;
	}
	

	// Shutdown our socket
	shutdown(Socket, SD_SEND);
	Trace::out("Shutdown socket.\n");

	// Close our socket entirely
	closesocket(Socket);
	Trace::out("Close socket.\n");

	// Cleanup Winsock
	WSACleanup();

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
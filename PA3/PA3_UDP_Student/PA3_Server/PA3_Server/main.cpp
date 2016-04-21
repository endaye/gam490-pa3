#include <iostream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
			
#include "Trace.h"
#include <vector>
#include <algorithm>

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT 8888

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

int main()
{
	//----------------------------------------------------
	// Add your magic here
	//----------------------------------------------------

	// Initialize Winsock
	WSADATA WsaDat;
	int iRecvResult, iSendResult;
	char recvBuffer[DEFAULT_BUFLEN];
	memset(recvBuffer, '\0', DEFAULT_BUFLEN - 1);

	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		Trace::out("WSA Initialzation failed.\r\n");
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	// Create our socket
	SOCKET Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (Socket == INVALID_SOCKET)
	{
		Trace::out("Socket creation failed.\r\n");
		WSACleanup();
		system("PAUSE");
		return 0;
	}
	else
	{
		Trace::out("Use UDP connection.\r\n");
	}

	// Setup our socket address structure
	SOCKADDR_IN serverInf, SockAddr;
	int slen = sizeof(SockAddr);
	serverInf.sin_family = AF_INET;
	serverInf.sin_addr.s_addr = INADDR_ANY;
	serverInf.sin_port = htons(DEFAULT_PORT);

	// Bind
	if (bind(Socket, (SOCKADDR*)(&serverInf), sizeof(serverInf)) == SOCKET_ERROR)
	{
		Trace::out("Unable to bind socket!\r\n");
		WSACleanup();
		system("PAUSE");
		return 1;
	}
	else 
	{
		Trace::out("Bind done!\r\n");
	}

	// Keep listening for data
	Trace::out("Waiting for data...\n");
	int recvInt;
	int i(0);
	int totalNum = -1;
	int packetSize = -1;
	Packet packet;
	do
	{
		// Clear the buffer by filling null
		memset(recvBuffer, '\0', DEFAULT_BUFLEN - 1);
		
		// Try to receive some data, this is a blocking call
		iRecvResult = recvfrom(Socket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)(&SockAddr), &slen);
		if (iRecvResult == SOCKET_ERROR)
		{
			Trace::out("Send failed with error: %d\n", WSAGetLastError());
			closesocket(Socket);
			WSACleanup();
			system("PAUSE");
			return 1;
		}
		else
		{
			memcpy(&packet, recvBuffer, sizeof(packet));
			if (packet.type == Type::Start)
			{
				totalNum = packet.a;
				packetSize = packet.b;
				Trace::out("Recv: %dB \t---\t Type: %d TotalNum: %d PacketSize: %d\n", iRecvResult, packet.type, totalNum, packetSize);
				//Trace::out("Start to recieve msg...\n");
			
				// Send ack message to client
				packet.type = Type::Ack;
				iSendResult = sendto(Socket, (char *)&packet, sizeof(packet), 0, (SOCKADDR*)(&SockAddr), slen);
				if (iSendResult == SOCKET_ERROR) {
					Trace::out("Send failed with error: %d\n", WSAGetLastError());
					closesocket(Socket);
					WSACleanup();
					system("PAUSE");
					return 1;
				}
				else
				{
					Trace::out("Sent: %dB \t---\t Type: %d Ack\n", iSendResult, packet.type);
					break;
				}
			}
		}
	} while (1);
	
	// Receive number data
	std::vector<int> v;
	i = 0;
	Trace::out("Start to recieve msg...\n");
	while (i < totalNum)
	{
		// Clear the buffer by filling null
		memset(recvBuffer, '\0', DEFAULT_BUFLEN - 1);

		// Try to receive some data, this is a blocking call
		iRecvResult = recvfrom(Socket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)(&SockAddr), &slen);
		if (iRecvResult == SOCKET_ERROR)
		{
			Trace::out("Send failed with error: %d\n", WSAGetLastError());
			closesocket(Socket);
			WSACleanup();
			system("PAUSE");
			return 1;
		}
		else
		{
			memcpy(&packet, recvBuffer, sizeof(packet));
			if (packet.type == Type::Value && packet.a == i)
			{
				// Send ack message to client
				packet.type = Type::Ack;
				iSendResult = sendto(Socket, (char *)&packet, sizeof(packet), 0, (SOCKADDR*)(&SockAddr), slen);
				if (iSendResult == SOCKET_ERROR) {
					Trace::out("Send failed with error: %d\n", WSAGetLastError());
					closesocket(Socket);
					WSACleanup();
					system("PAUSE");
					return 1;
				}
				else
				{
					v.push_back(packet.b);
					++i;
					Trace::out("Recv: %dB \t---\t Type: %d Rank: %d Value: %d\n", iRecvResult, packet.type, packet.a, packet.b);
					Trace::out("Sent: %dB \t---\t Type: %d Rank: &d\n", iSendResult, packet.type, packet.a);
				}
			}
		}
	}

	if (v.size() == totalNum)
	{
		Trace::out("Received all numbers. \n\n");
	}
	else
	{
		Trace::out("Received wrong numbers! \n\n");
		closesocket(Socket);
		WSACleanup();
		system("PAUSE");
		return 1;
	}

	// Sort the list
	std::sort(v.begin(), v.end());
	Trace::out("Server: Sorted List Done\n\n");

	// Print out the sorted data
	Trace::out("Server: Print sorted list\n");
	int j(0);
	for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it)
	{
		Trace::out("    i:%d  %d\n", j++, *it);
	}

	// Send back sorted data
	Trace::out("Server: Sending sorted list...\n");
	i = 0;
	std::vector<int>::iterator it = v.begin();
	do
	{
		packet.type = Type::Value;	// Header
		packet.a = i;				// Rank
		packet.b = *it;				// Value
		iSendResult = sendto(Socket, (char *)&packet, sizeof(packet), 0, (SOCKADDR*)(&SockAddr), slen);
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
			Trace::out("Sent: %dB \t---\t Type: %d Rank: %d Value: %d\n", iSendResult, packet.type, packet.a, packet.b);

			// Clear the buffer by filling null
			memset(recvBuffer, '\0', DEFAULT_BUFLEN - 1);

			// Recv ack message from server
			iRecvResult = recvfrom(Socket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)(&SockAddr), &slen);
			if (iRecvResult == SOCKET_ERROR)
			{
				memcpy(&packet, recvBuffer, sizeof(packet));
				Trace::out("Send failed with error: %d\n", WSAGetLastError());
				closesocket(Socket);
				WSACleanup();
				system("PAUSE");
				return 1;
			}
			else
			{
				memcpy(&packet, recvBuffer, sizeof(packet));
				if (packet.type == Type::Ack && packet.a == i)
				{
					Trace::out("Recv: %dB \t---\t Type: %d Rank: &d\n", iRecvResult, packet.type, packet.a);
					++i;
					++it;
				}
				else
				{
					Trace::out("Recv: %dB \t---\t Wrong Msg", iRecvResult);
				}
			}
		}
	} while (it != v.end() && i < v.size());
	/*
	if (it == v.end())
	{
		Trace::out("Server: Sent all sorted numbers.\n");
	}
	else
	{
		Trace::out("Server: Sent WRONG numbers!\n");
	}

	*/
	// Close our socket entirely
	closesocket(Socket);
	Trace::out("Close socket.\n");

	// Cleanup Winsock
	WSACleanup();

	//----------------------------------------------------
	// print and exit
	//----------------------------------------------------
	Trace::out("Server: Done\n\n");
	system("PAUSE");
	return 0;
}
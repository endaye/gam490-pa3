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

	// Receive a start
	char recvBuffer[DEFAULT_BUFLEN];
	memset(recvBuffer, 0, DEFAULT_BUFLEN - 1);

	// Receive message
	int recvInt;
	int i(0);
	int totalNum = -1;
	int packetSize = -1;
	bool isStart = 1;
	Packet packet;
	std::vector<int> v;
	do {
		iRecvResult = recv(TempSock, recvBuffer, sizeof(recvBuffer), 0);
		if (iRecvResult > 0) {
			// Parse the packet
			memcpy(&packet, recvBuffer, sizeof(packet));
			switch (packet.type)
			{
			case Type::Start:
			{
				totalNum = packet.a;
				packetSize = packet.b;
				isStart = 1;
				Trace::out("Recv: %dB \t---\t Type: %d TotalNum: %d PacketSize: %d\n", iRecvResult, packet.type, totalNum, packetSize);
				Trace::out("Start to recieve msg...\n");
				
				// Send ack message to client
				packet.type = Type::Ack;
				packet.a = -1;
				packet.b = -1;
				iSendResult = send(TempSock, (char *)&packet, sizeof(packet), 0);
				if (iSendResult == SOCKET_ERROR) {
					Trace::out("Send failed with error: %d\n", WSAGetLastError());
					closesocket(TempSock);
					closesocket(Socket);
					WSACleanup();
					system("PAUSE");
					return 1;
				}
				else
				{
					Trace::out("Sent: %dB \t---\t Type: %d\n", iSendResult, packet.type);
				}
				break;
			}
			case Type::Value:
			{
				v.push_back(packet.b);
				Trace::out("Recv: %dB \t---\t Type: %d Rank: %d Value: %d\n", iRecvResult, packet.type, packet.a, packet.b);
				
				// send ack message to client
				packet.type = Type::Ack;
				packet.a = -1;
				packet.b = -1;
				iSendResult = send(TempSock, (char *)&packet, sizeof(packet), 0);
				if (iSendResult == SOCKET_ERROR) {
					Trace::out("Send failed with error: %d\n", WSAGetLastError());
					closesocket(TempSock);
					closesocket(Socket);
					WSACleanup();
					system("PAUSE");
					return 1;
				}
				else
				{
					Trace::out("Sent: %dB \t---\t Type: %d\n", iSendResult, packet.type);
				}
				break;
			}
			case Type::Ack:
			{
				Trace::out("Recv: %dB \t---\t Type: %d\n", iRecvResult, packet.type);
				break;
			}
			case Type::Stop:
			{
				isStart = 0;
				Trace::out("Recv: %dB \t---\t Type: %d Stop recieving msg.\n", iRecvResult, packet.type);

				// send ack message to client
				packet.type = Type::Ack;
				packet.a = -1;
				packet.b = -1;
				iSendResult = send(TempSock, (char *)&packet, sizeof(packet), 0);
				if (iSendResult == SOCKET_ERROR) {
					Trace::out("Send failed with error: %d\n", WSAGetLastError());
					closesocket(TempSock);
					closesocket(Socket);
					WSACleanup();
					system("PAUSE");
					return 1;
				}
				else
				{
					Trace::out("Sent: %dB \t---\t Type: %d\n", iSendResult, packet.type);
				}
				break;
			}
			default:
			{
				Trace::out("Recv: %dB \t---\t Msg: %s\n", iRecvResult, "Wrong Msg.");
			}
			}
		}
		else if (iRecvResult == 0)
			Trace::out("Connection closing...\n");
		else  {
			Trace::out("recv failed with error: %d\n", WSAGetLastError());
			closesocket(TempSock);
			closesocket(Socket);
			WSACleanup();
			system("PAUSE");
			return 1;
		}
	} while (iRecvResult > 0 && isStart);

	if (v.size() == totalNum)
	{
		Trace::out("Received all numbers. \n\n");
	}
	else
	{
		Trace::out("Received wrong numbers! \n\n");
		closesocket(TempSock);
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
		iSendResult = send(TempSock, (char *)&packet, sizeof(packet), 0);
		if (iSendResult == SOCKET_ERROR)
		{
			Trace::out("Send failed with error: %d\n", WSAGetLastError());
			Trace::out("Sent out %d numbers\n", i);
			closesocket(TempSock);
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
			iRecvResult = recv(TempSock, recvBuffer, sizeof(recvBuffer), 0);
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
				Trace::out("Recv failed with error: %d\n", WSAGetLastError());
				closesocket(TempSock);
				closesocket(Socket);
				WSACleanup();
				system("PAUSE");
				return 1;
			}
		}
	} while (it != v.end());

	if (it == v.end())
	{
		Trace::out("Server: Sent all sorted numbers.\n");
	}
	else
	{
		Trace::out("Server: Sent WRONG numbers!\n");
	}

	// Shutdown the connection since we're done
	iRecvResult = shutdown(TempSock, SD_SEND);
	if (iRecvResult == SOCKET_ERROR) {
		Trace::out("Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(TempSock);
		closesocket(Socket);
		WSACleanup();
		system("PAUSE");
		return 1;
	}

	// Shutdown our socket
	shutdown(TempSock, SD_BOTH);
	shutdown(Socket, SD_BOTH);
	Trace::out("Shutdown socket.\n");

	// Close our socket entirely
	closesocket(TempSock);
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
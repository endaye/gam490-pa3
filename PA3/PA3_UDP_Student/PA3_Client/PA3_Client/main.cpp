#include <iostream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#include "Trace.h"
#include "TestData.h"

#define DEFAULT_BUFLEN 1024
#define DEFAULT_SERVER "localhost"
#define DEFAULT_PORT 8888

enum Type {
	Start = 1,
	Stop = 2,
	Value = 3,
	Ack = 4
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
	int iRecvResult, iSendResult;
	char recvBuffer[DEFAULT_BUFLEN];
	memset(recvBuffer, '\0', DEFAULT_BUFLEN - 1);


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

	if ((host = gethostbyname(DEFAULT_SERVER)) == NULL)
	{
		Trace::out("Failed to resolve hostname.\r\n");
		WSACleanup();
		system("PAUSE");
		return 1;
	}

	// Create our socket
	SOCKET Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (Socket == INVALID_SOCKET)
	{
		Trace::out("Winsock error - Socket creation Failed!\r\n");
		WSACleanup();
		system("PAUSE");
		return 1;
	}
	else
	{
		Trace::out("Use UDP connection.\r\n");
	}

	// Setup our socket address structure
	SOCKADDR_IN SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
	SockAddr.sin_port = htons(DEFAULT_PORT);
	int slen = sizeof(SockAddr);

	// Send a start
	Packet packet;
	
	do 
	{
		packet.type = Type::Start;
		packet.a = rNodeList.size();
		packet.b = sizeof(int);
		iSendResult = sendto(Socket, (char *)&packet, sizeof(packet), 0, (SOCKADDR*)(&SockAddr), slen);
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
				if (packet.type == Type::Ack)
				{
					Trace::out("Recv: %dB \t---\t Type: %d Ack\n", iRecvResult, packet.type);
					break;
				}
				else
				{
					Trace::out("Recv: %dB \t---\t Wrong Msg", iRecvResult);
				}
				
			}
		}
	} while (1);
	
	// Send number data
	int i(0);
	std::list<int>::iterator it = rNodeList.begin();
	do
	{
		packet.type = Type::Value;	// Header
		packet.a = i;				// Rank
		packet.b = *it;				// Value
		//iSendResult = send(Socket, (char *)&packet, sizeof(packet), 0);
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
	} while (it != rNodeList.end() && i < rNodeList.size());

	
	// Receive sorted numbers
	Trace::out("Client: Receiving sorted list...\n");
	it = rNodeList.begin();
	i = 0;
	while (it != rNodeList.end() && i < rNodeList.size()) 
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
					(*it) = packet.b;
					++it;
					++i;
					Trace::out("Recv: %dB \t---\t Type: %d Rank: %d Value: %d\n", iRecvResult, packet.type, packet.a, packet.b);
					Trace::out("Sent: %dB \t---\t Type: %d Rank: &d\n", iSendResult, packet.type, packet.a);
				}
			}
		}
	}

	if (it == rNodeList.end())
	{
		Trace::out("Client: Received all sorted numbers.\n");
	}
	else
	{
		Trace::out("Client: Received WRONG numbers!\n");
	}
	
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
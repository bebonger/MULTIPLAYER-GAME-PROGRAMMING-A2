#include "IOMultiplexClient.h"
#include <string>

IOMultiplexClient::IOMultiplexClient()
{
}

IOMultiplexClient::~IOMultiplexClient()
{
}

int IOMultiplexClient::startup_client(int argc, char** argv)
{
	printf("Destination IP Address [%s], Port number [%d]\n", IPAddress, Port);

	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		printf("WSAStartup() error!");
		return 1;
	}

	ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == ConnectSocket)
	{
		printf("socket() error");
		return 1;
	}

	///----------------------
	/// The sockaddr_in structure specifies the address family,
	/// IP address, and port of the server to be connected to.
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = inet_addr(IPAddress);

	///----------------------
	/// Connect to server.
	Return = connect(ConnectSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
	if (Return == SOCKET_ERROR)
	{
		closesocket(ConnectSocket);
		printf("Unable to connect to server: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	FD_ZERO(&ReadFds);
	FD_SET(ConnectSocket, &ReadFds);


	printf("enter messages : ");
	memset(Message, '\0', BUFSIZE);
	MessageLen = 0;
}

int IOMultiplexClient::run_client()
{
	while (1)
	{
		if (_kbhit())
		{ // To check keyboard input.
			Message[MessageLen] = _getch();
			if (('\n' == Message[MessageLen]) || ('\r' == Message[MessageLen]))
			{ // Send the message to server.
				putchar('\n');
				MessageLen++;
				Message[MessageLen] = '\0';

				Return = send(ConnectSocket, Message, MessageLen, 0);
				if (Return == SOCKET_ERROR)
				{
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				printf("Bytes Sent: %ld\n", Return);

				MessageLen = 0;
				memset(Message, '\0', BUFSIZE);
			}
			else
			{
				putchar(Message[MessageLen]);
				MessageLen++;
			}
		}
		else
		{
			TempFds = ReadFds;
			Timeout.tv_sec = 0;
			Timeout.tv_usec = 1000;

			if (SOCKET_ERROR == (Return = select(0, &TempFds, 0, 0, &Timeout)))
			{ // Select() function returned error.
				closesocket(ConnectSocket);
				printf("select() error\n");
				return 1;
			}
			else if (0 > Return)
			{
				printf("Select returned error!\n");
			}
			else if (0 < Return)
			{
				memset(Message, '\0', BUFSIZE);
				printf("Select Processed... Something to read\n");
				Return = recv(ConnectSocket, Message, BUFSIZE, 0);
				if (0 > Return)
				{ // recv() function returned error.
					closesocket(ConnectSocket);
					// printf("Exceptional error :Socket Handle [%d]\n", ConnectSocket);

					printf(RED, "Exceptional error :Socket Handle [%d]\n", ConnectSocket);
					return 1;
				}
				else if (0 == Return)
				{ // Connection closed message has arrived.
					closesocket(ConnectSocket);
					printf(RED, "Connection closed :Socket Handle [%d]\n", ConnectSocket);
					return 0;
				}
				else
				{ // Message received.
					printf("Bytes received   : %d\n", Return);
					printf("Message received : %s\n", Message);
					printf("enter messages : ");
				}
			}
		}
	}

	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}

int IOMultiplexClient::packet_add_data(const char DataName[], const int Value)
{
	// return PacketManager::packet_add_data(PacketBuffer, DataName, Value);
	return 0;
}

int IOMultiplexClient::packet_add_data(const char DataName[], const char Value[])
{
	// return PacketManager::packet_add_data(PacketBuffer, DataName, Value);
	return 0;
}
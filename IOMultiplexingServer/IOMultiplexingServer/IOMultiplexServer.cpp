#include "IOMultiplexServer.h"

IOMultiplexServer::IOMultiplexServer()
{
}

IOMultiplexServer::~IOMultiplexServer()
{
}

void IOMultiplexServer::send_welcome_message(SOCKET ClientSocket)
{
	char WelcomeMessage[100];
	int WelcomeMessageLength;

	sprintf_s(WelcomeMessage, "<Welcome to my I/O multiplexing server! Your Session ID is %d>", ClientSocket);
	WelcomeMessageLength = strlen(WelcomeMessage);

	send(ClientSocket, WelcomeMessage, WelcomeMessageLength, 0);
}

void IOMultiplexServer::session_info_message(fd_set ReadFds, SOCKET ClientSocket)
{
	int i;
	char InfoMessage[100];
	int InfoMessageLength;

	for (i = 1; i < ReadFds.fd_count; ++i)
	{
		if (ClientSocket == ReadFds.fd_array[i])
		{
			continue;
		}

		sprintf_s(InfoMessage, "<Already connected client with session ID %d>", ReadFds.fd_array[i]);
		InfoMessageLength = strlen(InfoMessage);

		send(ClientSocket, InfoMessage, InfoMessageLength, 0);
	}
}

void IOMultiplexServer::send_notice_message(fd_set ReadFds, SOCKET ClientSocket)
{
	int i;
	char InfoMessage[100];
	int InfoMessageLength;

	for (i = 1; i < ReadFds.fd_count; ++i)
	{
		if (ClientSocket == ReadFds.fd_array[i])
		{
			continue;
		}

		sprintf_s(InfoMessage, "<New client has connected. Session ID is %d>", ClientSocket);
		InfoMessageLength = strlen(InfoMessage);

		send(ReadFds.fd_array[i], InfoMessage, InfoMessageLength, 0);
	}
}

void IOMultiplexServer::whisper_to_one(fd_set ReadFds, char Message[], int MessageLength, SOCKET ClientSocket)
{
	int MsgPos;
	int FD_Index;
	char TargetSocket[BUFSIZ];
	char FailMessage[] = "<Fail to send a message>";
	int FailMessageLength = strlen(FailMessage);
	char SuccessMessage[BUFSIZE];
	int SuccessMessageLength;
	char WhisperMessage[BUFSIZE];
	int WhisperMessageLength;

	for (MsgPos = 1; MsgPos < MessageLength; ++MsgPos)
	{
		if (' ' == Message[MsgPos])
		{
			TargetSocket[MsgPos - 1] = '\0';
			break;
		}
		TargetSocket[MsgPos - 1] = Message[MsgPos];
	}
	if (MsgPos == MessageLength)
	{
		send(ClientSocket, FailMessage, FailMessageLength, 0);
		return;
	}

	for (FD_Index = 1; FD_Index < ReadFds.fd_count; ++FD_Index)
	{
		if (ReadFds.fd_array[FD_Index] == atoi(TargetSocket))
		{
			sprintf_s(WhisperMessage, "<Sender %d sent:%s>", ClientSocket, &Message[MsgPos + 1]);
			WhisperMessageLength = strlen(WhisperMessage);
			send(ReadFds.fd_array[FD_Index], WhisperMessage, WhisperMessageLength, 0);

			sprintf_s(SuccessMessage, "<Message send to %s>", TargetSocket);
			SuccessMessageLength = strlen(SuccessMessage);
			send(ClientSocket, SuccessMessage, SuccessMessageLength, 0);

			break;
		}
	}
	if (FD_Index == ReadFds.fd_count)
	{
		send(ClientSocket, FailMessage, FailMessageLength, 0);
	}
}

void IOMultiplexServer::send_to_all(fd_set ReadFds, char Message[], int MessageLength)
{
	int i;

	for (i = 1; i < ReadFds.fd_count; ++i)
	{
		send(ReadFds.fd_array[i], Message, MessageLength, 0);
	}
}

int IOMultiplexServer::startup_server(int argc, char** argv)
{
	if (2 == argc)
	{
		Port = atoi(argv[1]);
	}
	printf("Using port number : [%d]\n", Port);

	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		printf("WSAStartup() error!\n");
		return 1;
	}

	ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == ServerSocket)
	{
		printf("socket() error\n");
		return 1;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(ServerSocket, (SOCKADDR*)&ServerAddr,
		sizeof(ServerAddr)))
	{
		printf("bind() error\n");
		return 1;
	}

	if (SOCKET_ERROR == listen(ServerSocket, 5))
	{
		printf("listen() error\n");
		return 1;
	}

	FD_ZERO(&ReadFds);
	FD_SET(ServerSocket, &ReadFds);
}

int IOMultiplexServer::run_server()
{
	while (1)
	{
		TempFds = ReadFds;
		Timeout.tv_sec = 5;
		Timeout.tv_usec = 0;

		if (SOCKET_ERROR == (Return = select(0, &TempFds, 0, 0, &Timeout)))
		{ // Select() function returned error.
			printf("select() error\n");
			return 1;
		}
		if (0 == Return)
		{ // Select() function returned by timeout.
			printf("Select returned timeout.\n");
		}
		else if (0 > Return)
		{
			printf("Select returned error!\n");
		}
		else
		{
			for (Index = 0; Index < TempFds.fd_count; Index++)
			{
				if (TempFds.fd_array[Index] == ServerSocket)
				{ // New connection requested by new client.
					ClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientAddr, &ClientLen);
					FD_SET(ClientSocket, &ReadFds);
					printf("New Client Accepted : Socket Handle [%d]\n", ClientSocket);

					send_welcome_message(ClientSocket);
					session_info_message(ReadFds, ClientSocket);
					send_notice_message(ReadFds, ClientSocket);
				}
				else
				{ // Something to read from socket.
					memset(Message, '\0', BUFSIZE);
					Return = recv(TempFds.fd_array[Index], Message, BUFSIZE, 0);
					if (0 == Return)
					{ // Connection closed message has arrived.
						closesocket(TempFds.fd_array[Index]);
						printf("Connection closed :Socket Handle [%d]\n", TempFds.fd_array[Index]);
						FD_CLR(TempFds.fd_array[Index], &ReadFds);
					}
					else if (0 > Return)
					{ // recv() function returned error.
						closesocket(TempFds.fd_array[Index]);
						printf("Exceptional error :Socket Handle [%d]\n", TempFds.fd_array[Index]);
						FD_CLR(TempFds.fd_array[Index], &ReadFds);
					}
					else
					{ // Message recevied.
						if ('/' == Message[0])
						{
							whisper_to_one(ReadFds, Message, Return, TempFds.fd_array[Index]);
						}
						else
						{
							send_to_all(ReadFds, Message, Return);
						}
					}
				}
			}
		}
	}

	WSACleanup();

	return 0;
}

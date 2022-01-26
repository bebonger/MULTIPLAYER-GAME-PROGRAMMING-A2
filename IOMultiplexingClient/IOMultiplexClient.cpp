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
	hasBroadcasted = false;
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

	// Set Username
	printf("Enter Username : ");
	// memset(ClientID, '\0', 256);
	// MessageLen = 0;

	// std::cin >> ClientID;
	memset(ClientID, '\0', 256);
	memset(Message, '\0', BUFSIZE);
	MessageLen = 0;
}

int IOMultiplexClient::run_client()
{
	while (1)
	{
		if (_kbhit())
		{ // To check keyboard input.
			char character = _getch();
			Message[MessageLen] = character;
			if (('\n' == Message[MessageLen]) || ('\r' == Message[MessageLen]))
			{ // Send the message to server.
				putchar('\n');
				MessageLen++;
				Message[MessageLen] = '\0';

				// tell the server to create a user
				if (ClientID[0] == '\0') {
					Message[MessageLen - 1] = '\0';
					strcpy_s(ClientID, Message);
					Packet MSGTYPE_Packet;
					MSGTYPE_Packet << "MSGTYPE=SETUSER";
					MSGTYPE_Packet.Encode(ClientID);
					Return = send(ConnectSocket, MSGTYPE_Packet.PacketData, MSGTYPE_Packet.PacketLength, 0);

					system("cls");

					//printf("enter messages : ");

				}	
				else 
					Return = send_msg(Message);

				if (Return == SOCKET_ERROR)
				{
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				// printf("Bytes Sent: %ld\n", Return);

				MessageLen = 0;
				memset(Message, '\0', BUFSIZE);
			}
			else if (Message[MessageLen] == '\b')
			{
				putchar(Message[MessageLen]);
				Message[MessageLen] = '\0';
				if (MessageLen != 0) {
					Message[MessageLen - 1] = '\0';
					MessageLen--;
				}
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
				memset(ReadBuffer, '\0', BUFSIZE);
				// printf("Select Processed... Something to read\n");
				Return = recv(ConnectSocket, ReadBuffer, BUFSIZE, 0);
				if (0 > Return)
				{ // recv() function returned error.
					closesocket(ConnectSocket);
					// printf("Exceptional error :Socket Handle [%d]\n", ConnectSocket);

					printf("%sExceptional error :Socket Handle [%d]%s\n", RED, ConnectSocket, RESET);
					return 1;
				}
				else if (0 == Return)
				{ // Connection closed message has arrived.
					closesocket(ConnectSocket);
					printf("%sConnection closed :Socket Handle [%d]%s\n", RED, ConnectSocket, RESET);
					return 0;
				}
				else
				{ 
					process_incoming_messages();
				}
			}
		}
	}

	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}


int IOMultiplexClient::send_msg(std::string _msg)
{
 	Packet packet;
	
	if (_msg[0] == '\r')
		return 1;

	// Is msg a command
	if (_msg[0] == '!')
	{
		_msg.pop_back();
		if (_msg == "!help") {
			packet << "MSGTYPE=CMD_HELP";
		}
		else if (_msg.find("broadcast ", 1) == 1) {
		packet << "MSGTYPE=CMD_BROADCAST";
			std::string msgBuffer;
			for (int i = 11; i < _msg.length(); ++i) {
				msgBuffer.push_back(_msg[i]);
			}

			packet << ("MESSAGE=\"" + msgBuffer + "\"").c_str();
		}

		if (CurrentRoomName.empty()) {
			if (_msg == "!rooms") {
				packet << "MSGTYPE=CMD_ROOMLIST";
			}
			else if (_msg.find("create ", 1) == 1) {
				packet << "MSGTYPE=CMD_CREATE_ROOM";
				std::string roomName;
				for (int i = 8; i < _msg.length(); ++i) {
					if (_msg[i] == ' ' || _msg[i] == '\r' || _msg[i] == '\n')
						break;

					roomName.push_back(_msg[i]);
				}

				if (roomName.empty()) {
					printf("Empty Room Name\n");
					return 0;
				}

				packet << ("ROOM_NAME=" + roomName).c_str();
				CurrentRoomName = roomName;
				system("cls");
				printf("<Created room \"%s\">\nStart Chatting!\n", CurrentRoomName);


			}
			else if (_msg.find("join ", 1) == 1) {
				packet << "MSGTYPE=CMD_JOIN_ROOM";
				std::string roomName;

				for (int i = 6; i < _msg.length(); ++i) {
					if (_msg[i] == ' ' || _msg[i] == '\r' || _msg[i] == '\n')
						break;

					roomName.push_back(_msg[i]);
				}
				packet << ("ROOM_NAME=" + roomName).c_str();
				printf("<Attempting to join room \"%s\">\n", roomName.c_str());
			}
		}
		else {
			if (_msg == "!leave") {
				packet << "MSGTYPE=CMD_LEAVE_ROOM";
				CurrentRoomName = "";
				system("cls");
			}
			else if (_msg == "!users") {
				packet << "MSGTYPE=CMD_USER_LIST";
			}
			else if (_msg.find("whisper ", 1) == 1) {
				packet << "MSGTYPE=CMD_WHISPER";
				std::string ReceiverSocketID;
				int socketIDLength = 0;

				for (int i = 9; i < _msg.length(); ++i) {
					if (_msg[i] == ' ' || _msg[i] == '\r' || _msg[i] == '\n')
						break;

					socketIDLength++;
					ReceiverSocketID.push_back(_msg[i]);
				}
				packet << ("RECEIVER=" + ReceiverSocketID).c_str();

				std::string msgBuffer;
				for (int i = 10 + socketIDLength; i < _msg.length(); ++i) {
					msgBuffer.push_back(_msg[i]);
				}

				packet << ("MESSAGE=\"" + msgBuffer + "\"").c_str();
			}
		}
	}
	else {
		// Message
		packet << "MSGTYPE=MESSAGE";
		packet << ("MSG=\"" + _msg + "\"").c_str();
	}
	
	packet.Encode(ClientID);
	return send(ConnectSocket, packet.PacketData,packet.PacketLength, 0);
}

int IOMultiplexClient::send_packet(Packet _packet)
{
	_packet.Encode(ClientID);
	return send(ConnectSocket, _packet.PacketData, strlen(_packet.PacketData), 0);
}

void IOMultiplexClient::process_incoming_messages()
{
	// char DisplayBuffer[BUFSIZE] = { '\0' };
	if (PacketManager::decodable(ReadBuffer)) {
		Packet packet;
		strcpy_s(packet.PacketData, ReadBuffer);
		packet.Decode();

		// Retrieve the message type/command
		char MSGTYPE[BUFSIZE] = { '\0' };
		PacketManager::packet_parse_data(packet.PacketData, "MSGTYPE", MSGTYPE, packet.PacketLength);

		if (strcmp(MSGTYPE, "JOINED_ROOM") == 0)
		{
			char RoomName[BUFSIZE] = { '\0' };
			PacketManager::packet_parse_data(packet.PacketData, "ROOM_NAME", RoomName, packet.PacketLength);
			CurrentRoomName = RoomName;
			system("cls");
		}
	}
	else 
		printf("%s\n", ReadBuffer);
}
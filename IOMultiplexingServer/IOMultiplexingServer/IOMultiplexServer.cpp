#include "IOMultiplexServer.h"
#include "packet_manager.h"
#include <time.h>
#include "color.h"

IOMultiplexServer::IOMultiplexServer()
{
}

IOMultiplexServer::~IOMultiplexServer()
{
	for (int i = 0; i < RoomList.size(); ++i) {
		if (RoomList[i]) {
			delete RoomList[i];
			RoomList[i] = nullptr;
		}
	}

	for (std::map<SOCKET, UserInfo*>::iterator it = UserList.begin(); it != UserList.end(); ++it) {
		if (it->second) {
			delete (it->second);
			it->second = nullptr;
		}
	}
		
}

void IOMultiplexServer::send_welcome_message(SOCKET ClientSocket)
{
	char WelcomeMessage[100];
	int WelcomeMessageLength;

	sprintf_s(WelcomeMessage, "[Welcome %s! Your Session ID is %d]\nType !help for the available commands.\n", UserList[ClientSocket]->UserID, ClientSocket);
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

		sprintf_s(InfoMessage, "[Already connected client with session ID %d]\n", ReadFds.fd_array[i]);
		InfoMessageLength = strlen(InfoMessage);

		// send(ClientSocket, InfoMessage, InfoMessageLength, 0);
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

		sprintf_s(InfoMessage, "[New client has connected. Session ID is %d]", ClientSocket);
		// UserList.insert(std::pair<SOCKET, const char*>(ClientSocket, "ID"));
		// InfoMessageLength = strlen(InfoMessage);
		// send(ReadFds.fd_array[i], InfoMessage, InfoMessageLength, 0);
	}
}

void IOMultiplexServer::whisper_to_one(SOCKET SenderSocket, SOCKET ReceiverSocket, char Message[], int MessageLength)
{
	char FailMessage[BUFSIZE];
	sprintf(FailMessage, "%s [Fail to send a message] %s", BOLDRED, RESET);
	int FailMessageLength = strlen(FailMessage);
	char WhisperMessage[BUFSIZE];
	char SuccessMessage[BUFSIZE];

	time_t rawtime = time(NULL);
	struct tm* ptm = localtime(&rawtime);
	sprintf(WhisperMessage, "%s [%02d:%02d:%02d] %s whispers to you: %s %s", BOLDMAGENTA, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, UserList[SenderSocket]->UserID, Message, RESET);
	
	if (ReceiverSocket == NULL || UserList[SenderSocket]->currentRoom != UserList[ReceiverSocket]->currentRoom || 0 > send(ReceiverSocket, WhisperMessage, strlen(WhisperMessage), 0))
		send(SenderSocket, FailMessage, FailMessageLength, 0);
	else {
		sprintf(SuccessMessage, "%s [%02d:%02d:%02d] you whisper to %s: %s %s", BOLDMAGENTA, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, UserList[ReceiverSocket]->UserID, Message, RESET);
		send(SenderSocket, SuccessMessage, strlen(SuccessMessage), 0);
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

void IOMultiplexServer::send_to_room(RoomInfo* room, char Message[], int MessageLength)
{
	int i;

	for (i = 0; i < room->UserList.size(); ++i)
	{
		auto it = room->UserList.begin();
		std::advance(it, i);
		send(it->first, Message, MessageLength, 0);
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

					// send_welcome_message(ClientSocket);
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
						
						UserList.erase(TempFds.fd_array[Index]);
						// RoomList[UserRoomConnection[TempFds.fd_array[Index]]].UserList.erase(TempFds.fd_array[Index]);
					}
					else if (0 > Return)
					{ // recv() function returned error.
						closesocket(TempFds.fd_array[Index]);
						printf("Exceptional error :Socket Handle [%d]\n", TempFds.fd_array[Index]);
						FD_CLR(TempFds.fd_array[Index], &ReadFds);
						
						UserList.erase(TempFds.fd_array[Index]);
						// RoomList[UserRoomConnection[TempFds.fd_array[Index]]].UserList.erase(TempFds.fd_array[Index]);
					}
					else
					{ 
						process_incoming_messages(TempFds.fd_array[Index]);
					}
				}
			}
		}
	}

	WSACleanup();

	return 0;
}


void IOMultiplexServer::process_incoming_messages(SOCKET ClientSocket)
{
	// Retrieve the decoded packet from the message
	char RetrieveBuffer[BUFSIZE] = { '\0' };
	char ClientID[BUFSIZE] = { '\0' };
	int PacketLength = PacketManager::packet_decode(Message, ClientID, RetrieveBuffer);

	// Retrieve the message type/command
	char MSGTYPE[BUFSIZE] = { '\0' };
	PacketManager::packet_parse_data(RetrieveBuffer, "MSGTYPE", MSGTYPE, PacketLength);

	if (strcmp(MSGTYPE, "SETUSER") == 0) {
		UserList.insert(std::pair<SOCKET, UserInfo*>(ClientSocket, new UserInfo(ClientSocket, ClientID)));
		send_welcome_message(ClientSocket);
	}
	else if (strcmp(MSGTYPE, "MESSAGE") == 0) {

		if (!UserList[ClientSocket]->currentRoom)
			return;

		// retrieve the message data
		char MSGBuffer[BUFSIZE] = { '\0' };
		int MessageLen = PacketManager::packet_parse_data(RetrieveBuffer, "MESSAGE", MSGBuffer, PacketLength);

		// [TIMESTAMP] User: MSG
		time_t rawtime = time(NULL);
		struct tm* ptm = localtime(&rawtime);
		char SendBuffer[BUFSIZE] = { '\0' };
		sprintf(SendBuffer, "%s [%02d:%02d:%02d] %s: %s %s", BOLDWHITE, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, ClientID, MSGBuffer, RESET);
		send_to_room(UserList[ClientSocket]->currentRoom, SendBuffer, Return);
	}
	else if (strcmp(MSGTYPE, "CMD_HELP") == 0) {
		send_help_msg(ClientSocket);
	}
	else if (!UserList[ClientSocket]->currentRoom) {
		if (strcmp(MSGTYPE, "CMD_ROOMLIST") == 0) {
			send_room_list(ClientSocket);
		}
		else if (strcmp(MSGTYPE, "CMD_CREATE_ROOM") == 0) {
			char MSGBuffer[BUFSIZE] = { '\0' };
			int MessageLen = PacketManager::packet_parse_data(RetrieveBuffer, "ROOM_NAME", MSGBuffer, PacketLength);

			if (!room_exists(MSGBuffer)) {
				RoomInfo* room = new RoomInfo(new UserInfo(ClientSocket, UserList[ClientSocket]->UserID), MSGBuffer);
				RoomList.push_back(room);
				join_room(ClientSocket, room);
			}
			else {
				char SendBuffer[BUFSIZE] = { '\0' };
				sprintf(SendBuffer, "[Failed to create \"%s\", room already exists]", MSGBuffer);
				send(ClientSocket, SendBuffer, strlen(SendBuffer), 0);
			}
		}
		else if (strcmp(MSGTYPE, "CMD_JOIN_ROOM") == 0) {
			char MSGBuffer[BUFSIZE] = { '\0' };
			int MessageLen = PacketManager::packet_parse_data(RetrieveBuffer, "ROOM_NAME", MSGBuffer, PacketLength);

			if (!attempt_join_room(ClientSocket, MSGBuffer)) {
				char SendBuffer[BUFSIZE] = { '\0' };
				sprintf(SendBuffer, "[Failed to join \"%s\", room does not exist]", MSGBuffer);
				send(ClientSocket, SendBuffer, strlen(SendBuffer), 0);
			}
		}
	}
	else if (UserList[ClientSocket]->currentRoom) {
		if (strcmp(MSGTYPE, "CMD_LEAVE_ROOM") == 0) {
			leave_room(ClientSocket);
		}
		else if (strcmp(MSGTYPE, "CMD_USER_LIST") == 0) {
			send_user_list(ClientSocket);
		}
		else if (strcmp(MSGTYPE, "CMD_WHISPER") == 0) {
			char ReceiverSocket[BUFSIZ] = { '\0' };
			char SendBuffer[BUFSIZE] = { '\0' };
			PacketManager::packet_parse_data(RetrieveBuffer, "RECEIVER", ReceiverSocket, PacketLength);
			PacketManager::packet_parse_data(RetrieveBuffer, "MESSAGE", SendBuffer, PacketLength);

			// sprintf(SendBuffer, "[Failed to join \"%s\", room does not exist]", );

			whisper_to_one(ClientSocket, find_user_by_id(ReceiverSocket), SendBuffer, strlen(SendBuffer));
		}
	}
}

void IOMultiplexServer::send_user_list(SOCKET ClientSocket) {
	char userListMSG[BUFSIZE] = "\nUsers in current room\n===========\n";
	for (std::map<SOCKET, UserInfo*>::iterator it = UserList[ClientSocket]->currentRoom->UserList.begin(); it != UserList[ClientSocket]->currentRoom->UserList.end(); ++it) {
		char formattedStr[BUFSIZE] = { '\0' };
		sprintf(formattedStr, "%d: %s\n", ClientSocket, it->second->UserID);
		strcat(userListMSG, formattedStr);
	}
	send(ClientSocket, userListMSG, strlen(userListMSG), 0);
}

void IOMultiplexServer::send_room_list(SOCKET ClientSocket)
{
	char roomListMSG[BUFSIZE] = "\nRoom List\n===========\n";
	for (int i = 0; i < RoomList.size(); ++i) {
		sprintf(roomListMSG, "%s %d: %s\n", roomListMSG, i, RoomList[i]->RoomName);
	}
	send(ClientSocket, roomListMSG, strlen(roomListMSG), 0);
}

void IOMultiplexServer::send_help_msg(SOCKET ClientSocket)
{
	char helpMSG[BUFSIZE] = "\nCommands\n===========\n";
	strcat(helpMSG, "!help - sends this message\n");
	if (UserList[ClientSocket]->currentRoom) {
		strcat(helpMSG, "!leave - leaves the current room.\n");
		strcat(helpMSG, "!whisper [User] [MSG] - sends a private message to another user in the room.\n");
		strcat(helpMSG, "!users - shows a list of the users in the current room.\n");
		strcat(helpMSG, "!broadcast [MSG] - broadcasts a message to users in all rooms. Can only be done once per client lifetime.\n");
		if (UserList[ClientSocket]->currentRoom->IsUserModerator(ClientSocket))
		{
			strcat(helpMSG, "\nModerator Commands\n================\n");
			strcat(helpMSG, "!kick [User] - kicks a user from the current room.\n");
			strcat(helpMSG, "!ban [User] - bans a user from the current room.\n");
			strcat(helpMSG, "!mute [User] - prevents a user from chatting in the current room\n");
			strcat(helpMSG, "!unmute [User] - allows a previously muted user to continue chatting in the current room.\n");
		}
	}
	else {
		strcat(helpMSG, "!rooms - shows the available rooms.\n");
		strcat(helpMSG, "!create [RoomName] - creates a room with the given name.\n");
		strcat(helpMSG, "!join [RoomName] - joins a room with the given name.\n");
	}
	send(ClientSocket, helpMSG, strlen(helpMSG), 0);
}

bool IOMultiplexServer::room_exists(char RoomName[])
{
	for (RoomInfo* room : RoomList) {
		if (room->RoomName == RoomName)
			return true;
	}

	return false;
}

void IOMultiplexServer::join_room(SOCKET ClientSocket, RoomInfo* room)
{
	Packet packet;
	packet << "MSGTYPE=JOINED_ROOM";
	char roomName[256] = "ROOM_NAME=";
	strcat(roomName, room->RoomName);
	packet << roomName;
	packet.Encode("Server");
	send(ClientSocket, packet.PacketData, packet.PacketLength, 0);

	room->UserList[ClientSocket] = UserList[ClientSocket];
	UserList[ClientSocket]->currentRoom = room;

	char SendBuffer[BUFSIZE] = { '\0' };
	sprintf(SendBuffer, "%s [\"%s\" has joined the room!] %s", BOLDYELLOW, UserList[ClientSocket]->UserID, RESET);
	send_to_room(room, SendBuffer, strlen(SendBuffer));
}

bool IOMultiplexServer::attempt_join_room(SOCKET ClientSocket, char RoomName[])
{
	for (RoomInfo* room : RoomList) {
		if (strcmp(room->RoomName, RoomName) == 0) {
			join_room(ClientSocket, room);
			return true;
		}
	}

	return false;
}

void IOMultiplexServer::leave_room(SOCKET ClientSocket) {
	
	RoomInfo* room = UserList[ClientSocket]->currentRoom;

	UserList[ClientSocket]->currentRoom->UserList.erase(ClientSocket);
	UserList[ClientSocket]->currentRoom = nullptr;

	char SendBuffer[BUFSIZE] = { '\0' };
	sprintf(SendBuffer, "%s [\"%s\" has left the room!] %s", BOLDBLUE, UserList[ClientSocket]->UserID, RESET);
	send_to_room(room, SendBuffer, strlen(SendBuffer));

	send_welcome_message(ClientSocket);
}

SOCKET IOMultiplexServer::find_user_by_id(char id[]) {
	for (std::map<SOCKET, UserInfo*>::iterator it = UserList.begin(); it != UserList.end(); ++it) {
		if (strcmp(it->second->UserID, id) == 0) {
			return it->first;
		}
	}

	return NULL;
}

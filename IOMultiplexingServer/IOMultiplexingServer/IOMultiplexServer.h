#pragma once
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <map>
#include <vector>

/// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define BUFSIZE     1024
#define PORT_NUMBER 7890

// User Commands
// ===============
// !whisper
// !user

// Moderator Commands
// ===================
// !kick
// !ban
// !broadcast
// !mute

#pragma warning(disable:4996)

struct RoomInfo;
struct UserInfo;

struct UserInfo {
	SOCKET		ClientSocket;
	char        UserID[256];

	RoomInfo*   currentRoom;

	UserInfo(SOCKET clientSocket, const char* userID)
		: ClientSocket(clientSocket)
		, UserID{ '\0' }
		, currentRoom(NULL) {
		strcpy(UserID, userID);
	}
};

struct RoomInfo {
	char RoomName[256];

	std::map<SOCKET, UserInfo*>	UserList; // Map Client Sockets to their client IDs for easy access
	std::vector<UserInfo*>		ModeratorList;
	std::vector<UserInfo*>		Blacklist; // Blacklist for banned users
	std::vector<UserInfo*>		MuteList;

	RoomInfo(UserInfo* _creatorSocket, const char* _roomName)
		: RoomName{ '\0' } 
	{ 
		strcpy_s(RoomName, _roomName);
		UserList[_creatorSocket->ClientSocket] = _creatorSocket;
		ModeratorList.push_back(_creatorSocket);
	}
	~RoomInfo() {
		for (int i = 0; i < UserList.size(); ++i) {
			UserList[i] = nullptr;
		}

		for (int i = 0; i < Blacklist.size(); ++i) {
			Blacklist[i] = nullptr;
		}

		for (int i = 0; i < ModeratorList.size(); ++i) {
			ModeratorList[i] = nullptr;
		}

		for (int i = 0; i < MuteList.size(); ++i) {
			MuteList[i] = nullptr;
		}
	}

	bool IsUserModerator(SOCKET ClientSocket) 
	{
		for (UserInfo* info : ModeratorList) {
			if (info->ClientSocket == ClientSocket)
				return true;
		}

		return false;
	}
};

class IOMultiplexServer {
private:
	int          Port = PORT_NUMBER;
	WSADATA      WsaData;
	SOCKET       ServerSocket;
	SOCKADDR_IN  ServerAddr;

	unsigned int Index;
	int          ClientLen = sizeof(SOCKADDR_IN);
	SOCKET       ClientSocket;
	SOCKADDR_IN  ClientAddr;

	fd_set       ReadFds, TempFds;
	TIMEVAL      Timeout; // struct timeval timeout;

	char         Message[BUFSIZE];
	int          Return;

	std::vector<RoomInfo*>			RoomList;
	std::map<SOCKET, UserInfo*>	    UserList;

public:

	IOMultiplexServer();
	~IOMultiplexServer();

	SOCKET find_user_by_id(char id[]);
	void process_incoming_messages(SOCKET ClientSocket);

	void send_welcome_message(SOCKET ClientSocket);
	void session_info_message(fd_set ReadFds, SOCKET ClientSocket);
	void send_notice_message(fd_set ReadFds, SOCKET ClientSocket);
	void whisper_to_one(SOCKET SenderSocket, SOCKET ReceiverSocket, char Message[], int MessageLength);
	void send_to_all(fd_set ReadFds, char Message[], int MessageLength);
	void send_to_room(RoomInfo* room, char Message[], int MessageLength);
	
	bool room_exists(char RoomName[]);
	bool attempt_join_room(SOCKET ClientSocket, char RoomName[]);
	void join_room(SOCKET ClientSocket, RoomInfo* room);
	void leave_room(SOCKET ClientSocket);
	void ban_user(SOCKET ClientSocket, RoomInfo* room);
	void mute_user(SOCKET ClientSockets, RoomInfo* room);
	// void connect_user_to_room(SOCKET ClientSocket);

	void send_user_list(SOCKET ClientSocket);
	void send_room_list(SOCKET ClientSocket);
	void send_help_msg(SOCKET ClientSocket);

	int startup_server(int argc, char** argv);
	int run_server();

};
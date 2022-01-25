#pragma once
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

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

public:

	IOMultiplexServer();
	~IOMultiplexServer();

	void send_welcome_message(SOCKET ClientSocket);
	void session_info_message(fd_set ReadFds, SOCKET ClientSocket);
	void send_notice_message(fd_set ReadFds, SOCKET ClientSocket);
	void whisper_to_one(fd_set ReadFds, char Message[], int MessageLength, SOCKET ClientSocket);
	void send_to_all(fd_set ReadFds, char Message[], int MessageLength);

	int startup_server(int argc, char** argv);
	int run_server();

};
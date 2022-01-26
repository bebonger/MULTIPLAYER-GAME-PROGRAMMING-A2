#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <conio.h>
#include "color.h"
#include "packet_manager.h"
#include <map>
#include <vector>
#include <Windows.h>

/// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define BUFSIZE     1024
#define PORT_NUMBER 7890
#define IP_ADDRESS  "127.0.0.1"

// Client Functions
// ================
// room list
// connect to room
// leave room
// 

class IOMultiplexClient {
private:

	std::string  CurrentRoomName;
	char		 ClientID[256];

	int          Port = PORT_NUMBER;
	char         IPAddress[16] = IP_ADDRESS;
	WSADATA      WsaData;
	SOCKET       ConnectSocket;
	SOCKADDR_IN  ServerAddr;

	int          ClientLen = sizeof(SOCKADDR_IN);

	fd_set       ReadFds, TempFds;
	TIMEVAL      Timeout; // struct timeval timeout;

	char		 ReadBuffer[BUFSIZE];
	int			 ReadLen;

	char         Message[BUFSIZE];
	int          MessageLen;

	int          Return;

public:

	IOMultiplexClient();
	~IOMultiplexClient();

	int startup_client(int argc, char** argv);
	int run_client();

	int send_msg(std::string _msg);
	int send_packet(Packet _packet);

	void process_incoming_messages();

};
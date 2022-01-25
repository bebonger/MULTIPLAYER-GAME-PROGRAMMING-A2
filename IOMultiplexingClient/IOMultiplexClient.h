#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <conio.h>
#include "color.h"

/// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define BUFSIZE     1024
#define PORT_NUMBER 7890
#define IP_ADDRESS  "127.0.0.1"

class IOMultiplexClient {
private:

	int          Port = PORT_NUMBER;
	char         IPAddress[16] = IP_ADDRESS;
	WSADATA      WsaData;
	SOCKET       ConnectSocket;
	SOCKADDR_IN  ServerAddr;

	int          ClientLen = sizeof(SOCKADDR_IN);

	fd_set       ReadFds, TempFds;
	TIMEVAL      Timeout; // struct timeval timeout;

	char         Message[BUFSIZE];
	
	int          MessageLen;
	int          Return;

public:

	char		 PacketBuffer[BUFSIZE];

	IOMultiplexClient();
	~IOMultiplexClient();

	int startup_client(int argc, char** argv);
	int run_client();

	int packet_add_data(const char DataName[], const int Value);
	int packet_add_data(const char DataName[], const char Value[]);

};
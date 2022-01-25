#include "IOMultiplexServer.h"

int main(int argc, char **argv)
{
	IOMultiplexServer* server = new IOMultiplexServer();
	server->startup_server(argc, argv);
	server->run_server();

	return 0;
}

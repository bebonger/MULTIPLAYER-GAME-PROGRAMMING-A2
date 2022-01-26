#include "IOMultiplexClient.h"
// #include "packet_manager.h"

int main(int argc, char **argv)
{
	IOMultiplexClient* client = new IOMultiplexClient();
	client->startup_client(argc, argv);
	client->run_client();

	return 0;
}

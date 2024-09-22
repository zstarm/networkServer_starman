#include "server_obj.h"

int main() {
	tcp_server myServer;

	//myServer.set_addr_info(AI_PASSIVE, SOCK_STREAM, AF_UNSPEC); //testing error handling
	myServer.start_server();

	return 0;
}

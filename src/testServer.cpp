#ifdef UNIX_BUILD
#	include "server_obj.h"
#endif

#include <iostream>

int main() {
	tcp_server myServer(2);

	//myServer.set_addr_info(AI_PASSIVE, SOCK_STREAM, AF_UNSPEC); //testing error handling
	myServer.start_server();
	std::cin.ignore();
	myServer.close_server();
	while(1) {}

	return 0;
}

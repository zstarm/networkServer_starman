#ifdef UNIX_BUILD
#	include "server_obj.h"
#endif

#ifdef WIN32_BUILD
#	include "server_obj_win32.h"
#endif

#include <iostream>

int main() {
	tcp_server myServer(2, NULL, "56565");

	//myServer.set_addr_info(AI_PASSIVE, SOCK_STREAM, AF_UNSPEC); //testing error handling
	myServer.start_server();
	std::cin.ignore();
	myServer.close_server();
	std::cin.ignore();
	//while(1) {}

	return 0;
}

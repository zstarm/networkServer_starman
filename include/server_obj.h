#ifndef SERVER_OBJ
#define SERVER_OBJ

//#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <csignal>
#include <cerrno>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void sigchld_handler(int s);

class tcp_server {
	private:
		int sockfd, new_fd;
		struct addrinfo hints, *servinfo, *p;
    	struct sockaddr_storage their_addr; // connector's address information
		socklen_t sin_size;
		struct sigaction sa;
		int yes;
		char s[INET6_ADDRSTRLEN];
		int rv;

		bool successful_start;

		const char *NODE;
		const char *SERVICE;

		int BACKLOG;

		void * get_in_addr(struct sockaddr *a);
		void startup_procedure();

	public:
		tcp_server(const char *IP = NULL, const char *PORT = "3490");
		~tcp_server();

		void set_addr_info(int ai_family, int ai_socktype, int ai_flags);
		void change_backlog(int backlog);
		void start_server();


};

#endif

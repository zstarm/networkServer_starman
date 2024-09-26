#ifndef SERVER_OBJ_WIN32
#define SERVER_OBJ_WIN32

//#include <iostream>
#include <thread>
#include <cstdio>
#include <string>
#include <cstring>
#include <csignal>
#include <cerrno>

/*
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
*/
#include <winsock2.h>
#include<WS2tcpip.h>


void sigchld_handler(int s);

class tcp_server {
	private:
		WSAData wsaData;
		int sockfd, new_fd;
		int ai_family, ai_socktype, ai_flag; //flags used to create server socket
    	struct sockaddr_storage their_addr; // connector's address information
		socklen_t sin_size;
		struct sigaction sa;
		char remoteIP[INET6_ADDRSTRLEN];
		
		int fd_count; //number of polled sockets
		int fd_size; //number of simultaneous clients
		struct pollfd *pfds; //array to hold sockets of connected clients
		char buf[256]; //buffer for client data

		bool successful_start;
		bool server_full;
		//bool connected_client;
		bool maintain_server;

		const char *NODE;
		const char *SERVICE;

		int BACKLOG;

		std::thread worker_thread;

		//std::string client_msg;

		void * get_in_addr(struct sockaddr *a); //get address of a connect client
		int add_to_pfds(); //add a new client socket to poll
		void remove_from_pfds(int i); //remove a client socket from polled connections

		void WSA_startup();	

		void startup_procedure(); //start listener socket and bind to SERVICE port 
		void accept_new_connection();
		//void send_connection_msg(); //send conneciton msg to client
		//void send_ack(); //send acknowledge msg to client

		//void print_client_msg(); //print the clients message to console

	public:
		tcp_server(int serv_size = 1, const char *IP = NULL, const char *PORT = "3490");
		~tcp_server();

		void set_addr_info(int ai_family, int ai_socktype, int ai_flags); //send addrinfo struct members
		void change_backlog(int backlog); //change the backlog amount
		void start_server(); //start the TCP server 
		void close_server(); //close the active TCP server

		//bool connected(); //returns true if client is connected to the server
		//void listen_for_msg(bool v = false); //listen for message from client if there is a connection
											 //print msg to console if v=true
		//std::string get_client_msg(); //returns latest client msg as a string	

};

#endif

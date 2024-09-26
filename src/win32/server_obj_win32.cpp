#include "server_obj_win32.h"

void sigchld_handler(int s) {

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


tcp_server::tcp_server(int serv_size, const char *IP, const char *PORT) : fd_size(serv_size+1), NODE(IP), SERVICE(PORT) {
	//init boolean switches for server operation	
	successful_start = false;
	maintain_server = false;

	WSA_startup();

	//initially set server to run on computer's IP
	set_addr_info(AF_UNSPEC, SOCK_STREAM, AI_PASSIVE);

	change_backlog(1); //allow for a single backlogged client connection
	
	if(fd_size < 2) {
		printf("Not enough client slots provided. Allocating space for one client.\n");
		fd_size = 2;
	}
	pfds = (pollfd*)malloc(sizeof *pfds * fd_size); //allocate room to poll max amount of client connections
	fd_count = 0; //server has initially zero connections and no listener socket 
}

tcp_server::~tcp_server() {
	close(sockfd);
	close(new_fd);
	free(pfds);
	WSACleanup();
}

// get sockaddr, IPv4 or IPv6:
void* tcp_server::get_in_addr(struct sockaddr *sa) {

    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int tcp_server::add_to_pfds() {
	if(fd_count == fd_size) {
		if(send(new_fd, "Server is full! Try again later\n", 33, 0) == -1) { 
			perror("send");
		}
		return 0;
	}
	else {
		pfds[fd_count].fd = new_fd;
		pfds[fd_count].events = POLLIN;
		fd_count++;
		return 1;
	}
}


void tcp_server::remove_from_pfds(int i) {
	pfds[i] = pfds[fd_count - 1];
	fd_count--;
}

void tcp_server::WSA_startup() {

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup failed.\n");
		exit(1);
	}

	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		fprintf(stderr, "Version 2.2 of Winsock is not available.\n");
		WSACleanup();
		exit(2);
	}
}

void tcp_server::startup_procedure() {
	successful_start = true;

	struct addrinfo hints, *servinfo, *p;
	int rv;
	int yes = 1;
	
	memset(&hints, 0, sizeof hints);
	
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //use my IP 
	
	if ((rv = getaddrinfo(NODE, SERVICE, &hints, &servinfo)) != 0) {
		//std::cout << "getaddrinfo: " << gai_strerror(rv) << "\n";
		fprintf(stderr, "getaddrinfo %s\n", gai_strerror(rv));
		successful_start = false;
	}

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

	//adding listener socket to set
	pfds[0].fd = sockfd;
	pfds[0].events = POLLIN; //reports for incoming connections
	fd_count++; //increase polled socket count for listener
}


void tcp_server::accept_new_connection() {
	while(maintain_server) {  // main accept() loop
		int poll_count = poll(pfds, fd_count, -1); //poll for clients events	
		if(poll_count == -1) {
			perror("poll");
			break;
		}

		for(int i = 0; i < fd_count; i++) {
			if(pfds[i].revents & POLLIN) {
				//client data incoming
				if(pfds[i].fd == sockfd) {
					//listener recieved a new client connection
					sin_size = sizeof their_addr;
					new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
					if(new_fd == -1) {
						perror("accept");
					}
					else {
						int open = add_to_pfds(); //add new client to polled socket set
						inet_ntop(their_addr.ss_family,
								get_in_addr((struct sockaddr *)&their_addr),
								remoteIP, sizeof remoteIP);
						if(open) {
							printf("server: new connection from %s on socket %d\n", 
									remoteIP, new_fd);
						}
						else {
							close(new_fd);
							printf("server: server blocked connection tried from %s on socket %d\n",
									remoteIP, new_fd);
						}

					}
				}
				else { //not the listener socket
					int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);
					int sender_fd = pfds[i].fd;

					if(nbytes <= 0) {
						//error recieving client data or client disconencted
						if(nbytes == 0) {
							printf("server: client on socket %d hung up\n", sender_fd);
						}
						else {
							perror("recv");
						}

						close(pfds[i].fd); 
						remove_from_pfds(i);
					}
					else {
						//do something with client data
					}
				}
			}
		}

		/*	
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			if(maintain_server) {
				perror("accept");
				continue;
			}
			else {
				printf("Server ignoring incoming connection\n");
				break;
			}
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			//if (send(new_fd, "Hello, world!", 13, 0) == -1)
			if (send(new_fd, "Hello from the server!", 22, 0) == -1) { 
				perror("send");
			}
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
		*/
	
	}


}


void tcp_server::set_addr_info(int _ai_family, int _ai_socktype, int _ai_flags) {
	ai_family = _ai_family;
	ai_socktype = _ai_socktype;
	ai_flag = _ai_flags;
}


void tcp_server::change_backlog(int backlog) {
	BACKLOG = backlog;
}


void tcp_server::start_server() {

	startup_procedure();

	if(successful_start) {
		maintain_server = true;
		worker_thread = std::thread(&tcp_server::accept_new_connection, this);
		printf("server: waiting for connections...\n");
	}

	else {
		printf("startup unsuccessful");
	}
}

void tcp_server::close_server() {
	maintain_server = false;
	successful_start = false;
	//shutdown(sockfd, SHUT_RDWR);
	for(int i = 0; i < fd_count; i++) {
		shutdown(pfds[i].fd, SHUT_RDWR);
	}
	printf("server: closed...\n");
	worker_thread.join();
}



#include "server_obj.h"

void sigchld_handler(int s) {

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


tcp_server::tcp_server(const char *IP, const char *PORT) : yes(1), successful_start(false), NODE(IP), SERVICE(PORT) {
	
	memset(&hints, 0, sizeof hints);
	
	/*
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //use my IP 
	*/

	set_addr_info(AF_UNSPEC, SOCK_STREAM, AI_PASSIVE);

	change_backlog(10);

}

tcp_server::~tcp_server() {}

// get sockaddr, IPv4 or IPv6:
void* tcp_server::get_in_addr(struct sockaddr *sa) {

    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void tcp_server::startup_procedure() {
	successful_start = true;

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
}


void tcp_server::set_addr_info(int ai_family, int ai_socktype, int ai_flags) {
	hints.ai_family = ai_family;
	hints.ai_socktype = ai_socktype;
	hints.ai_flags = ai_flags;
}


void tcp_server::change_backlog(int backlog) {
	BACKLOG = backlog;
}


void tcp_server::start_server() {

	startup_procedure();

	if(successful_start) {
		printf("server: waiting for connections...\n");

		while(1) {  // main accept() loop
			sin_size = sizeof their_addr;
			new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
			if (new_fd == -1) {
				perror("accept");
				continue;
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
		}
	}

	else {
		printf("startup unsuccessful");
	}
}



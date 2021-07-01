// server.cc -- a simple socket server - serves only a single client
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXDATASIZE 1000
#define BACKLOG 10

int main(int argc, char *argv[]) {
	int sockfd;
	int new_fd;
	struct addrinfo hints, *servinfo;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	char s[INET_ADDRSTRLEN];
	int rv;

	char msg[MAXDATASIZE];
	char buf[MAXDATASIZE];
	int numbytes;

	if (argc != 2){
		printf("usage: server portnum\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		perror("server: socket");
		exit(1);
	}

	if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
		close(sockfd);
		perror("server: bind");
		exit(1);
	}

	freeaddrinfo(servinfo);

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf("server: waiting for connections on port %s...\n", argv[1]);

	sin_size = sizeof their_addr;
	// blocking
	new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
	if (new_fd == -1) {
		perror("accept");
		exit(1);
	}

	inet_ntop(their_addr.ss_family, &((struct sockaddr_in*)&their_addr)->sin_addr, s, sizeof s);
	printf("server: got connection form %s\n", s);

	close(sockfd);

	while (1) {
		// error
		if ((numbytes = recv(new_fd, buf, sizeof buf, 0)) == -1) {
			perror("recv");
			close(new_fd);
			exit(0);
		}

		// client stop (return value 0)
		if (numbytes == 0) {
			close(new_fd);
			break;
		}

		// print message
		buf[numbytes] = '\0';
		printf("server: received '%s'\n", buf);

		// send message to client
		if (!strcmp(buf, "hello")) strcpy(msg, "how are you?");
		else if (!strcmp(buf, "thank you")) strcpy(msg, "you are welcome!");
		else strcpy(msg, "what?");
		
		if (send(new_fd, msg, sizeof msg, 0) == -1) {
			perror("send");
			close(new_fd);
			exit(0);
		}
	
	}
	
	close(new_fd);

	return 0;
}

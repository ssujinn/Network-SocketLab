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
	int newfd;
	struct addrinfo hints, *servinfo;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	char s[INET_ADDRSTRLEN];
	int rv;

	// lab3
	fd_set master;
	fd_set read_fds;
	int fdmax;

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

	/* ---------------------lab3---------------------- */
	FD_ZERO(&master);
	FD_SET(sockfd, &master);
	fdmax = sockfd;

	while(1) {
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(1);
		}

		for (int i = 0; i <= fdmax; i++){
			if (FD_ISSET(i, &read_fds)){
				if (i == sockfd) {	// sockfd로부터 메세지 온 경우
					sin_size = sizeof their_addr;
					// blocking
					newfd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
					if (newfd == -1) {
						perror("accept");
						exit(1);
					}
					else {
						FD_SET(newfd, &master);
						if(newfd > fdmax)
							fdmax = newfd;
					}
					inet_ntop(their_addr.ss_family, &((struct sockaddr_in*)&their_addr)->sin_addr, s, sizeof s);
					printf("server: got connection form %s\n", s);
				}
				else {	// client와 통신하고 있는 socket으로부터 메세지 온 경우
					if ((numbytes = recv(i, buf, sizeof buf, 0)) <= 0){
						if (numbytes == 0){
							printf("selectserver: socket %d hung up\n", i);
						}
						else {
							perror("recv");
						}
						close(i);
						FD_CLR(i, &master);
					}
					else { // 메세지 온 경우
						buf[numbytes] = '\0';
						printf("server received: %s\n", buf);

						/*---------------lab4--------------*/
						for (int j = 0; j <= fdmax; j++) {
							if (j == sockfd) continue;
							if (j == i) continue;
							if (!FD_ISSET(j, &master)) continue;
							//printf("sending to socket %d\n", j);
							if (send(j, buf, strlen(buf), 0) == -1)
								perror("send");
						}
					}
				}
			}
		}
	}

	return 0;
}

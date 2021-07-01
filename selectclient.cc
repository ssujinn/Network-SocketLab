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

int main(int argc, char *argv[]) {
	int sockfd;
	int numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo;
	int rv;
	char s[INET_ADDRSTRLEN];

	// lab4
	fd_set master, read_fds;


	if (argc != 3){
		fprintf(stderr, "usage: client hostname portnum\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// create socket
	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){
		perror("client: socket");
		return 2;
	}

	// connect to server
	if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		close(sockfd);
		perror("client: connect");
		return 2;
	}

	// print to screen
	inet_ntop(servinfo->ai_family, &((struct sockaddr_in*)servinfo->ai_addr)->sin_addr, s, sizeof s);
	printf("client: connecting to %s\n", s);

	// free allocated memory for servinfo
	freeaddrinfo(servinfo);

	/* ---------------------lab4-----------------------*/
	FD_ZERO(&master);
	FD_SET(0, &master);
	FD_SET(sockfd, &master);

	int quit = 0;
	while (1) {
		read_fds = master;
		if (select(sockfd + 1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(1);
		}

		for (int i = 0; i <= sockfd + 1; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == 0) {
					// message entered by user
					//gets(buf);
					fgets(buf, sizeof buf, stdin);
					buf[strlen(buf) - 1] = '\0';
					if (strcmp(buf, "quit") == 0) {
						quit = 1;
						break;
					}
					if (send(sockfd, buf, strlen(buf), 0) == -1) {
						perror("send");
						close(sockfd);
						exit(1);
					}
				}
				else if (i == sockfd) {
					// message received from server
					if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
						perror("recv");
						close(sockfd);
						exit(1);
					}
					
					if (numbytes == 0) {
						printf("server disconnected. \n");
						quit = 1;
						break;
					}

					buf[numbytes] = '\0';
					printf("%s\n", buf);
				}
			}
		}

		if (quit) break;
	}

	printf("connection closed.\n");
	close(sockfd);

	return 0;
}

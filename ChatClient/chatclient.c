/*******************************************************************************
 * Name        : chatclient.c
 * Editor      : Kaiqi Chee
 * Date        : 05/07/2021
 * Description : Chat client with sockets.
 * Pledge	: I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"

int client_socket = -1;
char username[MAX_NAME_LEN+1];
char inbuf[BUFLEN+1];
char outbuf[MAX_MSG_LEN+1];

int usage_message(char *executable){
	printf("Usage: %s <server IP> <port>\n", executable);
	return EXIT_FAILURE;
}

int handle_stdin(){
	memset(outbuf, '\0', MAX_MSG_LEN + 1);
	int bytes = 0;
	if ((bytes = get_string(outbuf, BUFLEN)) == -1) {
		fprintf(stderr, "Error: Failed to read from stdin. %s.\n",
				strerror(errno));
		return EXIT_SUCCESS;
	}
	if (bytes==TOO_LONG) {
		printf("Sorry, limit your message to %d characters.\n",
				MAX_MSG_LEN);
	}
	else if(bytes != NO_INPUT){
		send(client_socket, outbuf, MAX_MSG_LEN + 1, 0);
	}
	if (strcmp(outbuf, "bye")==0){
		printf("Goodbye.\n");
		close(client_socket);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int handle_client_socket(){
	memset(inbuf, '\0', BUFLEN + 1);
	int bytes_recvd;
	if((bytes_recvd = recv(client_socket, inbuf, BUFLEN, 0))==-1){
		fprintf(stderr,"Warning: Failed to receive incoming message. %s.\n", strerror(errno));
		return EXIT_SUCCESS;
	}
	if(bytes_recvd==0){
		fprintf(stderr,"\nConnection to server has been lost.\n");
		return EXIT_FAILURE;
	}
	inbuf[bytes_recvd]='\0';
	if (strcmp(inbuf, "bye")==0){
		printf("\nServer initiated shutdown.\n");
		close(client_socket);
		return EXIT_FAILURE;
	}
	else{
		printf("\n%s\n", inbuf);
	}
	return EXIT_SUCCESS;
}

int create_socket() {
    int client_socket;
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
		return -1;
	}
    return client_socket;
}

int main(int argc, char *argv[]){
	if(argc!=3){
		usage_message("./chatclient");
		return EXIT_FAILURE;
	}
	int bytes_recvd;
	int bytes = 0;
	struct sockaddr_in serv_addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	memset(&serv_addr, 0, addrlen);
	serv_addr.sin_family = AF_INET;
	int ip_conversion = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
	if (ip_conversion == 0) {
		fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
		return EXIT_FAILURE;
	}
	else if (ip_conversion < 0) {
		fprintf(stderr, "Error: Failed to convert IP address. %s.\n",
				strerror(errno));
		return EXIT_FAILURE;
	}
	//error check port
	int port;
	bool b=parse_int(argv[2], &port, "Port");

	if (b==false){
		return EXIT_FAILURE;
	}
	if (port<1024 || port>65535){
		fprintf(stderr, "Error: Port must be in range [1024, 65535].\n");
		return EXIT_FAILURE;
	}
	serv_addr.sin_port = htons(port);

	while(true){
		fflush(stdout);
		printf("Enter your username: ");
		fflush(stdout);
		memset(username,'\0', MAX_NAME_LEN+1);
		if ((bytes = read(STDIN_FILENO, username, BUFLEN)) == -1) {
			fprintf(stderr, "Error: Failed to read from stdin. %s.\n",
					strerror(errno));
			return EXIT_FAILURE;
		}
		username[bytes-1]='\0';
		if(bytes-1==0){
			continue;
		}
		else if (bytes-1>MAX_NAME_LEN){
			printf("Sorry, limit your username to %d characters.\n", MAX_NAME_LEN);
			continue;
		}
		else{
			break;
		}
	}
	printf("Hello, %s. Let's try to connect to the server.\n", username);
	if ((client_socket = create_socket()) == -1) {
		return EXIT_FAILURE;
	}
	if (connect(client_socket, (struct sockaddr*) &serv_addr, addrlen) == -1) {
		fprintf(stderr, "Error: Failed to connect to server. %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}
	else{
		memset(inbuf, '\0', BUFLEN+1);
		if((bytes_recvd = recv(client_socket, inbuf, BUFLEN, 0)) == -1){
			fprintf(stderr,"Error: Failed to receive message from server. %s.\n", strerror(errno));
			return EXIT_FAILURE;
		}
		if(bytes_recvd==0){
			fprintf(stderr,"All connections are busy. Try again later.\n");
			return EXIT_FAILURE;
		}
		printf("\n");
		printf("%s", inbuf);
		printf("\n");
		printf("\n");
		if(send(client_socket, username, bytes, 0)==-1){
			fprintf(stderr, "Error: Failed to send username to server. %s.\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}
    fd_set sockset;
	while (true) {
		fflush(stdout);
		printf("[%s]: ",username);
		fflush(stdout);
		FD_ZERO(&sockset);
		FD_SET(client_socket, &sockset);
		FD_SET(STDIN_FILENO, &sockset);

		if (select(client_socket + 1, &sockset, NULL, NULL, NULL)< 0) {
			fprintf(stderr, "Error: select() failed. %s.\n", strerror(errno));
			return EXIT_FAILURE;
		}
		if (FD_ISSET(STDIN_FILENO, &sockset)) {
			if(handle_stdin()==EXIT_FAILURE){
				break;
			}
		}
		if (FD_ISSET(client_socket, &sockset)) {
			if(handle_client_socket()==EXIT_FAILURE){
				break;
			}
		}
	}
	return EXIT_SUCCESS;
}

#include "transmission_debug.h"	
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

void transmission_debug::send(void *data, size_t len)
{
	write(sockfd_, data, len);
}

void transmission_debug::receive(void *data, size_t len)
{
	read(sockfd_, data, len);
}

void transmission_debug::close_socket()
{
	close(sockfd_);
	sockfd_ = 0;
}

Error transmission_debug::connect_to_master(const char *host, int port, transmission **out)
{
	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (isalpha(host[0])) {
		// use DNS to get IP address
		struct hostent *hostEntry;
		hostEntry = gethostbyname(host);
		if (!hostEntry) {
			return kError_NoSuchHostname;
		}
		memcpy(&server_addr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);	
	} else {
		// IP was passed as argument
		server_addr.sin_addr.s_addr = inet_addr(host);
	}

	// create socket
	int sockfd = socket(PF_INET,SOCK_STREAM,0);
	if (!sockfd) {
		return kError_SocketCreation;
	}

	// connect to server
	if (connect(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		return kError_SocketConnect;
	}

	*out = new transmission_debug(sockfd);
	return kError_None;
}
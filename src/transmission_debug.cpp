#include "blackbox.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include "transmission_debug.h"	

#if PLAT == PLAT_WINDOWS
#	pragma comment(lib, "Ws2_32.lib")
#	include <WinSock2.h>
#	include <Ws2tcpip.h>
#	define write(s,d,l) ::send(s,(char*)d,l,0)
#	define read(s,d,l) ::recv(s,(char*)d,l,0)
#	define close(s)	closesocket(s)
#else
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <unistd.h>
#	include <netdb.h>
#	include <arpa/inet.h>
#endif

static Error perform_dns_lookup(sockaddr_in *server_addr, const char *host)
{
#if PLAT == PLAT_WINDOWS
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	if (getaddrinfo(host, NULL, &hints, &res) != 0) {
		return kError_NoSuchHostname;
	}

	server_addr->sin_addr.S_un = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.S_un;
	freeaddrinfo(res);
#else
	hostent *hostEntry = gethostbyname(host);
	if (!hostEntry) {
		return kError_NoSuchHostname;
	}
	memcpy(&server_addr->sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);	
#endif
	return kError_None;
}

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
		Error dnsError = perform_dns_lookup(&server_addr, host);
		if (dnsError != kError_None) {
			return dnsError;
		}
	} else {
		// IP was passed as argument
#if PLAT == PLAT_WINDOWS
		inet_pton(AF_INET, host, &server_addr.sin_addr.s_addr);
#else
		server_addr.sin_addr.s_addr = inet_addr(host);
#endif
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
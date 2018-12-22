#include "blackbox.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include "transmission_debug.h"	
#include "plat_compat.h"
#include "cprintf.h"
#include "crc32.h"

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

static int get_error_code()
{
#if PLAT == PLAT_WINDOWS
	return WSAGetLastError();
#else
	return errno;
#endif
}

static bool is_would_block_error()
{
#if PLAT == PLAT_WINDOWS
	if (get_error_code() == WSAEWOULDBLOCK) {
		return true;
	}
#else
	if (get_error_code() == EWOULDBLOCK) {
		return true;
	}
#endif
	return false;
}

static Error perform_dns_lookup(sockaddr_in *server_addr, const char *host)
{
#if PLAT == PLAT_WINDOWS
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;
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

size_t transmission_debug::send(void *data, size_t len)
{
	int status = sendto(sockfd_, (char*)data, len, 0, (sockaddr*)&server_addr, sizeof(server_addr));
	if (status < 0) {
		c_printf("{y}warn: {d}%s couldn't send data (%d, %d)\n", __FUNCTION__, status, get_error_code());
	}
	return (size_t)status;
}

size_t transmission_debug::receive(void *data, size_t len)
{
	return receive_internal(data, len, true);
}

size_t transmission_debug::send_message(message const& msg)
{
	size_t bytes = send(msg.data, msg.len);
	if (bytes < msg.len) {
		c_printf("{y}warn: {d}server didn't receive all data (%d/%d bytes), skipping %s\n",
			bytes, msg.len, __FUNCTION__);
		return bytes;
	}

	// save send time if this message was marked as reliable.
	if (msg.get_flags() & (uint8_t)packet_flags::reliable) {
		ack_wait *wait = new ack_wait;
		wait->msg = msg;
		wait->send_time = millis();

		waiting_for_ack.push_back(wait);
	}
	return bytes;
}

bool transmission_debug::receive_message(message &msg)
{
	size_t bytes = receive(msg.data, msg.len);
	return bytes > 0;
}

void transmission_debug::update(message_listener *listener)
{
	message msg;
	msg.len = packet_max_size;
	msg.data = new uint8_t[msg.len];

	// receiving data
	while (receive_message(msg)) {
		// TODO: verify checksum
		// msg.get_checksum() == crc32...

		// If this is a ACK message we have to look if this
		// is for a reliable message we send in the past.
		// In every case, we hide this message from our message listener.
		if (msg.get_flags() & (uint8_t)packet_flags::ack) {
			if (msg.get_bbid() == bbid_) {
				std::vector<ack_wait>::size_type ack_msg_index;
				bool found_ack_msg = false;
				for (std::vector<ack_wait>::size_type i = 0; i < waiting_for_ack.size(); ++i) {
					ack_wait *wait = waiting_for_ack[i];
					const message &wfa_msg = wait->msg;
					if (wfa_msg.get_packet_id() == msg.get_packet_id() &&
						wfa_msg.get_bbid() == msg.get_bbid() &&
						wfa_msg.get_packet_number() == msg.get_packet_number())
					{
						found_ack_msg = true;
						ack_msg_index = i;

						delete wait;
					}
				}

				if (found_ack_msg) {
					waiting_for_ack.erase(waiting_for_ack.begin() + ack_msg_index);
				} else {
					c_printf("{r}error: {d}received ACK message without sending a reliable message\n");
				}
			}

			// hide message from listener
			continue;
		}

		listener->on_message(msg);
	}

	// check if we need to re-send any reliable packets
	if (!waiting_for_ack.empty()) {
		uint64_t now = millis();
		for (std::vector<ack_wait>::size_type i = 0; i < waiting_for_ack.size(); ++i) {
			message &msg = waiting_for_ack[i]->msg;
			uint64_t send_time = waiting_for_ack[i]->send_time;

			if (now - send_time > packet_reliable_timeout) {
				waiting_for_ack[i]->send_time = now;
			}
		}
	}
}

size_t transmission_debug::receive_internal(void *data, size_t len, bool nonblocking)
{
	int flags = 0;

	if (nonblocking) {
#if PLAT == PLAT_WINDOWS
		u_long mode = 1;
		int result = ioctlsocket(sockfd_, FIONBIO, &mode);
		if (result != NO_ERROR) {
			c_printf("{y}warn: {d}%s failed to enter non-blocking mode\n", __FUNCTION__);
		}
#else
		flags = MSG_DONTWAIT;
#endif
	}

	int status = recvfrom(sockfd_, (char*)data, len, flags, NULL, NULL);
	if (status < 0) {
		if (is_would_block_error()) {
			status = 0;
		} else {
			c_printf("{y}warn: {d}%s couldn't receive data (%d)\n", __FUNCTION__, status);
		}
	}

	if (nonblocking) {
#if PLAT == PLAT_WINDOWS
		u_long mode = 0;
		int result = ioctlsocket(sockfd_, FIONBIO, &mode);
		if (result != NO_ERROR) {
			c_printf("{y}warn: {d}%s failed to enter blocking mode\n", __FUNCTION__);
		}
#endif
	}

	return (size_t)status;
}

void transmission_debug::close_socket()
{
	close(sockfd_);
	sockfd_ = 0;
}

Error transmission_debug::connect_to_master(const char *host, int port, uint16_t bbid, transmission **out)
{
	struct sockaddr_in server_addr;

#if PLAT == PLAT_WINDOWS
	WSADATA wsa_data;
	int wsa_status = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (wsa_status != NO_ERROR) {
		c_printf("{r}error: {d}failed to initialize winsock\n");
		return kError_SocketCreation;
	}
#endif

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
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!sockfd) {
		return kError_SocketCreation;
	}

	// TCP: connect to server
	//if (connect(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
	//	return kError_SocketConnect;
	//}

	*out = new transmission_debug(sockfd, server_addr, bbid);
	return kError_None;
}
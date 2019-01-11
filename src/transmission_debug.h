#include "transmission.h"
#include "errors.h"
#include "blackbox.h"
#include <vector>

// include definition for sockaddr_in
#if PLAT == PLAT_WINDOWS
#	include <WinSock2.h>
#else
#	include <netinet/in.h>
#endif

struct ack_wait
{
	ack_wait()
	{
	}
	/*ack_wait(message const& m, uint64_t st): msg(m), send_time(st)
	{
	}
	ack_wait(ack_wait const& o): msg(o.msg), send_time(o.send_time)
	{
	}*/
	message *msg;
	uint64_t send_time;
};

struct fragmented_packet
{
	fragmented_packet(message *m): msg(m)
	{
	}
	message *msg;
};

class transmission_debug: public transmission
{
	int sockfd_;
	sockaddr_in server_addr;
	std::vector<ack_wait*> waiting_for_ack;
	uint16_t bbid_;
public:
	transmission_debug(int sockfd, sockaddr_in addr, uint16_t bbid):
		sockfd_(sockfd), server_addr(addr), bbid_(bbid)
	{
	}
	virtual ~transmission_debug()
	{
		close_socket();
	}
	virtual const char *name()
	{
		return "debug";
	}
	size_t send(void *data, size_t len);
	size_t receive(void *data, size_t len);

	virtual size_t send_message(message const& msg);
	size_t send_message_internal(message const& msg, bool fragment_handled=false);
	bool receive_message(message &msg);

	void update(message_listener *listener);

private:
	size_t receive_internal(void *data, size_t len, bool nonblocking);
	void close_socket();

public:
	static Error connect_to_master(const char *host, int port, uint16_t bbid, transmission **out);
};
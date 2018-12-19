#include "transmission.h"
#include "errors.h"

class transmission_debug: public transmission
{
	int sockfd_;
public:
	transmission_debug(int sockfd): sockfd_(sockfd)
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
	virtual void send(void *data, size_t len);
	virtual void receive(void *data, size_t len);

public:
	void close_socket();
	static Error connect_to_master(const char *host, int port, transmission **out);
};
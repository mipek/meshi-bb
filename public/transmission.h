#ifndef _inclde_public_transmission
#define _inclde_public_transmission
 
class transmission
{
public:
	virtual ~transmission() {};
	virtual const char *name() =0;
	//virtual bool init(char *msg, size_t len) =0;
	virtual void send(void *data, size_t len) =0;
	virtual void receive(void *data, size_t len) =0;
};

#endif //_inclde_public_transmission
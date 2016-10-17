#ifndef NET_CONNECTION_H_
#define NET_CONNECTION_H_

#define APR_DECLARE_STATIC

#include <apr_pools.h>
#include <apr_general.h>
#include <apr_network_io.h>

class Net_connection
{
public:
	bool send_data(const char* data, size_t length, size_t* sent_bytes = NULL);
	bool receive_data(char* data, size_t buffer_size, size_t* received_bytes = NULL);
	void disconnect();
protected:
	apr_pool_t		*mp;		//pool used in class
	apr_socket_t	*sock;		//Remote computer related socket
	bool init_succ;
	bool connected;
	Net_connection();
	~Net_connection();
};

#endif //NET_CONNECTION_H_

#ifndef NET_CONNECTION_H_
#define NET_CONNECTION_H_

#define APR_DECLARE_STATIC

#include <apr_pools.h>
#include <apr_general.h>
#include <apr_network_io.h>
#include <iostream>

using namespace std;

class Net_connection
{
public:
	Net_connection();
	bool send_data(const char* data, size_t length, size_t* sent_bytes = NULL);
	bool receive_data(char* data, size_t buffer_size, size_t* received_bytes = NULL);
	bool is_connected();
	virtual void disconnect() = 0;
	virtual ~Net_connection();
	//learned something here: http://stackoverflow.com/questions/12092933/calling-virtual-function-from-destructor
	//The actual type of the object changes during construction and it changes again during destruction.
	//When a destructor is being executed, the object is of exactly that type,
	//and never a type derived from it. 
	//Therefore, cannot call virtual function from destructor, since destructor will try to execute
	//Net_connection::disconnect();
	//So can't call disconnect from destructor, must use virtual destructor.
protected:
	apr_pool_t		*mp;		//pool used in class
	apr_socket_t	*sock;		//Remote computer related socket
	bool init_succ;
	bool connected;
};

#include "Client.h"
#include "Server.h"
#endif //NET_CONNECTION_H_

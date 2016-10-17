#include "Net_connection.h"
#include <iostream>

using namespace std;

Net_connection::Net_connection()
{
	connected = false;
	sock = NULL;
	mp = NULL;
	//It is safe to call apr_initialize several times as long as
	//apr_terminate is called the same number of times.
	if (apr_initialize() == APR_SUCCESS)	//Initialize Apr. 
	{
		if (apr_pool_create(&mp, NULL) == APR_SUCCESS)	//Create pool for net connection
		{
			init_succ = true;
			return;
		}
		else
		{
			cerr << "Could not create APR pool for net connection." << endl;
			mp = NULL;
		}
		apr_terminate();
	}
	else
		cerr << "Could not initialize APR." << endl;
}

bool Net_connection::send_data(const char* data, size_t length, size_t* sent_bytes)
{
	bool ret_val = false;
	size_t bytes = length;
	apr_status_t rv;
	if (connected)
	{
		rv = apr_socket_send(sock, data, &bytes);
		if (sent_bytes != NULL)
			*sent_bytes = bytes;
		if (rv == APR_SUCCESS)	//Send data
		{
			ret_val = true;
		}
	}
	return ret_val;
}

bool Net_connection::receive_data(char* data, size_t buffer_size, size_t* received_bytes)
{
	bool ret_val = false;
	apr_status_t rv;
	size_t bytes;
	if (connected && buffer_size != 0)
	{
		bytes = buffer_size;
		rv = apr_socket_recv(sock, data, &bytes);
		if (received_bytes != NULL)
			*received_bytes = bytes;
		if (rv == APR_SUCCESS || rv != APR_EOF)
		{
			ret_val = true;
		}
	}
	return ret_val;
}

void Net_connection::disconnect()
{
	if (connected)
	{
		apr_socket_close(sock);
		sock = NULL;
	}
}

Net_connection::~Net_connection()
{
	disconnect();
	if (mp != NULL)
	{
		apr_pool_destroy(mp);
		apr_terminate();
	}
}
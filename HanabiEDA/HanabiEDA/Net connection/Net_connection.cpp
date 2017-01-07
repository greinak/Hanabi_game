#include "Net_connection.h"
#include <iostream>
#include <iterator>
#include <iomanip>

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
			cerr << "[NET_CONNECTION][ERROR] : Could not create APR pool for net connection." << endl;
			mp = NULL;
		}
		apr_terminate();
	}
	else
		cerr << "[NET_CONNECTION][ERROR] : Could not initialize APR." << endl;
}

bool Net_connection::send_data(const char* data, size_t length, size_t* sent_bytes)
{
	bool ret_val = false;
	size_t bytes = length;
	apr_status_t rv;
	if (sent_bytes != nullptr)
	{
		if (length != 0)
			cout << "[NET_CONNECTION][INFO] : Sending data..." << endl;
		if (connected)
		{
			rv = apr_socket_send(sock, data, &bytes);
			if (sent_bytes != NULL)
				*sent_bytes = bytes;
			if (rv == APR_SUCCESS)	//Send data
			{
				if (length != 0)
				{
					ios::fmtflags f(cout.flags());
					cout << "[NET_CONNECTION][INFO] : " << (int) length << " byte(s) sent! Data: " ;
					for (unsigned int i = 0; i < length; i++)
						cout << uppercase << setw(2) << setfill('0') << hex << (int)((unsigned char*)data)[i];
					cout << endl;
					cout.flags(f);
				}
				ret_val = true;
			}
			else
			{
				if (length != 0)
					cerr << "[NET_CONNECTION][ERROR] : Cannot send data." << endl;
			}
		}
	}
	else
		cerr << "[NET_CONNECTION][ERROR] : Bad call to send data." << endl;
	return ret_val;
}

bool Net_connection::receive_data(char* data, size_t buffer_size, size_t* received_bytes)
{
	bool ret_val = false;
	apr_status_t rv;
	size_t bytes;
	if (received_bytes != nullptr)
	{
		if (connected && buffer_size != 0 && send_data(nullptr, 0, received_bytes))	//Sending zero length data is a workaround for detecting when connection falls
		{
			bytes = buffer_size;
			rv = apr_socket_recv(sock, data, &bytes);
			if (received_bytes != NULL)
				*received_bytes = bytes;
			if (rv == APR_SUCCESS || rv != APR_EOF)
			{
				if (*received_bytes != 0)
				{
					ios::fmtflags f(cout.flags());
					cout << "[NET_CONNECTION][INFO] : Received " << (int) *received_bytes << " byte(s)! Data: ";
					for (unsigned int i = 0; i < *received_bytes; i++)
						cout << uppercase << setw(2) << setfill('0') << hex << (int)((unsigned char*)data)[i];
					cout << endl;
					cout.flags(f);
				}
				ret_val = true;
			}
		}
		if (!ret_val)
			cerr << "[NET_CONNECTION][ERROR] : Error receiving data! Connection with remote computer may have been lost." << endl;
	}
	else
		cerr << "[NET_CONNECTION][ERROR] : Bad call to receive data." << endl;
	return ret_val;
}

bool Net_connection::is_connected()
{
	return connected;
}

Net_connection::~Net_connection()
{
	if (mp != NULL)
	{
		apr_pool_destroy(mp);
		apr_terminate();
	}
}
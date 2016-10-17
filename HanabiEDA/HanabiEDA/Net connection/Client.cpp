#include "Client.h"

bool Client::connect_to_server(string server_ip, unsigned int port)
{
	if (!connected && init_succ)
	{
		apr_sockaddr_t *sa;
		if (apr_sockaddr_info_get(&sa, server_ip.c_str(), AF_INET, port, 0, mp) == APR_SUCCESS)
		{
			if (apr_socket_create(&sock, APR_UNSPEC, SOCK_STREAM, APR_PROTO_TCP, mp) == APR_SUCCESS)
			{
				apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1);
				apr_socket_timeout_set(sock, 1);	//1us to get APR_SUCCESS if connection is successful
				apr_status_t rv = apr_socket_connect(sock, sa);
				if (rv == APR_SUCCESS)
				{
					//See next comment
					connected = true;
					apr_socket_timeout_set(sock, 0);
					return true;
				}
				apr_socket_close(sock);
				sock = NULL;
			}
			//Checked apr source code. when source code uses sockaddr, it does not free it
			//I suppose apr pool manages this
		}
	}
	return false;
}

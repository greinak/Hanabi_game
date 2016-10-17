#include "Server.h"

Server::Server(unsigned int port)
{
	serv_sock = NULL;
	apr_sockaddr_t* sa;
	if (apr_sockaddr_info_get(&sa, NULL, AF_INET, port, 0, mp) == APR_SUCCESS)
	{
		if (apr_socket_create(&serv_sock, APR_UNSPEC, SOCK_STREAM, APR_PROTO_TCP, mp) == APR_SUCCESS)
		{
			apr_socket_opt_set(serv_sock, APR_SO_NONBLOCK, 1);
			apr_socket_timeout_set(serv_sock, 0);
			apr_socket_opt_set(serv_sock, APR_SO_REUSEADDR, 1);
			if (apr_socket_bind(serv_sock, sa) == APR_SUCCESS)
			{
				if (apr_socket_listen(serv_sock, SOMAXCONN) == APR_SUCCESS)
				{
					//See next comment
					return;
				}
			}
			apr_socket_close(serv_sock);
			serv_sock = NULL;
		}
		//Checked apr source code. when source code uses sockaddr, it does not free it
		//I suppose apr pool manages this
	}
}

Server::~Server()
{
	disconnect();
	if (serv_sock != NULL)
		apr_socket_close(serv_sock);
}

bool Server::listen_for_connection(apr_interval_time_t timeout)
{
	if (serv_sock != NULL && !connected && init_succ)
	{
		clock_t server_timer = clock();
		do
			apr_socket_accept(&sock, serv_sock, mp);
		while (sock == NULL && ((clock() - server_timer) / CLOCKS_PER_SEC) < timeout);
		if (sock != NULL)
		{
			apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1);
			apr_socket_timeout_set(sock, 0);
			connected = true;
			return true;
		}
	}
	return false;
}

#include "Server.h"
#include <iostream>

Server::Server(unsigned int port)
{
	cout << "[NET_CONNECTION][INFO] : Initializing server with port " << port << endl;
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
					//See next comment
					return;
			}
			apr_socket_close(serv_sock);
			serv_sock = NULL;
		}
		//Checked apr source code. when source code uses sockaddr, it does not free it
		//I suppose apr pool manages this
	}
}

bool Server::listen_for_connection(string allowed, unsigned int timeout_ms)
{
	if (serv_sock != NULL && !connected && init_succ)
	{
		if (apr_socket_listen(serv_sock, -1) == APR_SUCCESS)
		{
			cout << "[NET_CONNECTION][INFO] : Listening for connections, timeout: " << timeout_ms << "ms." << endl;
			clock_t server_timer = clock();
			bool go_on = true;
			do
				apr_socket_accept(&sock, serv_sock, mp);
			while (sock == NULL && (go_on = (float)((clock() - server_timer) / (float)CLOCKS_PER_SEC) * 1000 < timeout_ms));
			if (sock != NULL)
			{
				apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1);
				apr_socket_timeout_set(sock, 0);
				connected = true;
				apr_socket_close(serv_sock);
				serv_sock = nullptr;
				cout << "[NET_CONNECTION][INFO] : Connected to client!" << endl;
				return true;
			}
		}
	}
	return false;
}

void Server::disconnect()
{
	if (connected)
	{
		cout << "[NET_CONNECTION][INFO] : Disconnecting server" << endl;
		connected = false;
		apr_socket_close(sock);
		sock = NULL;
		if (serv_sock != nullptr)
		{
			apr_socket_close(serv_sock);
			serv_sock = nullptr;
		}
	}
}

Server::~Server()
{
	disconnect();
	cout << "[NET_CONNECTION][INFO] : Server closed" << endl;
}
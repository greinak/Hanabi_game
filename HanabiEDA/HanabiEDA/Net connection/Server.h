#ifndef SERVER_H_
#define SERVER_H_

#include "Net_connection.h"

#define DEFAULT_PORT	13796

class Server : public Net_connection
{
public:
	Server(unsigned int port = DEFAULT_PORT);
	~Server();
	bool listen_for_connection(apr_interval_time_t timeout);
private:
	apr_socket_t* serv_sock;
};

#endif //SERVER_H_
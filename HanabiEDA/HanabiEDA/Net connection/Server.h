#ifndef SERVER_H_
#define SERVER_H_

#include "Net_connection.h"

class Server : public Net_connection
{
public:
	Server(unsigned int port);
	~Server();
	bool listen_for_connection(unsigned int timeout_ms);
private:
	apr_socket_t* serv_sock;
};

#endif //SERVER_H_
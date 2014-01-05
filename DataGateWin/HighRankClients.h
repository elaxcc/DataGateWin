#ifndef _HIGH_RANK_CLIENTS_
#define _HIGH_RANK_CLIENTS_

#include "NetCommon.h"

namespace high_rank_client
{

class server_connection : public Net::connection
{
public:
	server_connection(int socket);
	~server_connection();

public: // Net::connection
	virtual int process_events(short int polling_events);

private:
	enum status
	{
		status_not_logined = 1,
		status_logined = 2
	};

private:
	std::vector<char> id_;
	status status_;
	std::vector<char> data_buffer_;
};

class server : public Net::server
{
public:
	server(Net::net_manager *net_manager, int port, bool nonblocking, bool no_nagle_delay);
	~server();

public: // Net::server
	virtual Net::i_net_member* create_connection(int socket);

private:
};

} // high_rank_client

#endif // _HIGH_RANK_CLIENTS_
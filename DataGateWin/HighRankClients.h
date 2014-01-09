#ifndef _HIGH_RANK_CLIENTS_
#define _HIGH_RANK_CLIENTS_

#include "NetCommon.h"
#include "LocalCommunication.h"

class data_base;

namespace high_rank_clients
{

class server;

class server_connection : public Net::connection
{
public:
	server_connection(data_base *db, int socket, server *own_server);
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
	data_base *db_;
	server *own_server_;
};

class server : public Net::server, public Net::i_local_communicator
{
public:
	server(data_base *db,
		Net::net_manager *net_manager,
		Net::local_communicator_manager *local_comm_manager,
		int port, bool nonblocking, bool no_nagle_delay);
	~server();

	void register_client(const std::string& client_id, server_connection* connection);
	void unregister_client(const std::string& client_id);

public: // Net::server
	virtual Net::i_net_member* create_connection(int socket);

public: // Net::i_local_communicator
	virtual int process_message(const std::string& link,
		const std::vector<char>& data);

private:
	data_base *db_;
	Net::local_communicator_manager *local_comm_manager_;
	std::map<std::string, server_connection*> clients_;
};

} // high_rank_client

#endif // _HIGH_RANK_CLIENTS_
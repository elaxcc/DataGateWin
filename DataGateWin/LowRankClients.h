#ifndef _LOW_CLIENT_SERVER_
#define _LOW_CLIENT_SERVER_

#include "NetCommon.h"
#include "LocalCommunication.h"
#include "HighRankClients.h"

class data_base;

namespace low_rank_clients
{

	class server_connection : public Net::connection, public Net::i_local_communicator
{
public:
	server_connection(data_base *db,
		high_rank_clients::server *high_rank_server,
		Net::local_communicator_manager *local_comm_manager,
		int socket);
	~server_connection();

public: // Net::connection
	virtual int process_events(short int polling_events);

public: // Net::i_local_communicator
	virtual int process_message(const std::string& link,
		const std::vector<char>& data);

private:
	enum status
	{
		status_not_logined = 1,
		status_logined = 2
	};

private:
	std::vector<char> lc_id_;
	std::vector<char> hc_id_;
	std::vector<char> data_buffer_;
	status status_;
	data_base *db_;
	high_rank_clients::server *high_rank_server_;
	Net::local_communicator_manager *local_comm_manager_;
	std::string link_to_hs_;
};

class server : public Net::server
{
public:
	static const std::string c_local_communication_prefix_;

public:
	server(data_base *db,
		Net::net_manager *net_manager,
		high_rank_clients::server *high_rank_server,
		Net::local_communicator_manager *local_comm_manager,
		int port, bool nonblocking, bool no_nagle_delay);
	~server();

public: // Net::server
	virtual Net::i_net_member* create_connection(int socket);

private:
	data_base *db_;
	high_rank_clients::server *high_rank_server_;
	Net::local_communicator_manager *local_comm_manager_;
};

} // low_rank_clients

#endif // _LOW_CLIENT_SERVER_
#include "stdafx.h"

#include "HighRankClients.h"

#include "DataBase.h"

namespace high_rank_client
{

server_connection::server_connection(data_base *db, int socket)
	: Net::connection(socket, Net::c_poll_event_in)
	, status_(status_not_logined)
	, db_(db)
{
}

server_connection::~server_connection()
{
	id_.clear();
	data_buffer_.clear();
}

int server_connection::process_events(short int polling_events)
{
	if (polling_events & Net::c_poll_event_in)
	{
		if (status_ == status_not_logined)
		{
			
		}
		else
		{
			
		}
	}

	return Net::error_no_;
}

///////////////////////////////////////////////////////////////////////////

server::server(data_base *db,
	Net::net_manager *net_manager, int port,
	bool nonblocking, bool no_nagle_delay)
	: Net::server(net_manager, port, nonblocking, no_nagle_delay)
	, db_(db)
{
}
server::~server()
{
}

Net::i_net_member* server::create_connection(int socket)
{
	server_connection* new_connection = new server_connection(db_, socket);
	return new_connection;
}

} // high_rank_client

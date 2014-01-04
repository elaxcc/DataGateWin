#include "stdafx.h"

#include "LowRankClients.h"
#include "Protocol.h"

namespace low_rank_clients
{

server_connection::server_connection(int socket)
	: Net::connection(socket, Net::c_poll_event_in)
	, status_(status_not_logined)
{

}

server_connection::~server_connection()
{
	full_id_.clear();
}

int server_connection::process_events(short int polling_events)
{
	if (polling_events == Net::c_poll_event_in)
	{
		char login_data_buffer[1024];
		int incoming_data_size = 0;
		int result = Net::recv_data(get_socket(), login_data_buffer,
			sizeof(login_data_buffer), &incoming_data_size);

		if (result != Net::error_no_)
		{
			// some problems occurs
			return result;
		}

		data_buffer_.insert(data_buffer_.end(), login_data_buffer,
			login_data_buffer + incoming_data_size);

		if (status_ == status_not_logined)
		{
			// try to login

			if (data_buffer_.size() < LC_LONIG_PACKET_LEN)
			{
				// didn't get all login data, wait for next recv()
				return Net::error_no_;
			}
			else
			{
				//!fixme check full ID in database
				// if it wrong close connection, else login is ok
				
				full_id_.insert(full_id_.begin(), data_buffer_.begin(),
					data_buffer_.begin() + LC_LONIG_PACKET_LEN)
				data_buffer_.erase(data_buffer_.begin(),
					data_buffer_.begin() + LC_LONIG_PACKET_LEN);

				status_ = status_logined;
				
				return Net::error_no_;
			}
		}
		else
		{
			//!fixme send data to HC
		}
	}

	return Net::error_no_;
}

///////////////////////////////////////////////////////////////////////////

server_connection::data_parser::data_parser()
{

}

server_connection::data_parser::~data_parser()
{
	buffer_.clear();
}

void server_connection::data_parser::parse(
	char *data, unsigned int data_len)
{
	buffer_.insert(buffer_.end(), data, data + data_len);
}

///////////////////////////////////////////////////////////////////////////

server::server(Net::net_manager *net_manager,
	int port, bool nonblocking, bool no_nagle_delay)
	: Net::server(net_manager, port, nonblocking, no_nagle_delay)
{
}

server::~server()
{
}

Net::i_net_member* server::create_connection(int socket)
{
	server_connection *new_connection = new server_connection(socket);
	return new_connection;
}

} // low_rank_clients


#include "stdafx.h"

#include "HighRankClients.h"

#include "DataBase.h"
#include "Protocol.h"

namespace high_rank_clients
{

server_connection::server_connection(data_base *db, int socket,
	server *own_server)
	: Net::connection(socket, Net::c_poll_event_in)
	, status_(status_not_logined)
	, db_(db)
	, own_server_(own_server_)
{
}

server_connection::~server_connection()
{
	hc_id_.clear();
	data_buffer_.clear();
}

int server_connection::process_events(short int polling_events)
{
	int recv_result = Net::error_no_;

	if (polling_events & Net::c_poll_event_in)
	{
		while (recv_result == Net::error_no_)
		{
			char login_data_buffer[4096];
			int incoming_data_size = 0;

			recv_result = Net::recv_data(get_socket(), login_data_buffer,
				sizeof(login_data_buffer), &incoming_data_size);

			if (incoming_data_size > 0)
			{
				data_buffer_.insert(data_buffer_.end(), login_data_buffer,
					login_data_buffer + incoming_data_size);
			}
		}

		if (status_ == status_not_logined)
		{
			// if get all login data, wait for next recv()
			if (data_buffer_.size() >= USER_ID_LEN)
			{
				hc_id_.insert(hc_id_.begin(), data_buffer_.begin(),
					data_buffer_.begin() + USER_ID_LEN);

				data_buffer_.erase(data_buffer_.begin(),
					data_buffer_.begin() + USER_ID_LEN);

				bool is_exist = db_->check_user_id(std::string(hc_id_.front(), USER_ID_LEN));

				if (is_exist)
				{
					own_server_->register_client(std::string(hc_id_.front(), hc_id_.size()), this);
					status_ = status_logined;
				}
				else
				{
					close();
					return Net::error_connection_is_closed_;
				}
			}
		}
		else
		{
			// send data to lc
			
			if (data_buffer_.size() >= LC_ID_LEN)
			{
				std::string link_to_lc = std::string(hc_id_.front(), hc_id_.size()) +
					std::string(data_buffer_.front(), LC_ID_LEN);
				data_buffer_.erase(data_buffer_.begin(), data_buffer_.begin() + LC_ID_LEN);
				own_server_->send_message(link_to_lc, data_buffer_);
			}
		}
	}

	if (recv_result == Net::error_connection_is_closed_)
	{
		own_server_->unregister_client(std::string(hc_id_.front(), hc_id_.size()));
		return recv_result;
	}
	return Net::error_no_;
}

///////////////////////////////////////////////////////////////////////////

server::server(data_base *db,
	Net::net_manager *net_manager,
	Net::local_communicator_manager *local_comm_manager,
	int port, bool nonblocking, bool no_nagle_delay)
	: Net::server(net_manager, port, nonblocking, no_nagle_delay)
	, Net::i_local_communicator(local_comm_manager)
	, db_(db)
	, local_comm_manager_(local_comm_manager)
{
}

server::~server()
{
	clients_.clear();
}

void server::register_client(const std::string& client_id, server_connection* connection)
{
	if (!client_id.empty())
	{
		clients_.insert(std::make_pair(client_id, connection));
	}
}

void server::unregister_client(const std::string& client_id)
{
	std::map<std::string, server_connection*>::iterator iter =
		clients_.find(client_id);
	if (iter != clients_.end())
	{
		clients_.erase(iter);
	}
}

Net::i_net_member* server::create_connection(int socket)
{
	server_connection* new_connection = new server_connection(db_, socket, this);
	return new_connection;
}

int server::process_message(const std::string& link,
	const std::vector<char>& data)
{
	std::string client_id(link.substr(0, USER_ID_LEN));
	std::map<std::string, server_connection*>::iterator iter =
		clients_.find(client_id);
	if (iter != clients_.end())
	{
		Net::send_data(iter->second->get_socket(), (char *) link.substr(USER_ID_LEN).c_str(), LC_ID_LEN);
		Net::send_data(iter->second->get_socket(), (char *) &data[0], data.size());
	}

	return 0;
}

} // high_rank_client

#include "stdafx.h"

#include "LowRankClients.h"
#include "HighRankClients.h"
#include "Protocol.h"
#include "DataBase.h"

namespace low_rank_clients
{

server_connection::server_connection(data_base *db,
	high_rank_clients::server *high_rank_server,
	Net::local_communicator_manager *local_comm_manager,
	int socket)
	: Net::connection(socket, Net::c_poll_event_in)
	, Net::i_local_communicator(local_comm_manager)
	, status_(status_not_logined)
	, db_(db)
	, high_rank_server_(high_rank_server)
	, local_comm_manager_(local_comm_manager)
{
}

server_connection::~server_connection()
{
	lc_id_.clear();
	hc_id_.clear();
	data_buffer_.clear();

	local_comm_manager_->destroy_link(link_to_hs_);
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
			// try to login

			// if get all login data, wait for next recv()
			if (data_buffer_.size() >= LC_LONIG_PACKET_LEN)
			{
				// check full ID in database
				// if it wrong close connection, else login is ok

				hc_id_.insert(hc_id_.begin(), data_buffer_.begin(),
					data_buffer_.begin() + USER_ID_LEN);
				lc_id_.insert(lc_id_.begin(), &(data_buffer_[USER_ID_LEN]),
					&(data_buffer_[USER_ID_LEN]) + LC_ID_LEN);

				bool is_exist = db_->check_lc_id(std::string(&hc_id_[0], USER_ID_LEN),
					std::string(&lc_id_[0], LC_ID_LEN));

				if (is_exist)
				{
					link_to_hs_ = std::string(&hc_id_[0], hc_id_.size()) +
						std::string(&lc_id_[0], lc_id_.size());
					local_comm_manager_->create_link(link_to_hs_, this, high_rank_server_);
					status_ = status_logined;

					Net::send_data(get_socket(), &data_buffer_[0], LC_LONIG_PACKET_LEN);

					data_buffer_.erase(data_buffer_.begin(),
						data_buffer_.begin() + LC_LONIG_PACKET_LEN);
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
			// send data to HC

			send_message(link_to_hs_, data_buffer_);

			data_buffer_.clear();
		}
	}

	if (recv_result == Net::error_connection_is_closed_)
	{
		// some problems occurs
		return recv_result;
	}
	return Net::error_no_;
}

int server_connection::process_message(const std::string& link,
	const std::vector<char>& data)
{
	int send_result = Net::send_data(get_socket(), (char *) &data[0], data.size());
	return 0;
}

///////////////////////////////////////////////////////////////////////////

const std::string server::c_local_communication_prefix_ = "lc:";

server::server(data_base *db,
	Net::net_manager *net_manager,
	high_rank_clients::server *high_rank_server,
	Net::local_communicator_manager *local_comm_manager,
	int port, bool nonblocking, bool no_nagle_delay)
	: Net::server(net_manager, port, nonblocking, no_nagle_delay)
	, db_(db)
	, high_rank_server_(high_rank_server)
	, local_comm_manager_(local_comm_manager)
{
}

server::~server()
{
}

Net::i_net_member* server::create_connection(int socket)
{
	server_connection *new_connection = new server_connection(
		db_, high_rank_server_, local_comm_manager_, socket);
	return new_connection;
}

} // low_rank_clients


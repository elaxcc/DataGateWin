#include "stdafx.h"

#include "HighRankClients.h"

#include "DataBase.h"
#include "Protocol.h"

namespace high_rank_clients
{

namespace
{

unsigned int login_passwd_crc_len_size = 4;

}

server_connection::server_connection(data_base *db, int socket,
	server *own_server)
	: Net::connection(socket, Net::c_poll_event_in)
	, status_(status_not_logined)
	, db_(db)
	, own_server_(own_server)
{
}

server_connection::~server_connection()
{
	data_buffer_.clear();
	own_server_->unregister_client(hc_id_);
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
			login_parser_.parse(data_buffer_);
			data_buffer_.clear();

			if (login_parser_.is_complete())
			{
				hc_id_ = db_->get_user_id_by_login(
					login_parser_.get_login(), login_parser_.get_passwd());

				if (hc_id_.empty())
				{
					return Net::error_connection_is_closed_;
				}

				status_ = status_logined;
				own_server_->register_client(hc_id_, this);

				// send answer
				Net::send_data(get_socket(), (char *) g_hs_login_answer.c_str(),
					g_hs_login_answer.size());
			}
		}
		else
		{
			// send data to lc
			
			if (data_buffer_.size() >= LC_ID_LEN)
			{
				hc_to_lc_parser_.parse(data_buffer_);
				data_buffer_.clear();

				if (hc_to_lc_parser_.is_complete() && 
					!hc_to_lc_parser_.is_bad_packet())
				{

					const std::vector<char>& incoming_lc_id =
						hc_to_lc_parser_.get_lc_id();
					std::string link_to_lc = hc_id_ +
						std::string(&incoming_lc_id[0], + incoming_lc_id.size());

					own_server_->send_message(link_to_lc, 
						hc_to_lc_parser_.get_data());
				}
				hc_to_lc_parser_.flush();
			}
		}
	}

	if (recv_result == Net::error_connection_is_closed_)
	{
		return recv_result;
	}
	return Net::error_no_;
}

///////////////////////////////////////////////////////////////////////////

void server_connection::login_parser::parse(const std::vector<char>& data)
{
	if (!is_complete_)
	{
		buffer_.insert(buffer_.end(), data.begin(), data.end());
		buffer_all_data_.insert(buffer_all_data_.end(), data.begin(), data.end());

		if (!got_login_len_)
		{
			if (buffer_.size() < login_passwd_crc_len_size)
			{
				return;
			}

			login_len_ = 0x000000FF & buffer_[0];
			login_len_ = login_len_ | (0x0000FF00 & (buffer_[1] << 8));
			login_len_ = login_len_ | (0x00FF0000 & (buffer_[2] << 16));
			login_len_ = login_len_ | (0xFF000000 & (buffer_[3] << 24));

			buffer_.erase(buffer_.begin(), buffer_.begin() + login_passwd_crc_len_size);

			got_login_len_ = true;
		}

		if (!got_login_)
		{
			if (buffer_.size() < login_len_)
			{
				return;
			}

			login_.insert(login_.end(), buffer_.begin(), buffer_.begin() + login_len_);
			buffer_.erase(buffer_.begin(), buffer_.begin() + login_len_);

			got_login_ = true;
		}

		if (!got_passwd_len_)
		{
			if (buffer_.size() < login_passwd_crc_len_size)
			{
				return;
			}

			passwd_len_ = 0x000000FF & buffer_[0];
			passwd_len_ = passwd_len_ | (0x0000FF00 & (buffer_[1] << 8));
			passwd_len_ = passwd_len_ | (0x00FF0000 & (buffer_[2] << 16));
			passwd_len_ = passwd_len_ | (0xFF000000 & (buffer_[3] << 24));

			buffer_.erase(buffer_.begin(), buffer_.begin() + login_passwd_crc_len_size);

			got_passwd_len_ = true;
		}

		if (!got_passwd_)
		{
			if (buffer_.size() < passwd_len_)
			{
				return;
			}

			passwd_.insert(passwd_.end(), buffer_.begin(), buffer_.begin() + passwd_len_);
			buffer_.erase(buffer_.begin(), buffer_.begin() + passwd_len_);

			got_passwd_ = true;
		}

		if (!got_crc_)
		{
			if (buffer_.size() < login_passwd_crc_len_size)
			{
				return;
			}

			crc_ = 0x000000FF & buffer_[0];
			crc_ = crc_ | (0x0000FF00 & (buffer_[1] << 8));
			crc_ = crc_ | (0x00FF0000 & (buffer_[2] << 16));
			crc_ = crc_ | (0xFF000000 & (buffer_[3] << 24));

			boost::uint32_t calculated_crc = Crc32((const unsigned char*) &buffer_all_data_[0],
				buffer_all_data_.size() - login_passwd_crc_len_size);

			got_crc_ = true;

			if (crc_ == calculated_crc)
			{
				buffer_.clear();
				buffer_all_data_.clear();

				is_complete_ = true;
			}
			else
			{
				got_login_len_ = true;
				got_login_ = true;
				got_passwd_len_ = true;
				got_passwd_ = true;
				got_crc_ = true;
			}
		}
	}
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
	if (iter != clients_.end() && iter->second && iter->second->is_logined())
	{
		Net::send_data(iter->second->get_socket(), (char *) &data[0], data.size());
	}
	else
	{
		//!fixme save data to DB
		//!fixme do something if not logined
	}

	return 0;
}

} // high_rank_client

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

	class login_parser
	{
	public:
		login_parser()
			: is_complete_(false)
			, got_login_len_(false)
			, got_login_(false)
			, got_passwd_len_(false)
			, got_passwd_(false)
			, got_crc_(false)
		{}
		~login_parser()
		{
			buffer_.clear();
			buffer_all_data_.clear();
		}

		bool is_complete() { return is_complete_; }
		void parse(const std::vector<char>& data);

		std::string get_login() { return login_; }
		std::string get_passwd() { return passwd_; }
	private:
		std::vector<char> buffer_;
		std::vector<char> buffer_all_data_;

		unsigned int login_len_;
		std::string login_;
		unsigned int passwd_len_;
		std::string passwd_;
		boost::uint32_t crc_;

		bool is_complete_;

		bool got_login_len_;
		bool got_login_;
		bool got_passwd_len_;
		bool got_passwd_;
		bool got_crc_;
	};

private:
	std::string hc_id_;
	status status_;
	std::vector<char> data_buffer_;
	data_base *db_;
	server *own_server_;
	login_parser login_parser_;
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
#ifndef _LOW_CLIENT_SERVER_
#define _LOW_CLIENT_SERVER_

#include "NetCommon.h"
#include "LocalCommunication.h"

class data_base;

namespace low_rank_clients
{

	class server_connection : public Net::connection, public Net::i_local_communicator
{
public:
	server_connection(data_base *db,
		Net::local_communicator_manager *local_comm_manager,
		const std::string& local_communication_link, int socket);
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

	class data_parser
	{
	public:
		data_parser();
		~data_parser();

		void parse(char *data, unsigned int data_len);

	private:
		std::vector<char> buffer_;
	};

private:
	std::vector<char> full_id_;
	std::vector<char> data_buffer_;
	status status_;
	data_base *db_;
	std::string local_communication_link_;
};

class server : public Net::server
{
public:
	static const std::string c_local_communication_prefix_;

public:
	server(data_base *db,
		Net::local_communicator_manager *local_comm_manager,
		Net::net_manager *net_manager,
		int port, bool nonblocking, bool no_nagle_delay);
	~server();

public: // Net::server
	virtual Net::i_net_member* create_connection(int socket);

private:
	data_base *db_;
	Net::local_communicator_manager *local_comm_manager_;
};

} // low_rank_clients

#endif // _LOW_CLIENT_SERVER_
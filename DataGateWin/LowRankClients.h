#ifndef _LOW_CLIENT_SERVER_
#define _LOW_CLIENT_SERVER_

#include "NetCommon.h"

class data_base;

namespace low_rank_clients
{

class server_connection : public Net::connection
{
public:
	server_connection(data_base *db, int socket);
	~server_connection();

public: // Net::connection
	virtual int process_events(short int polling_events);

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
};

class server : public Net::server
{
public:
	server(data_base *db,
		Net::net_manager *net_manager,
		int port, bool nonblocking, bool no_nagle_delay);
	~server();

public: // Net::server
	virtual Net::i_net_member* create_connection(int socket);

private:
	data_base *db_;
};

} // low_rank_clients

#endif // _LOW_CLIENT_SERVER_
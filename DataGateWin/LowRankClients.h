#ifndef _LOW_CLIENT_SERVER_
#define _LOW_CLIENT_SERVER_

#include "NetCommon.h"

namespace low_rank_clients
{

class server_connection : public Net::connection
{
public:
	server_connection(int socket);
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
};

class server : public Net::server
{
public:
	server(Net::net_manager *net_manager, int port, bool nonblocking, bool no_nagle_delay);
	~server();

public: // Net::server
	virtual Net::i_net_member* create_connection(int socket);
};

} // low_rank_clients

#endif // _LOW_CLIENT_SERVER_
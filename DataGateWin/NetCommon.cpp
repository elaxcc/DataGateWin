#include "stdafx.h"

#include "NetCommon.h"

#include "NetManager.h"

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <netinet/tcp.h>
	#include <poll.h>
	#include <errno.h>
	#include <string.h>
#elif defined WIN || WIN32 || WIN64
	#include <windows.h>
#endif

namespace Net
{

namespace
{

const int c_client_queue_size = 5;

}

/*!
 * common client
 */

simple_client::simple_client(bool nonblocking, bool no_nagle_delay)
	: socket_(-1)
	, nonblocking_(nonblocking)
	, no_nagle_delay_(no_nagle_delay)
{
}

simple_client::~simple_client()
{
	if (socket_ > 0)
	{
#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
		shutdown(socket_, SHUT_RDWR);
		close(socket_);
#elif defined WIN || WIN32 || WIN64
		closesocket(socket_);
#endif
	}
}

int simple_client::connect_to(const std::string& address, int port)
{

	address_.sin_family = AF_INET;
	address_.sin_port = htons(port);
	address_.sin_addr.s_addr = inet_addr(address.c_str());


	// отключить алгоритм нагла

	socket_ = socket(AF_INET, SOCK_STREAM, 0);
#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (socket_ < 0)
#elif defined WIN || WIN32 || WIN64
	if (socket_ == INVALID_SOCKET)
#endif
	{
		return error_create_socket_;
	}


	if (no_nagle_delay_)
	{
#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
		int set_opt_val = 1;
#elif defined WIN || WIN32 || WIN64
		char set_opt_val = 1;
#endif

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
		if (setsockopt(socket_, SOL_TCP, TCP_NODELAY,
			&set_opt_val, sizeof(set_opt_val)) < 0)
#elif defined WIN || WIN32 || WIN64
		if (setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY,
			&set_opt_val, sizeof(set_opt_val)) != 0)
#endif
		{
			return error_set_opt_nodelay_;
		}
	}

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (nonblocking_)
	{
		if (fcntl(socket_, F_SETFL, O_NONBLOCK) < 0)
		{
			return error_create_nonblock_socket_;
		}
	}
#elif defined WIN || WIN32 || WIN64
	if (nonblocking_)
	{
		u_long iMode = 1;
		if (ioctlsocket(socket_, FIONBIO, &iMode) != 0)
		{
			return error_create_nonblock_socket_;
		}
	}
#endif

	int connect_result = connect(socket_, (struct sockaddr *) &address_,
			sizeof(struct sockaddr));
#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (nonblocking_ && connect_result == -1 && errno != EINPROGRESS)
#elif defined WIN || WIN32 || WIN64
	if (connect_result != 0)
#endif
	{
		return error_connect_;
	}
	return error_no_;
}

int simple_client::close_connection()
{
	int result;

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	result = shutdown(socket_, SHUT_RDWR);
	close(socket_);
#elif defined WIN || WIN32 || WIN64
	result = closesocket(socket_);
#endif

	socket_ = -1;

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (result < 0)
#elif defined WIN || WIN32 || WIN64
	if (result != 0)
#endif
	{
		return error_shutdown_;
	}
	return error_no_;
}

int simple_client::get_socket() const
{
	return socket_;
}

/*!
 * common server
 */

simple_server::simple_server(bool nonblocking, bool no_nagle_delay)
	: socket_(-1)
	, nonblocking_(nonblocking)
	, no_nagle_delay_(no_nagle_delay)
{
}

simple_server::~simple_server()
{
	if (socket_ > 0)
	{
#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
		shutdown(socket_, SHUT_RDWR);
		close(socket_);
#elif defined WIN || WIN32 || WIN64
		closesocket(socket_);
#endif
	}
}

int simple_server::start_listen(int port)
{

	address_.sin_family = AF_INET;
	address_.sin_port = htons(port);
	address_.sin_addr.s_addr = htonl(INADDR_ANY);

	socket_ = socket(AF_INET, SOCK_STREAM, 0);
#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (socket_ < 0)
#elif defined WIN || WIN32 || WIN64
	if (socket_ == INVALID_SOCKET)
#endif
	{
		return error_create_socket_;
	}

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	int set_opt_val = 1;
#elif defined WIN || WIN32 || WIN64
	char set_opt_val = 1;
#endif

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
			&set_opt_val, sizeof(set_opt_val)) < 0)
#elif defined WIN || WIN32 || WIN64
	if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
		&set_opt_val, sizeof(set_opt_val)) != 0)
#endif
	{
		return error_set_opt_reuse_addr_;
	}

	if (no_nagle_delay_)
	{
		set_opt_val = 1;
#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
		if (setsockopt(socket_, SOL_TCP, TCP_NODELAY,
			&set_opt_val, sizeof(set_opt_val)) < 0)
#elif defined WIN || WIN32 || WIN64
		if (setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY,
			&set_opt_val, sizeof(set_opt_val)) != 0)
#endif
		{
			return error_set_opt_nodelay_;
		}
	}

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (nonblocking_)
	{
		if (fcntl(socket_, F_SETFL, O_NONBLOCK) < 0)
		{
			return error_create_nonblock_socket_;
		}
	}
#elif defined WIN || WIN32 || WIN64
	if (nonblocking_)
	{
		u_long iMode = 1;
		if (ioctlsocket(socket_, FIONBIO, &iMode) != 0)
		{
			return error_create_nonblock_socket_;
		}
	}
#endif

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (bind(socket_, (struct sockaddr*) &address_,
			sizeof(struct sockaddr)) < 0)
#elif defined WIN || WIN32 || WIN64
	if (bind(socket_, (struct sockaddr*) &address_,
		sizeof(struct sockaddr)) != 0)
#endif
	{
		return error_binding_;
	}

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (listen(socket_, c_client_queue_size) < 0)
#elif defined WIN || WIN32 || WIN64
	if (listen(socket_, c_client_queue_size) != 0)
#endif
	{
		return error_start_listen_;
	}

	return error_no_;
}

int simple_server::stop_listen()
{
	int result;

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	result = shutdown(socket_, SHUT_RDWR);
	close(socket_);
#elif defined WIN || WIN32 || WIN64
	result = closesocket(socket_);
#endif

	socket_ = -1;

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (result < 0)
#elif defined WIN || WIN32 || WIN64
	if (result != 0)
#endif
	{
		return error_shutdown_;
	}
	return error_no_;
}

int simple_server::client_accept(int *client_socket,
	struct sockaddr_in *client_addr) const
{
#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	unsigned int addr_size = sizeof(struct sockaddr);
#elif defined WIN || WIN32 || WIN64
	int addr_size = sizeof(struct sockaddr);
#endif
	*client_socket = accept(socket_, (struct sockaddr*) client_addr,
			&addr_size);

#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	if (*client_socket < 0)
#elif defined WIN || WIN32 || WIN64
	if (*client_socket == INVALID_SOCKET)
#endif
	{
		return error_client_accept_;
	}
	return error_no_;
}

int simple_server::get_socket() const
{
	return socket_;
}

/*!
 * common function
 */

int send_data(int socket, char *data, int data_size)
{
	int sended_bytes;
		int total_bytes = data_size;

	while(total_bytes)
	{
#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
		sended_bytes = send(
				socket,
				&(data[data_size - total_bytes]),
				total_bytes, MSG_NOSIGNAL);
#elif defined WIN || WIN32 || WIN64
		sended_bytes = send(
			socket,
			&(data[data_size - total_bytes]),
			total_bytes, 0);
#endif

		if (sended_bytes < 0)
		{
			return error_send_;
		}

		total_bytes -= sended_bytes;
	}
	return error_no_;
}

int recv_data(int socket, char *buf, int buf_size, int *recv_data_size)
{

	*recv_data_size = recv(socket, buf, buf_size, 0);

	if (*recv_data_size == 0)
	{
		return error_connection_is_closed_;
	}
	else if (*recv_data_size < 0)
	{
		return error_recv_;
	}
	return error_no_;
}

#if defined WIN || WIN32 || WIN64
WSADATA wsadata_;
bool init()
{
	return WSAStartup(MAKEWORD(2, 0), &wsadata_) == 0;
}
#endif

} // Net

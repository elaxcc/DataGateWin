#include "stdafx.h"

#include "NetManager.h"

#include "NetCommon.h"

namespace Net
{

namespace
{

const int c_infinum_timeout = -1;

}

net_manager::net_manager()
{
}

net_manager::~net_manager()
{
	for (unsigned int i = 0; i < net_members_.size(); ++i)
	{
		delete net_members_[i];
	}

	net_members_.clear();
	polling_list_.clear();
}

int net_manager::add_member(i_net_member *member)
{
	if (member)
	{
		net_members_.push_back(member);

		pollfd fd = {member->get_socket(), 0, 0};
		polling_list_.push_back(fd);
	}

	return member->get_socket();
}

void net_manager::remove_member(int socket)
{
	std::vector<i_net_member*>::iterator member_iter =
		net_members_.begin();
	for(; member_iter != net_members_.end(); ++member_iter)
	{
		if ((*member_iter)->get_socket() == socket)
		{
			delete (*member_iter);
			net_members_.erase(member_iter);
			remove_from_polling_list(socket);

			return;
		}
	}
}

int net_manager::process_sockets()
{
	int poll_result = poll_sockets(c_infinum_timeout);

	std::vector<int> members_for_delete; // store for disconnected net members

	if (poll_result == error_no_)
	{
		// process net members and remember who is disconnected
		unsigned members_cnt = polling_list_.size();
		for (unsigned int i = 0; i < members_cnt; ++i)
		{
			if (polling_list_[i].revents & POLLHUP)
			{
				members_for_delete.push_back(
					net_members_[i]->get_socket());

				continue;
			}

			short int member_polling_flags =
				net_members_[i]->get_polling_flags();
			if ((member_polling_flags != 0)
				&& (polling_list_[i].revents & member_polling_flags))
			{
				int process_result =
					net_members_[i]->process_events(polling_list_[i].revents);
				if (process_result == error_connection_is_closed_)
				{
					members_for_delete.push_back(
						net_members_[i]->get_socket());
				}
			}
		}

		// remove disconnected net members from net members list
		if (!members_for_delete.empty())
		{
			for (unsigned int i = 0; i < members_for_delete.size(); ++i)
			{
				remove_member(members_for_delete[i]);
			}
		}
	}

	return poll_result;
}

int net_manager::members_cnt()
{
	return net_members_.size();
}

int net_manager::poll_sockets(int timeout)
{
	for (unsigned int i = 0; i < polling_list_.size(); ++i)
	{
		polling_list_[i].events = net_members_[i]->get_polling_flags();
		polling_list_[i].revents = 0;
	}

	int result;
#if defined LINUX || UNIX || linux || unix || __linux || __linux__ || __unix || __unix__ || __gnu_linux__
	result = poll(polling_list_.data(),
		polling_list_.size(), timeout);
#elif defined WIN || WIN32 || WIN64
	result = WSAPoll(&(*polling_list_.begin()),
		polling_list_.size(), timeout);
#endif
	

	if (result == 0)
	{
		return error_poll_timeout_;
	}
	else if (result < 0)
	{
		return error_poll_;
	}
	return error_no_;
}

void net_manager::remove_from_polling_list(int socket)
{
	std::vector<pollfd>::iterator iter = polling_list_.begin();
	for (; iter != polling_list_.end(); ++iter)
	{
		if (iter->fd == socket)
		{
			polling_list_.erase(iter);
			return;
		}
	}
}

} // nemaspace Net

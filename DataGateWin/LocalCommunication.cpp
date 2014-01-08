#include "stdafx.h"

#include "LocalCommunication.h"

namespace Net
{

i_local_communicator::i_local_communicator(
	local_communicator_manager *manager)
	: id_(0)
	, links_cnt_(0)
	, manager_(manager)
{
}

i_local_communicator::~i_local_communicator()
{
}

int i_local_communicator::get_id() const
{
	return id_;
}

void i_local_communicator::send_message(const std::string& link,
	const std::vector<char>& message)
{
	manager_->add_message(link, id_, message);
}

void i_local_communicator::set_id(int id)
{
	id_ = id;
}

local_communicator_manager::local_communicator_manager()
	: last_added_member_id_(0)
{
}

local_communicator_manager::~local_communicator_manager()
{
	messages_.clear();
	members_.clear();
	links_list_.clear();
}

void local_communicator_manager::add_communicator(
	i_local_communicator *member)
{
	if (member)
	{
		std::map<int, i_local_communicator*>::iterator iter =
			members_.find(member->get_id());
		if (iter == members_.end())
		{
			last_added_member_id_++;
			member->set_id(last_added_member_id_);

			members_.insert(
				std::pair<int, i_local_communicator*>(
						member->get_id(), member));
		}
	}
}

bool local_communicator_manager::create_link(
	const std::string& link,
	i_local_communicator* member_first,
	i_local_communicator* member_second)
{
	std::map<std::string, real_link>::iterator link_iter =
		links_list_.find(link);

	if (link.empty() || !member_first || !member_second
		|| link_iter != links_list_.end())
	{
		return false;
	}

	add_communicator(member_first);
	add_communicator(member_second);

	member_first->links_cnt_++;
	member_second->links_cnt_++;

	links_list_.insert(
		std::pair<std::string, real_link>(
			link, real_link(
				member_first->get_id(),
				member_second->get_id())));

	return true;
}

void local_communicator_manager::destroy_link(
	const std::string& link)
{
	std::map<std::string, real_link>::iterator link_iter =
		links_list_.find(link);
	if (link_iter != links_list_.end())
	{
		std::map<int, i_local_communicator*>::iterator first_iter =
			members_.find(link_iter->second.first_);
		first_iter->second->links_cnt_--;
		if (first_iter->second || first_iter->second->links_cnt_ == 0)
		{
			members_.erase(first_iter);
		}

		std::map<int, i_local_communicator*>::iterator second_iter =
			members_.find(link_iter->second.second_);
		second_iter->second->links_cnt_--;
		if (second_iter->second || second_iter->second->links_cnt_ == 0)
		{
			members_.erase(second_iter);
		}

		links_list_.erase(link_iter);
	}
}

void local_communicator_manager::add_message(
	const std::string& link, int source_id,
	const std::vector<char>& message)
{
	std::map<std::string, real_link>::iterator link_iter =
		links_list_.find(link);
	if (link_iter == links_list_.end())
	{
		return;
	}

	messages_.push_back(
		message_info(
			link,
			source_id,
			message));
}

void local_communicator_manager::process()
{
	unsigned int current_messages_cnt_for_process = messages_.size();
	if (!current_messages_cnt_for_process)
	{
		return;
	}

	for (unsigned int i = 0; i < current_messages_cnt_for_process; ++i)
	{
		std::map<std::string, real_link>::iterator link_iter =
			links_list_.find(messages_[i].link_);
		if (link_iter == links_list_.end())
		{
			continue;
		}

		int destination_id = 0;
		if (link_iter->second.first_ == messages_[i].source_id_)
		{
			destination_id = link_iter->second.second_;
		}
		else
		{
			destination_id = link_iter->second.first_;
		}

		std::map<int, i_local_communicator*>::iterator member_iter =
			members_.find(destination_id);

		if (member_iter->second)
		{
			member_iter->second->process_message(
				messages_[i].link_,	messages_[i].message_);
		}
		else
		{
			destroy_link(link_iter->first);
		}
	}

	messages_.erase(messages_.begin(),
		messages_.begin() + current_messages_cnt_for_process);
}

} // namespace Net

#ifndef _TIMER_H_
#define _TIMER_H_

#include "ThreadWithStop.h"

namespace Net
{

class i_timer_member
{
public:
	i_timer_member()
		: id_(0)
	{
	}

	virtual int process() = 0;

private:
	friend class timer;

private:
	int id_;
};

class timer
{
public:
	timer(unsigned long m_sec);
	~timer();
	void add_member(i_timer_member *member);
	void remove_member(i_timer_member *member);
	void process();
	void start();
	void stop();

public:
	std::map<int, i_timer_member*> members_;
	int last_setted_id_;
	int ms_delay_;
	boost::scoped_ptr<ThreadWithStop> thread_;
	mutable boost::shared_mutex mutex_;
};

} // namespace Net

#endif // _TIMER_H_
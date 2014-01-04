#include "stdafx.h"

#include "ThreadWithStop.h"

ThreadWithStop::ThreadWithStop(const Callback& callback)
: need_stop_(false)
, callback_(callback)
{
}

ThreadWithStop::~ThreadWithStop()
{
	stop();
	join();
}

bool ThreadWithStop::need_stop() const
{
	boost::unique_lock<boost::shared_mutex> locker(mutex_);
	return need_stop_;
}

void ThreadWithStop::start()
{
	thread_.reset(new boost::thread(
		boost::bind(&ThreadWithStop::thread_function, this)));
}

void ThreadWithStop::stop()
{
	boost::unique_lock<boost::shared_mutex> locker(mutex_);
	need_stop_ = true;
}

bool ThreadWithStop::is_complete() const
{
	if (thread_->joinable())
	{
		return thread_->timed_join(boost::posix_time::microsec(0));
	}
	return false;
}

void ThreadWithStop::join() const
{
	if (thread_->joinable())
	{
		thread_->join();
	}
}

boost::thread::id ThreadWithStop::get_id()
{
	boost::thread::id thread_id = thread_->get_id();
	return thread_id;
}

void ThreadWithStop::thread_function()
{
	boost::unique_lock<boost::shared_mutex> locker(mutex_);
	bool thread_need_stop = need_stop_;
	locker.unlock();

	callback_(thread_need_stop);
}

#pragma once

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/detail/thread.hpp>

class ThreadWithStop
{
public:
	typedef boost::function<void(bool)> Callback;

public:
	ThreadWithStop(const Callback& callback);
	~ThreadWithStop();

	void start();
	void stop();
	bool need_stop() const;
	bool is_complete() const;
	void join() const;
	boost::thread::id get_id();

private:
	void thread_function();

private:
	mutable boost::shared_mutex mutex_;
	boost::scoped_ptr<boost::thread> thread_;
	Callback callback_;
	bool need_stop_;
};

#include "stdafx.h"
#include "sub_process_mgr.h"

sub_process_mgr::sub_process_mgr(const std::string & sub_process_path)
	: sub_process_path_(sub_process_path)
{

}

sub_process_mgr::~sub_process_mgr()
{
	stop();
}

bool sub_process_mgr::start()
{
	if (!running_) {
		auto ret = jlib::daemon(utf8::a2w(sub_process_path_), false);
		if (ret == 0) {
			running_ = true;
			feed_watch_dog();
		}
	}

	return running_;
}

bool sub_process_mgr::stop()
{
	if (running_) {
		std::string cmd;
		cmd = "taskkill /F /IM " + sub_process_path_;
		std::system(cmd.c_str());
		running_ = false;
	}

	return true;
}

bool sub_process_mgr::restart()
{
	stop();
	return start();
}

void sub_process_mgr::feed_watch_dog()
{
	last_time_feed_ = std::chrono::steady_clock::now();
}

long long sub_process_mgr::get_elapsed_ms() const
{
	auto now = std::chrono::steady_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time_feed_);
	return diff.count();
}

#pragma once

class sub_process_mgr : public boost::noncopyable
{
public:
	explicit sub_process_mgr(const std::string& sub_process_path);
	virtual ~sub_process_mgr();

	bool running() const { return running_; }
	bool start();
	bool stop();
	bool restart();

	void feed_watch_dog();
	long long get_elapsed_ms() const;


private:
	std::chrono::steady_clock::time_point last_time_feed_ = {};
	bool running_ = false;
	std::string sub_process_path_ = "";
};

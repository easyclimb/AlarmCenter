#pragma once


#include <string>
#include <list>
#include <mutex>
#include <memory>


typedef struct update_progress {
	int progress_;
	int total_;
	std::wstring msg_;

	update_progress(int progress, int total, const std::wstring& msg)
		: progress_(progress), total_(total), msg_(msg)
	{}

}update_progress;

typedef std::shared_ptr<update_progress> up_ptr;
typedef std::list<up_ptr> up_list;

class update_tool
{
public:
	update_tool();
	~update_tool();

	void start();
	void add_up(const up_ptr& up);
	void add_up(int progress, int total, const std::wstring& msg);
	up_ptr get_up();

private:
	std::mutex mutex_;
	up_list ups_;

};


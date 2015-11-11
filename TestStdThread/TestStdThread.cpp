// TestStdThread.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <process.h>
#include <string>
#include <cstdio>

class print_function
{
private:
	std::string func_name_;
	int tid;
public:
	print_function(const char* func_name) : func_name_(func_name) {
		tid = ::_getpid();
		std::cout << "tid:" << tid << " " << func_name_ << " in" << std::endl;
	}
	~print_function() { std::cout << "tid:" << tid << " " << func_name_ << " out" << std::endl; }
};

#define PRINT_FUNCTION print_function _func_##__LINE__(__func__);


std::condition_variable g_cond;
std::mutex g_mutex;

void func()
{
	PRINT_FUNCTION;
	while (true) {
		std::unique_lock<std::mutex> lock(g_mutex);
		g_cond.wait_for(lock, std::chrono::microseconds(1));
	}
}

int main()
{
	PRINT_FUNCTION;
	std::thread t(func);
	std::
	t.join();
    return 0;
}


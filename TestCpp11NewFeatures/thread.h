#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <string>

#include <boost/noncopyable.hpp>

namespace util {
	class thread : boost::noncopyable
	{
	public:
		typedef std::function<void()> thread_func;
		explicit thread(const thread_func&, const std::string& name = std::string());
		~thread();

		void start();
		void join();
		bool started() const{}

	private:
		bool _started;
		bool _joined;
		std::thread::id _tid;
		thread_func _func;
		std::string _name;

		static std::atomic_int32_t _num_created;
	};
}

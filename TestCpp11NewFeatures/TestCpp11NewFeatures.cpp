// TestCpp11NewFeatures.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../AlarmCenter/observer.h"
#include <iostream>
#include "signal_slot.h"

typedef std::shared_ptr<std::string> target;

class observer : public dp::observer<target>, public std::enable_shared_from_this<observer>
{
public:
	void on_update(const target& target) {
		std::cout << target->c_str() << std::endl;
	}
};



class subject :public dp::observable<target>
{

};

void test_observer()
{
	auto obs = std::make_shared<observer>();
	subject sbj;
	std::weak_ptr<observer> w(obs);
	sbj.register_observer(w);
	sbj.notify_observers(std::make_shared<std::string>("hello world!"));
	obs.reset();
	sbj.notify_observers(std::make_shared<std::string>("are u ok?"));
}


void test_signal_slot() {
	using namespace util;
	auto callback1 = [](const std::string& str) {std::cout << ("callback1 " + str).c_str() << std::endl; };
	auto callback2 = [](const std::string& str) {std::cout << ("callback2 " + str).c_str() << std::endl; };
	auto callback3 = [](const std::string& str) {std::cout << ("callback3 " + str).c_str() << std::endl; };
	slot slot;
	{
		signal<void(const std::string&)> signal;
		slot = signal.connect(callback1);
		signal.call("");
		signal.call("abc");

		auto slot2 = signal.connect(callback2);
		auto slot3 = signal.connect(callback3);
		signal.call("hello world!");

	}
}


int main()
{
	//test_observer();

	test_signal_slot();


    return 0;
}


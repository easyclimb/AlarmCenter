// TestCpp11NewFeatures.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../AlarmCenter/observer.h"
#include <iostream>

typedef std::shared_ptr<std::string> target;

class observer : public dp::ovserver<target>, public std::enable_shared_from_this<observer>
{
public:
	void on_update(target target) {
		std::cout << target->c_str() << std::endl;
	}
};



class subject :public dp::observable<target>
{

};


int main()
{
	auto obs = std::make_shared<observer>();
	subject sbj;
	std::weak_ptr<observer> w(obs);
	sbj.register_observer(w);
	sbj.notify_observers(std::make_shared<std::string>("hello world!"));
	obs.reset();
	sbj.notify_observers(std::make_shared<std::string>("are u ok?"));
    return 0;
}


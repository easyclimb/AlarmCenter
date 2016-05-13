#include "C:/dev/Global/Log.h"

IMPLEMENT_CLASS_LOG_STATIC_MEMBER;

int main()
{
	{
		auto log = jlib::log::get_instance();
		log->set_line_prifix("CJ");
		log->set_log_file_foler("");
		log->set_output_to_dbg_view(true);
		log->set_output_to_file(true);
		log->set_output_to_console(true);

		AUTO_LOG_FUNCTION;

		{
			jlib::range_log log("test1");
		}

		{
			jlib::range_log log(L"test2");
		}
	}

	using namespace jlib;
	using namespace std::chrono;
	auto now = system_clock::now();
	JLOGA(time_point_to_string(now).c_str());
	{
		auto day = hours(24);
		now += day;
		JLOGA(time_point_to_string(now).c_str());
	}


	//_CrtDumpMemoryLeaks();
	return 0;
}

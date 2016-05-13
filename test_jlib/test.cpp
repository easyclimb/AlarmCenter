#include "C:/dev/Global/Log.h"

IMPLEMENT_CLASS_LOG_STATIC_MAMBER;

int main()
{
	{
		auto log = jlib::log::get_instance();
		log->set_line_prifix("CJ");
		log->set_log_file_foler("");
		log->set_output_to_dbg_view(true);
		log->set_output_to_file(true);

		AUTO_LOG_FUNCTION;

		{
			jlib::range_log log("test1");
		}

		{
			jlib::range_log log(L"test2");
		}
	}


	//_CrtDumpMemoryLeaks();
	return 0;
}

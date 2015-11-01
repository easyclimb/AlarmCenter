// TestBoostInterProcess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "C:/boost_1_59_0/stage/lib/libboost_date_time-vc120-mt-gd-1_59.lib")
#include <boost\\interprocess\\managed_shared_memory.hpp>
#include <cstdlib>
#include <sstream>
#include <iostream>
static const char g_shm_name[] = "MySharedMemory";

int main(int argc, char* argv[])
{
	using namespace boost::interprocess;
	typedef struct my_struct
	{
		double x;
		double y;
		int level;
		std::wstring title;
		int ademco_id;
		int sub_id;
		int type;
	}my_struct;

	if (argc == 1) {
		struct shm_remove
		{
			shm_remove() { shared_memory_object::remove(g_shm_name); }
			~shm_remove() { shared_memory_object::remove(g_shm_name); }
		}remover;

		managed_shared_memory segment(create_only, g_shm_name, 65536);
		managed_shared_memory::size_type free_memory = segment.get_free_memory();
		void* shptr = segment.allocate(1024);
		if (free_memory <= segment.get_free_memory())
			return 1;
		memcpy(shptr, g_shm_name, strlen(g_shm_name));

		managed_shared_memory::handle_t handle = segment.get_handle_from_address(shptr);
		std::stringstream s;
		s << argv[0] << " " << handle;
		s << std::ends;

		if (0 != std::system(s.str().c_str()))
			return 1;
		if (free_memory != segment.get_free_memory())
			return 1;

	} else {
		managed_shared_memory segment(open_only, g_shm_name);
		managed_shared_memory::handle_t handle = 0;
		std::stringstream s; s << argv[1]; s >> handle;
		void* msg = segment.get_address_from_handle(handle);
		std::cout << reinterpret_cast<const char*>(msg) << std::endl;
		segment.deallocate(msg);
		std::system("pause");
	}

}


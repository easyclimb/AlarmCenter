// TestBoostInterProcess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "C:/boost_1_59_0/stage/lib/libboost_date_time-vc120-mt-gd-1_59.lib")
#include <boost\\interprocess\\managed_shared_memory.hpp>
#include <cstdlib>
#include <sstream>
#include <iostream>
static const char g_shm_name[] = "MySharedMemory";

typedef struct shm_remove
{
	shm_remove() { boost::interprocess::shared_memory_object::remove(g_shm_name); }
	~shm_remove() { boost::interprocess::shared_memory_object::remove(g_shm_name); }
}shm_remove;

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

int main_process_using_shared_memory_as_a_pool_of_unnamed_memory_blocks(int argc, char* argv[])
{
	using namespace boost::interprocess;
	shm_remove remover;

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

	return 0;
}

int child_process_using_shared_memory_as_a_pool_of_unnamed_memory_blocks(int argc, char* argv[])
{
	using namespace boost::interprocess;
	managed_shared_memory segment(open_only, g_shm_name);
	managed_shared_memory::handle_t handle = 0;
	std::stringstream s; s << argv[1]; s >> handle;
	void* msg = segment.get_address_from_handle(handle);
	std::cout << reinterpret_cast<const char*>(msg) << std::endl;
	segment.deallocate(msg);
	std::system("pause");
	return 0;
}

int main(int argc, char* argv[])
{
	if (argc == 1) {
		return main_process_using_shared_memory_as_a_pool_of_unnamed_memory_blocks(argc, argv);
	} else {
		return child_process_using_shared_memory_as_a_pool_of_unnamed_memory_blocks(argc, argv);
	}

}


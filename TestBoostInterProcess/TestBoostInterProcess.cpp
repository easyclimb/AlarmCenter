// TestBoostInterProcess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "C:/boost_1_59_0/stage/lib/libboost_date_time-vc140-mt-gd-1_59.lib")
#include <boost\\interprocess\\managed_shared_memory.hpp>
#include <boost\\interprocess\\allocators\\allocator.hpp>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <process.h>
static const char g_shm_name[] = "MySharedMemory";
static const char g_instance_name[] = "my_struct instance";

using namespace boost::interprocess;

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

typedef struct shm_remove
{
	shm_remove() { boost::interprocess::shared_memory_object::remove(g_shm_name); }
	~shm_remove() { boost::interprocess::shared_memory_object::remove(g_shm_name); }
}shm_remove;

typedef struct my_struct
{
	double _x;
	double _y;
	int _level;
	//std::wstring _title;
	wchar_t _title[1024];
	int _ademco_id;
	int _sub_id;
	int _type;
	my_struct() {}
	my_struct(double x, double y, int level, const wchar_t* title, int ademco_id, int sub_id, int type) 
		: _x(x), _y(y), _level(level),/* _title(title), */_ademco_id(ademco_id), _sub_id(sub_id), _type(type) 
	{
		wcscpy(_title, title);
	}
}my_struct;

typedef boost::interprocess::allocator<my_struct, boost::interprocess::managed_shared_memory::segment_manager> ShmemAllocator;


int main_process_using_shared_memory_as_a_pool_of_unnamed_memory_blocks(int argc, char* argv[])
{
	PRINT_FUNCTION
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
	PRINT_FUNCTION
	managed_shared_memory segment(open_only, g_shm_name);
	managed_shared_memory::handle_t handle = 0;
	std::stringstream s; s << argv[1]; s >> handle;
	void* msg = segment.get_address_from_handle(handle);
	std::cout << reinterpret_cast<const char*>(msg) << std::endl;
	segment.deallocate(msg);
	//std::system("pause");
	return 0;
}

int lanch_child_process(char* argv)
{
	PRINT_FUNCTION
	std::string s(argv); s += " child ";
	return std::system(s.c_str());
}

int main_process_creating_named_shared_memory_objects(int argc, char* argv[])
{
	PRINT_FUNCTION
	shm_remove remover;
	managed_shared_memory segment(create_only, g_shm_name, 65536);
	my_struct* instance = segment.construct<my_struct>
		(g_instance_name)
		(0.0, 0.0, 14, L"Creating named shared memory objects", 0, 0, 0);
	if (0 != lanch_child_process(argv[0]))
		return 0;

	if (segment.find<my_struct>(g_instance_name).first)
		return 1;

	return 0;
}

void print_my_struct(const my_struct* ms)
{
	PRINT_FUNCTION
	std::cout << "_x " <<  ms->_x << std::endl
		<< "_y " << ms->_y << std::endl
		<< "_level " << ms->_level << std::endl
		//<< ms->_title.c_str() << std::endl
		//<< "_title " << ms->_title
		<< "_ademco_id " << ms->_ademco_id << std::endl
		<< "_sub_id " << ms->_sub_id << std::endl
		<< "_type " << ms->_type << std::endl
		;
	std::wcout << "_title " << ms->_title << std::endl;
}

int child_process_creating_named_shared_memory_objects(int argc, char* argv[])
{
	PRINT_FUNCTION
	managed_shared_memory segment(open_only, g_shm_name);
	std::pair<my_struct*, managed_shared_memory::size_type> res;
	res = segment.find<my_struct>(g_instance_name);
	//std::cout << res.second << std::endl;
	if (res.second != 1) {
		return 1;
	}
	print_my_struct(res.first);
	segment.destroy<my_struct>(g_instance_name);
	//std::system("pause");
	return 0;
}

//int main_process_creating_complex_objects_in_shared_memory(int argc, char* argv[])
//{
//	shm_remove remover;
//	managed_shared_memory segment(create_only, g_shm_name, 65536);
//	const ShmemAllocator alloc_inst(segment.get_segment_manager());
//	my_struct* instance = segment.construct<my_struct>(g_instance_name)(alloc_inst);
//	instance->_title = L"test";
//	if (0 != lanch_child_process(argv[0]))
//		return 0;
//	if (segment.find<my_struct>(g_instance_name).first)
//		return 1;
//	return 0;
//}
//
//int child_process_creating_complex_objects_in_shared_memory(int argc, char* argv[])
//{
//	managed_shared_memory segment(open_only, g_shm_name);
//	my_struct* instance = segment.find<my_struct>(g_instance_name).first;
//	print_my_struct(instance);
//	segment.destroy<my_struct>(g_instance_name);
//	return 0;
//}

int main(int argc, char* argv[])
{
	PRINT_FUNCTION
	int ret = 0;
	if (argc == 1) {
		//return main_process_using_shared_memory_as_a_pool_of_unnamed_memory_blocks(argc, argv);
		ret = main_process_creating_named_shared_memory_objects(argc, argv);
		//return main_process_creating_complex_objects_in_shared_memory(argc, argv);
	} else {
		//return child_process_using_shared_memory_as_a_pool_of_unnamed_memory_blocks(argc, argv);
		ret = child_process_creating_named_shared_memory_objects(argc, argv);
		//child_process_creating_complex_objects_in_shared_memory(argc, argv);
	}
	//ptrdiff_t t;
	std::cout << "ret=" << ret << std::endl;
	std::system("pause");
	return ret;
}


// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <afx.h>
#include <iostream>
#include <string>
#include "C:/dev/Global/global.h"
#include "../AlarmCenter/video.h"

typedef struct Test
{
	int a;
	int b;
	static const int c = 1;
	static const int d = 2;
	Test() { memset(this, 0, sizeof(Test)); }
}Test;

typedef struct Test2
{
	Test a;
	int b;
	Test2() { memset(this, 0, sizeof(Test2)); }
}Test2;

typedef struct ConnID
{
	char _1;
	char _2;
	char _3;

	ConnID() : _1(-1), _2(-1), _3(-1) { printf("ConnID()\n"); }
	~ConnID() { printf("~ConnID()\n"); }
	ConnID& operator=(int conn_id) { printf("ConnID operator=\n"); FromInt(conn_id); return *this; }

	ConnID(int conn_id)
	{
		FromInt(conn_id);
	}

	void FromInt(int conn_id)
	{
		_1 = LOBYTE(HIWORD(conn_id));
		_2 = HIBYTE(LOWORD(conn_id));
		_3 = LOBYTE(LOWORD(conn_id));
	}

	void FromCharArray(char arr[3])
	{
		_1 = arr[0];
		_2 = arr[1];
		_3 = arr[2];
	}

	int ToInt()
	{
		return MAKELONG(MAKEWORD(_3, _2), MAKEWORD(_1, 0));
	}
}ConnID;

typedef struct PrivateCmd
{
	static const size_t DEFAULT_CAPACITY = 64;
	char* _data;
	size_t _size;
	size_t _capacity;

	PrivateCmd() : _data(NULL), _size(0), _capacity() { printf("PrivateCmd()\n"); }

	~PrivateCmd() { printf("~PrivateCmd()\n"); Clear(); }

	void Clear() { if (_data) { delete[] _data; } memset(this, 0, sizeof(PrivateCmd)); }

	void Expand()
	{
		if (_data == NULL) {
			_size = 0;
			_capacity = DEFAULT_CAPACITY;
			_data = new char[_capacity];
		}

		if (_size < _capacity) return;

		char* old_data = _data;
		_data = new char[_capacity <<= 1];
		memcpy(_data, old_data, _size);
		delete[] old_data;
	}

	void Append(char cmd)
	{
		Expand();
		_data[_size++] = cmd;
	}

	void Append(const char* cmd, size_t cmd_len)
	{
		for (size_t i = 0; i < cmd_len; i++) {
			Append(cmd[i]);
		}
	}

	void AppendConnID(const ConnID& connID)
	{
		Append(connID._1);
		Append(connID._2);
		Append(connID._3);
	}

	void Assign(const char* cmd, size_t cmd_len)
	{
		Clear();
		for (size_t i = 0; i < cmd_len; i++) {
			Append(cmd[i]);
		}
	}

	PrivateCmd& operator=(const PrivateCmd& rhs)
	{
		printf("PrivateCmd operator=\n");
		Clear();
		for (size_t i = 0; i < rhs._size; i++) {
			Append(rhs._data[i]);
		}
		return *this;
	}

	ConnID GetConnID() const
	{
		ConnID id;
		if (_size >= 3) {
			id.FromCharArray(_data);
		}
		return id;
	}
}PrivateCmd;

int _tmain(int argc, _TCHAR* argv[])
{
	// 2015ƒÍ9‘¬7»’ 17:27:18 ≤‚ ‘std::map
	video::ZoneUuid zone(0, 1, 2);
	video::ZoneUuid zone2(1, 0, 0);
	video::DeviceInfo device(NULL, 1);
	video::DeviceInfo device2(NULL, 0);
	video::CBindMap map;
	map[zone] = device;
	for (const auto& i : map) {
		std::cout << i.first.toString() << i.second.toString() << std::endl;
	}
	map[zone2] = device2;
	for (const auto& i : map) {
		std::cout << i.first.toString() << i.second.toString() << std::endl;
	}
	// end

	Test test;
	int n = sizeof(test);
	Test2 test2;
	n = sizeof(test2);

	ConnID id;
	id = 123;
	PrivateCmd cmd;
	cmd.AppendConnID(id);
	ConnID id2 = cmd.GetConnID();


	system("pause");
	return 0;
}


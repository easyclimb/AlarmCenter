#pragma once

#include <string>



//struct MemoryMappedStruct
//{
//	typedef struct _MemoryMappedStruct
//	{
//		bool _updated;
//		virtual size_t get_size() const = 0;
//	}_MemoryMappedStruct;
//
//	_MemoryMappedStruct* _writer;
//	_MemoryMappedStruct* _reader;
//};


template <class T>
class CMemoryMappedFileSharingTool
{
private:
	T _data;
	HANDLE _handle;
	std::wstring _obj_name;
public:
	CMemoryMappedFileSharingTool<T>() : _data(), _handle(INVALID_HANDLE_VALUE) {}
	~CMemoryMappedFileSharingTool<T>() { ForceClose(); }
	void ForceClose() { if (INVALID_HANDLE_VALUE != _handle) { CloseHandle(_handle); _handle = INVALID_HANDLE_VALUE; } _obj_name.clear(); }
	BOOL CreateOrOpen(const wchar_t* obj_name);


};




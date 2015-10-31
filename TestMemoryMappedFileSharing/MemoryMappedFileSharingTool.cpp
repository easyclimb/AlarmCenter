#include "stdafx.h"
#include "MemoryMappedFileSharingTool.h"


template <class T>
BOOL CMemoryMappedFileSharingTool<T>::CreateOrOpen(const wchar_t* obj_name)
{
	ForceClose();
	BOOL ok = TRUE;
	do {
		_obj_name = obj_name;
		_handle = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, _data->size(), obj_name);
		if (_handle == nullptr) { ok = FALSE; break; }
		if (GetLastError() == ERROR_ALREADY_EXISTS) {

		}
	} while (0);

}

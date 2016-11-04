// AutoSerialPort.cpp: implementation of the CAutoSerialPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AutoSerialPort.h"

namespace util
{
	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////

	CAutoSerialPort::CAutoSerialPort()
	{

	}

	CAutoSerialPort::~CAutoSerialPort()
	{

	}

	BOOL CAutoSerialPort::CheckValidSerialPorts(std::list<int>& list)
	{
		BOOL b = FALSE;
		CString port = _T("");
		for (int i = 1; i < 256; i++) {
			port.Format(_T("COM%d"), i);
			if (IsFileExists(port)) {
				port.Format(_T("%d"), i);
				list.push_back(i);
				b = TRUE;
			}
		}
		return b;
	}

	BOOL CAutoSerialPort::IsFileExists(LPCTSTR lpszFile)
	{
		BOOL bExist = FALSE;
		HANDLE file = ::CreateFile(lpszFile,
								   GENERIC_READ,
								   FILE_SHARE_READ,
								   nullptr,
								   OPEN_EXISTING,
								   FILE_ATTRIBUTE_NORMAL,
								   nullptr);

		if (file != INVALID_HANDLE_VALUE) {
			bExist = TRUE;
			::CloseHandle(file);
		} else if (ERROR_ACCESS_DENIED == GetLastError()) {
			//bExist = TRUE;
		}
		return bExist;
	}

};

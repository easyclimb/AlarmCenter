// AutoSerialPort.h: interface for the CAutoSerialPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOSERIALPORT_H__72E604B2_99EA_46C3_A726_775BD678CB2B__INCLUDED_)
#define AFX_AUTOSERIALPORT_H__72E604B2_99EA_46C3_A726_775BD678CB2B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>

namespace util
{
	class CAutoSerialPort
	{
	public:
		BOOL IsFileExists(LPCTSTR lpszFile);
		BOOL CheckValidSerialPorts(std::list<int>& list);
		CAutoSerialPort();
		virtual ~CAutoSerialPort();

	};
};

#endif // !defined(AFX_AUTOSERIALPORT_H__72E604B2_99EA_46C3_A726_775BD678CB2B__INCLUDED_)

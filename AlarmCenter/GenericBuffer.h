// GenericBuffer.h: interface for the CGenericBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GENERICBUFFER_H__45945758_7D50_40DC_9D28_9EACDA0969B2__INCLUDED_)
#define AFX_GENERICBUFFER_H__45945758_7D50_40DC_9D28_9EACDA0969B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace util {

#define DEFAULT_BUFFER_LENGTH ((DWORD)(1024 * 1024 * 1))

	class CGenericBuffer
	{
		typedef struct _BUFFER
		{
			char  *buff;
			DWORD posR;
			DWORD posW;
			DWORD len;
			_BUFFER()
			{
				buff = NULL;
				len = DEFAULT_BUFFER_LENGTH;
				posR = 0;
				posW = 0;
			}
		}BUFFER;
	public:
		DWORD GetValidateLen();
		void Clear();
		DWORD Write(const char * pBuf, DWORD dwLen);
		DWORD Read(char * pBuf, DWORD dwLen);
		CGenericBuffer(DWORD bufflen = DEFAULT_BUFFER_LENGTH);
		virtual ~CGenericBuffer();
	private:
		BUFFER m_buf;
		CRITICAL_SECTION m_csBuf;
	};
};
#endif // !defined(AFX_GENERICBUFFER_H__45945758_7D50_40DC_9D28_9EACDA0969B2__INCLUDED_)

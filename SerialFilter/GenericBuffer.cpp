// GenericBuffer.cpp: implementation of the CGenericBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GenericBuffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGenericBuffer::CGenericBuffer(DWORD bufflen)
{
	ASSERT(bufflen > 0 && bufflen <= DEFAULT_BUFFER_LENGTH);
	m_buf.buff = new char[bufflen];
	ASSERT(m_buf.buff);
	memset(m_buf.buff, 0, bufflen);
	m_buf.len = bufflen;
	m_buf.posR = 0;
	m_buf.posW = 0;
	InitializeCriticalSection(&m_csBuf);
	//CLog::WriteLog("CGenericBuffer construction");
}

CGenericBuffer::~CGenericBuffer()
{
	if (m_buf.buff != NULL) {
		delete[] m_buf.buff;
		m_buf.buff = NULL;
	}
	DeleteCriticalSection(&m_csBuf);
	//CLog::WriteLog("CGenericBuffer destruction");
}

//#define TRACEBUFF
#ifdef TRACEBUFF
static inline void traceR(unsigned long posR, unsigned long posW, unsigned long dwLen)
{
	char dbgbuf[64];
	sprintf_s(dbgbuf, "CGenericBuffer::Read   %4d, posR %d, posW %d\n", 
			  dwLen, posR, posW);
	OutputDebugString(dbgbuf);
}
static inline void traceW(unsigned long posR, unsigned long posW, unsigned long dwLen)
{
	char dbgbuf[64];
	sprintf_s(dbgbuf, "CGenericBuffer::Write  %4d, posR %d, posW %d\n", 
			  dwLen, posR, posW);
	OutputDebugString(dbgbuf);
}
#define TRACER(r,w,l) traceR(r,w,l)
#define TRACEW(r,w,l) traceW(r,w,l)
#else	/* TRACEBUFF */
#define TRACER(r,w,l)
#define TRACEW(r,w,l)
#endif	/* TRACEBUFF */

DWORD CGenericBuffer::Read(char *pBuf, DWORD dwLen)
{
	CLocalLock lock(&m_csBuf);
	DWORD dwReadLen = 0;
	DWORD dwValidateLen = 0;
	if (dwLen <= m_buf.len) {
		if (m_buf.posW > m_buf.posR) {
			// ��ָ�����дָ�룬��������δд��һ��
			// ���㻺��������Ч���ݵĳ���
			dwValidateLen = m_buf.posW - m_buf.posR;
			if (dwValidateLen >= dwLen) {
				//��Ч�����㹻����ȡָ����������
				TRACER(m_buf.posR, m_buf.posW, dwLen);
				memcpy(pBuf, m_buf.buff + m_buf.posR, dwLen);
				m_buf.posR += dwLen;
				dwReadLen = dwLen;
			} else {
				// ��Ч���ݳ��Ȳ���ָ�����ȣ���ȡ��Ч����
				memcpy(pBuf, m_buf.buff + m_buf.posR, dwValidateLen);
				m_buf.posR += dwValidateLen;
				dwReadLen = dwValidateLen;
			}
		} else if (m_buf.posW < m_buf.posR) {
			// ��ָ���Ѿ�����дָ�룬˵���������Ѿ�д��һ�֣��������˻�����ͷ����һ��������
			dwValidateLen = m_buf.len - (m_buf.posR - m_buf.posW);
			if (dwValidateLen >= dwLen) {
				//��Ч�����㹻����ȡָ����������
				if (m_buf.posR + dwLen <= m_buf.len) {
					TRACER(m_buf.posR, m_buf.posW, dwLen);
					memcpy(pBuf, m_buf.buff + m_buf.posR, dwLen);
					m_buf.posR += dwLen;
				} else {
					TRACER(m_buf.posR, m_buf.posW, dwLen);
					int extLen = m_buf.posR + dwLen - m_buf.len;
					memcpy(pBuf, m_buf.buff + m_buf.posR, dwLen - extLen);
					memcpy(pBuf + dwLen - extLen, m_buf.buff, extLen);
					m_buf.posR = extLen;
				}
				dwReadLen = dwLen;
			} else {
				// ��Ч���ݳ��Ȳ���ָ�����ȣ���ȡ��Ч����
				if (m_buf.posR + dwValidateLen <= m_buf.len) {
					TRACER(m_buf.posR, m_buf.posW, dwValidateLen);
					memcpy(pBuf, m_buf.buff + m_buf.posR, dwValidateLen);
					m_buf.posR += dwValidateLen;
				} else {
					TRACER(m_buf.posR, m_buf.posW, dwLen);
					int extLen = m_buf.posR + dwValidateLen - m_buf.len;
					memcpy(pBuf, m_buf.buff + m_buf.posR, dwValidateLen - extLen);
					memcpy(pBuf + dwValidateLen - extLen, m_buf.buff, extLen);
					m_buf.posR = extLen;
				}
				dwReadLen = dwValidateLen;
			}
		}
	}
	return dwReadLen;
}

DWORD CGenericBuffer::Write(const char *pBuf, DWORD dwLen)
{
	CLocalLock lock(&m_csBuf);
	if (dwLen <= m_buf.len) {
		if (m_buf.posW + dwLen <= m_buf.len) {
			TRACEW(m_buf.posR, m_buf.posW, dwLen);
			memcpy((m_buf.buff + m_buf.posW), pBuf, dwLen);
			m_buf.posW += dwLen;
		} else if (m_buf.posW + dwLen > m_buf.len) {
			TRACEW(m_buf.posR, m_buf.posW, dwLen);
			int extLen = m_buf.posW + dwLen - m_buf.len;
			memcpy((m_buf.buff + m_buf.posW), pBuf, dwLen - extLen);
			memcpy(m_buf.buff, pBuf + dwLen - extLen, extLen);
			m_buf.posW = extLen;
		}
		return dwLen;
	}
	return 0;
}

void CGenericBuffer::Clear()
{
	CLocalLock lock(&m_csBuf);
	m_buf.posR = 0;
	m_buf.posW = 0;
}

DWORD CGenericBuffer::GetValidateLen()
{
	CLocalLock lock(&m_csBuf);
	DWORD dwValidateLen = 0;
	if (m_buf.posW >= m_buf.posR) {
		dwValidateLen = m_buf.posW - m_buf.posR;
	} else if (m_buf.posW < m_buf.posR) {
		dwValidateLen = m_buf.len - (m_buf.posR - m_buf.posW);
	}

	return dwValidateLen;
}

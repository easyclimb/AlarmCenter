// GenericBuffer.cpp: implementation of the CGenericBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GenericBuffer.h"

namespace util
{
	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////
	CGenericBuffer::CGenericBuffer(DWORD bufflen)
	{
		assert(bufflen > 0 && bufflen <= DEFAULT_BUFFER_LENGTH);
		m_buf.buff.reserve(bufflen);
		m_buf.buff.assign(bufflen, 0);
		m_buf.len = bufflen;
		m_buf.posR = 0;
		m_buf.posW = 0;
		//JLOG("CGenericBuffer construction");
	}

	CGenericBuffer::~CGenericBuffer()
	{
		//JLOG("CGenericBuffer destruction");
	}

	//#define TRACEBUFF
#ifdef TRACEBUFF
	inline void traceR(unsigned long posR, unsigned long posW, unsigned long dwLen)
	{
		char dbgbuf[64];
		sprintf_s(dbgbuf, "CGenericBuffer::Read   %4d, posR %d, posW %d\n", 
				  dwLen, posR, posW);
		OutputDebugString(dbgbuf);
	}
	inline void traceW(unsigned long posR, unsigned long posW, unsigned long dwLen)
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
		std::lock_guard<std::mutex> lock(m_csBuf);
		DWORD dwReadLen = 0;
		DWORD dwValidateLen = 0;
		if (dwLen <= m_buf.len) {
			if (m_buf.posW > m_buf.posR) {
				// 读指针大于写指针，缓冲区还未写满一轮
				// 计算缓冲区中有效数据的长度
				dwValidateLen = m_buf.posW - m_buf.posR;
				if (dwValidateLen >= dwLen) {
					//有效长度足够，读取指定长度数据
					TRACER(m_buf.posR, m_buf.posW, dwLen);
					memcpy(pBuf, &m_buf.buff[m_buf.posR], dwLen);
					m_buf.posR += dwLen;
					dwReadLen = dwLen;
				} else {
					// 有效数据长度不够指定长度，读取有效数据
					memcpy(pBuf, &m_buf.buff[m_buf.posR], dwValidateLen);
					m_buf.posR += dwValidateLen;
					dwReadLen = dwValidateLen;
				}
			} else if (m_buf.posW < m_buf.posR) {
				// 读指针已经大于写指针，说明缓冲区已经写满一轮，并覆盖了缓冲区头部的一部分数据
				dwValidateLen = m_buf.len - (m_buf.posR - m_buf.posW);
				if (dwValidateLen >= dwLen) {
					//有效长度足够，读取指定长度数据
					if (m_buf.posR + dwLen <= m_buf.len) {
						TRACER(m_buf.posR, m_buf.posW, dwLen);
						memcpy(pBuf, &m_buf.buff[m_buf.posR], dwLen);
						m_buf.posR += dwLen;
					} else {
						TRACER(m_buf.posR, m_buf.posW, dwLen);
						int extLen = m_buf.posR + dwLen - m_buf.len;
						memcpy(pBuf, &m_buf.buff[m_buf.posR], dwLen - extLen);
						memcpy(pBuf + dwLen - extLen, &m_buf.buff[0], extLen);
						m_buf.posR = extLen;
					}
					dwReadLen = dwLen;
				} else {
					// 有效数据长度不够指定长度，读取有效数据
					if (m_buf.posR + dwValidateLen <= m_buf.len) {
						TRACER(m_buf.posR, m_buf.posW, dwValidateLen);
						memcpy(pBuf, &m_buf.buff[m_buf.posR], dwValidateLen);
						m_buf.posR += dwValidateLen;
					} else {
						TRACER(m_buf.posR, m_buf.posW, dwLen);
						int extLen = m_buf.posR + dwValidateLen - m_buf.len;
						memcpy(pBuf, &m_buf.buff[m_buf.posR], dwValidateLen - extLen);
						memcpy(pBuf + dwValidateLen - extLen, &m_buf.buff[0], extLen);
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
		std::lock_guard<std::mutex> lock(m_csBuf);
		if (dwLen <= m_buf.len) {
			if (m_buf.posW + dwLen <= m_buf.len) {
				TRACEW(m_buf.posR, m_buf.posW, dwLen);
				memcpy(&m_buf.buff[m_buf.posW], pBuf, dwLen);
				m_buf.posW += dwLen;
			} else if (m_buf.posW + dwLen > m_buf.len) {
				TRACEW(m_buf.posR, m_buf.posW, dwLen);
				int extLen = m_buf.posW + dwLen - m_buf.len;
				memcpy(&m_buf.buff[m_buf.posW], pBuf, dwLen - extLen);
				memcpy(&m_buf.buff[0], pBuf + dwLen - extLen, extLen);
				m_buf.posW = extLen;
			}
			return dwLen;
		}
		return 0;
	}

	void CGenericBuffer::Clear()
	{
		std::lock_guard<std::mutex> lock(m_csBuf);
		m_buf.posR = 0;
		m_buf.posW = 0;
	}

	DWORD CGenericBuffer::GetValidateLen()
	{
		std::lock_guard<std::mutex> lock(m_csBuf);
		DWORD dwValidateLen = 0;
		if (m_buf.posW >= m_buf.posR) {
			dwValidateLen = m_buf.posW - m_buf.posR;
		} else if (m_buf.posW < m_buf.posR) {
			dwValidateLen = m_buf.len - (m_buf.posR - m_buf.posW);
		}

		return dwValidateLen;
	}

};

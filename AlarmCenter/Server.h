// Server.h: interface for the CServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVER_H__1AD9A3F7_E110_465E_8023_586BD5B8CA79__INCLUDED_)
#define AFX_SERVER_H__1AD9A3F7_E110_465E_8023_586BD5B8CA79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace net {
namespace server {

class CServer
{
public:
	BOOL IsConnectionEstablished() const { return m_bServerStarted; }
	BOOL SendToClient(int ademco_id, int ademco_event, const char* psw = NULL);
	void Stop();
	BOOL Start(WORD port);
	virtual ~CServer(){}
	static CServer *GetInstance(){
		m_Lock4GetInstance.Lock();
		if (m_pInst == NULL) {
			static CServer server;
			m_pInst = &server;
		}
			//m_pInst = new CServer();
		m_Lock4GetInstance.UnLock();
		return m_pInst;
	}
	//static void Release();
private:
	CServer() : m_bServerStarted(false)
	{
	//m_pDataPacketMgr = NULL;
	}
	static CServer *m_pInst;
	static CLock m_Lock4GetInstance;
	bool m_bServerStarted;
protected:
};


NAMESPACE_END
NAMESPACE_END

#endif // !defined(AFX_SERVER_H__1AD9A3F7_E110_465E_8023_586BD5B8CA79__INCLUDED_)

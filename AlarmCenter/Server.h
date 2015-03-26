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
	BOOL SendToClient(int ademco_id, int ademco_event, int gg,
					  int zone, const char* psw);
	void Stop();
	BOOL Start(WORD port);
	~CServer(){}
	//static void Release();
private:
	DECLARE_SINGLETON(CServer)
	bool m_bServerStarted;
protected:
};


NAMESPACE_END
NAMESPACE_END

#endif // !defined(AFX_SERVER_H__1AD9A3F7_E110_465E_8023_586BD5B8CA79__INCLUDED_)

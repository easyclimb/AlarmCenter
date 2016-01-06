// Server.h: interface for the CServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVER_H__1AD9A3F7_E110_465E_8023_586BD5B8CA79__INCLUDED_)
#define AFX_SERVER_H__1AD9A3F7_E110_465E_8023_586BD5B8CA79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <boost/noncopyable.hpp>

namespace net {
namespace server {

class CServer : public boost::noncopyable
{
public:
	BOOL IsConnectionEstablished() const { return m_bServerStarted; }
	BOOL SendToClient(int ademco_id, int ademco_event, int gg,
					  int zone, const ademco::char_array_ptr& = nullptr);
	void Stop();
	BOOL Start(unsigned int& port);
	CServer();
	~CServer(){}
private:
	bool m_bServerStarted;
protected:
};


NAMESPACE_END
NAMESPACE_END

#endif // !defined(AFX_SERVER_H__1AD9A3F7_E110_465E_8023_586BD5B8CA79__INCLUDED_)

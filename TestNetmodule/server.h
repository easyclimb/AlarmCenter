#pragma once

namespace net {
namespace server {

typedef void(__stdcall *OnAdemcoEvent)(void* udata, int ademco_event);

class CServer
{
public:
	OnAdemcoEvent m_cb;
	void* m_udata;
	BOOL IsConnectionEstablished() const { return m_bServerStarted; }
	BOOL SendToClient(int ademco_id, int ademco_event, int gg,
					  int zone, const char* xdata, int xdata_len);
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


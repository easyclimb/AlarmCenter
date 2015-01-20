#include "stdafx.h"
#include "NetworkConnector.h"
#include "Client.h"
#include "Server.h"

namespace net
{

CNetworkConnector* CNetworkConnector::m_pInstance = NULL;
CLock CNetworkConnector::m_lock4Instance;

CNetworkConnector::CNetworkConnector()
{}


CNetworkConnector::~CNetworkConnector()
{}


BOOL CNetworkConnector::StartNetwork(WORD listeningPort, 
									 const char* tranmit_server_ip, 
									 WORD transmit_server_port)
{
	int	nRet;
	WSAData	wsData;

	nRet = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (nRet < 0) {
		CLog::WriteLog(L"Can't load winsock.dll.\n");
		return FALSE;
	}

	do {
		if (!CServer::GetInstance()->Start(listeningPort))
			break;

		if (tranmit_server_ip && !CClient::GetInstance()->Start(tranmit_server_ip, 
			transmit_server_port))
			break;

		return TRUE;
	} while (0);

	return FALSE;
}


void CNetworkConnector::StopNetWork()
{
	CServer::GetInstance()->Release();
	CServer::GetInstance()->Stop();
	CClient::GetInstance()->Stop();
	WSACleanup();
}


BOOL CNetworkConnector::Send(int ademco_id, int ademco_event, const char* psw)
{
	CServer* server = CServer::GetInstance();
	CClient* client = CClient::GetInstance();

	if (server->IsConnectionEstablished()) {
		return server->SendToClient(ademco_id, ademco_event, psw);
	} else if (client->IsConnectionEstablished()) {
		return client->SendToTransmitServer(ademco_id, ademco_event, psw);
	}

	return FALSE;
}




NAMESPACE_END

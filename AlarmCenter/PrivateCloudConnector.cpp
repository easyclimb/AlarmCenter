#include "stdafx.h"
#include "AlarmCenter.h"
#include "PrivateCloudConnector.h"
#include "json/json.h"
#include "SdkMgrEzviz.h"
#include "InputDlg.h"

namespace video {
namespace ezviz {

IMPLEMENT_SINGLETON(CPrivateCloudConnector)
CPrivateCloudConnector::CPrivateCloudConnector()
	: _ip()
	, _port(0)
	, _appKey()
{
	AUTO_LOG_FUNCTION;
}


CPrivateCloudConnector::~CPrivateCloudConnector()
{
	AUTO_LOG_FUNCTION;
}


bool CPrivateCloudConnector::get_accToken(std::string& accToken,
										  const std::string& phone,
										  const std::string& user_id)
{
	AUTO_LOG_FUNCTION;
	enum _MsgType
	{
		TYPE_BIND = 1,
		TYPE_VERIFY = 2,
	};
	// \"type\":\"%d\",
	int msg_id = 0;
	char buff[1024] = { 0 }, buff2[1024] = { 0 };
	const char* fmt1 = "{\"id\":\"%d\",\"method\":\"%s\",\"system\":{\"key\":\"%s\",\"time\":\"%d\",\"ver\":\"1.0\"}";// , \"params\":{\"type\":\"%d\",\"userId\":\"%s\",\"phone\":\"%s\"}}";
	const char* fmt2 = ",\"params\":{\"userId\":\"%s\",\"phone\":\"%s\"}}";
	sprintf_s(buff, fmt1, msg_id, "getAccToken", _appKey.c_str(), time(NULL));// , TYPE_VERIFY, user_id, phone);
	sprintf_s(buff2, fmt2, user_id.c_str(), phone.c_str());
	strcat_s(buff, buff2);
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	bool ok = false;

	do {
		if (s == INVALID_SOCKET) {
			LOG(L"create socket failed");
			break;
		}

		sockaddr_in addr = { 0 };
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = inet_addr(_ip.c_str());
		addr.sin_port = htons(_port & 0xFFFF);

		// set the socket in non-blocking mode.
		unsigned long non_blocking_mode = 1;
		int result = ioctlsocket(s, FIONBIO, &non_blocking_mode);
		if (result != NO_ERROR) {
			CLog::WriteLog(_T("ioctlsocket failed : %d\n"), result);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(s);
			break;
		}

		int ret = connect(s, (struct sockaddr *) &addr,
						  sizeof(struct sockaddr));

		if (ret != -1) {
			CLog::WriteLogA("connect to %s:%d failed\n", _ip.c_str(), _port);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(s);
			break;
		}

		TIMEVAL tm = { 3, 0 };
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(s, &fdset);
		if (select(s + 1, NULL, &fdset, NULL, &tm) <= 0) {
			CLog::WriteLogA("connect to %s:%d failed\n", _ip.c_str(), _port);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(s);
			break;
		}

		int error, len;
		len = sizeof(int);
		getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
		if (error != NO_ERROR) {
			CLog::WriteLogA("connect to %s:%d failed\n", _ip.c_str(), _port);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(s);
			break;
		}

		// set the socket in blocking mode.
		non_blocking_mode = 0;
		result = ioctlsocket(s, FIONBIO, &non_blocking_mode);
		if (result != NO_ERROR) {
			CLog::WriteLog(_T("ioctlsocket failed : %d\n"), result);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(s);
			break;
		}

		ret = send(s, buff, strlen(buff), 0);
		if (ret <= 0 || size_t(ret) != strlen(buff)) {
			LOG(L"send to private cloud failed");
			CLOSESOCKET(s);
			break;
		}

		ret = recv(s, buff, 1024, 0);
		if (ret <= 0) {
			LOG(L"recv from private cloud failed");
			CLOSESOCKET(s);
			break;
		}

		buff[ret] = 0;
		Json::Reader reader;
		Json::Value value;
		if (reader.parse(buff, value)) {
			Json::Value code = value["result"]["code"];
			if (!code.empty()) {
				if (code.asString() == "10011") { // 
					Json::Value sign = value["szsign"];
					if (!sign.empty()) {
						std::string szsign = sign.asString();
						ezviz::CSdkMgrEzviz *mgr = ezviz::CSdkMgrEzviz::GetInstance();
						ret = mgr->m_dll.GetAccessTokenSmsCode(szsign);
						CInputDlg dlg;
						if (IDOK != dlg.DoModal())
							break;
						USES_CONVERSION;
						std::string verify_code = W2A(dlg.m_edit);
						ret = mgr->m_dll.VerifyAccessTokenSmsCode(verify_code, user_id.c_str(),
																  phone.c_str(), _appKey.c_str());
						ok = get_accToken(accToken, phone, user_id);
					}
				} else if (code.asString() == "200") {
					accToken = value["result"]["data"]["accessToken"].asString();
					ok = true;
					break;
				}
			}
		}

	} while (0);

	CLOSESOCKET(s);

	return ok;
}






NAMESPACE_END
NAMESPACE_END

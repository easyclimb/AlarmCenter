#include "stdafx.h"
#include "AlarmCenter.h"
#include "PrivateCloudConnector.h"
#include "json/json.h"
#include "SdkMgrEzviz.h"
#include "InputDlg.h"

namespace video {
namespace ezviz {

#define SECUREVALIDATE_REQ "{\"method\":\"msg/sdk/secureValidate\",\"params\":{\"smsCode\": \"%s\",\"accessToken\": \"%s\"}}"


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
										  const std::string& user_id,
										  MsgType type)
{
	AUTO_LOG_FUNCTION;
	
	// \"type\":\"%d\",
	int msg_id = 0;
	char buff[1024] = { 0 }, buff2[1024] = { 0 };
	const char* fmt1 = "{\"id\":\"%d\",\"method\":\"%s\",\"system\":{\"key\":\"%s\",\"time\":\"%d\",\"ver\":\"1.0\"}";// , \"params\":{\"type\":\"%d\",\"userId\":\"%s\",\"phone\":\"%s\"}}";
	const char* fmt2 = ",\"params\":{\"type\":\"%d\",\"userId\":\"%s\",\"phone\":\"%s\"}}";
	if (type == TYPE_GET)
		sprintf_s(buff, fmt1, msg_id, "getAccToken", _appKey.c_str(), time(nullptr));// , TYPE_VERIFY, user_id, phone);
	else if (type == TYPE_HD)
		sprintf_s(buff, fmt1, msg_id, "getSmsSign", _appKey.c_str(), time(nullptr));
	sprintf_s(buff2, fmt2, type, user_id.c_str(), phone.c_str());
	strcat_s(buff, buff2);
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	bool ok = false;

	do {
		if (s == INVALID_SOCKET) {
			JLOG(L"create socket failed");
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

		TIMEVAL tm = { 10, 0 };
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(s, &fdset);
		if (select(s + 1, nullptr, &fdset, nullptr, &tm) <= 0) {
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
			JLOG(L"send to private cloud failed");
			CLOSESOCKET(s);
			break;
		}

		FD_ZERO(&fdset);
		FD_SET(s, &fdset);
		if (select(s + 1, &fdset, nullptr, nullptr, &tm) <= 0) {
			CLog::WriteLogA("recv from %s:%d failed\n", _ip.c_str(), _port);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(s);
			break;
		}
		ret = recv(s, buff, 1024, 0);
		if (ret <= 0) {
			JLOG(L"recv from private cloud failed");
			CLOSESOCKET(s);
			break;
		}

		buff[ret] = 0;
		Json::Reader reader;
		Json::Value value;
		if (reader.parse(buff, value)) {
			if (type == TYPE_GET) {
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
							ok = get_accToken(accToken, phone, user_id, TYPE_GET);
						}
					} else if (code.asString() == "200") {
						accToken = value["result"]["data"]["accessToken"].asString();
						ok = true;
						break;
					}
				}
			} else {
				//Json::Value sign = value.asString();
				//Json::Value sign = value["system"]["sign"];
				//std::string szsign = sign.asString();
				std::string szsign = buff;
				ezviz::CSdkMgrEzviz *mgr = ezviz::CSdkMgrEzviz::GetInstance();
				ret = mgr->m_dll.GetAccessTokenSmsCode(szsign);
				if (ret != 0) break;
				CInputDlg dlg;
				if (IDOK != dlg.DoModal())
					break;
				USES_CONVERSION;
				std::string verify_code = W2A(dlg.m_edit);
				ret = mgr->m_dll.VerifyAccessTokenSmsCode(verify_code, value["params"]["userId"].asString(),
													 phone.c_str(), _appKey.c_str());
				/*char reqStr[1024] = { 0 };
				sprintf_s(reqStr, SECUREVALIDATE_REQ, verify_code.c_str(), accToken.c_str());
				char* pOutStr = NULL;
				int iLen = 0;
				ret = mgr->m_dll.RequestPassThrough(reqStr, &pOutStr, &iLen);*/
				if (ret != 0) break;
				ok = get_accToken(accToken, phone, user_id, TYPE_GET);
				/*pOutStr[iLen] = 0;
				if (reader.parse(pOutStr, value)) {
					Json::Value result = value["result"];
					int iResult = 0;
					if (result["code"].isString()) {
						iResult = atoi(result["code"].asString().c_str());
					} else if (result["code"].isInt()) {
						iResult = result["code"].asInt();
					}
					if (200 == iResult) {
						ok = get_accToken(accToken, phone, user_id, TYPE_GET);
					} else {
						ok = false;
					}
				}*/
				
				break;
			}
		}

	} while (0);

	CLOSESOCKET(s);

	return ok;
}






NAMESPACE_END
NAMESPACE_END

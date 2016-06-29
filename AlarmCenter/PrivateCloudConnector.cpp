#include "stdafx.h"
#include "AlarmCenter.h"
#include "PrivateCloudConnector.h"
#include "json/json.h"
#include "SdkMgrEzviz.h"
#include "InputDlg.h"

namespace video {
namespace ezviz {

#define SECUREVALIDATE_REQ "{\"method\":\"msg/sdk/secureValidate\",\"params\":{\"smsCode\": \"%s\",\"accessToken\": \"%s\"}}"


//IMPLEMENT_SINGLETON(private_cloud_connector)
private_cloud_connector::private_cloud_connector()
{
	AUTO_LOG_FUNCTION;
}


private_cloud_connector::~private_cloud_connector()
{
	AUTO_LOG_FUNCTION;
}


bool private_cloud_connector::get_accToken(const std::string& ip, unsigned int port,
										  const std::string& appKey, 
										  std::string& accToken,
										  const std::string& phone,
										  const std::string& user_id,
										  msg_type type)
{
	AUTO_LOG_FUNCTION;
	
	// \"type\":\"%d\",
	int msg_id = 0;
	char buff[1024] = { 0 }, buff2[1024] = { 0 };
	const char* fmt1 = "{\"id\":\"%d\",\"method\":\"%s\",\"system\":{\"key\":\"%s\",\"time\":\"%d\",\"ver\":\"1.0\"}";// , \"params\":{\"type\":\"%d\",\"userId\":\"%s\",\"phone\":\"%s\"}}";
	const char* fmt2 = ",\"params\":{\"type\":\"%d\",\"userId\":\"%s\",\"phone\":\"%s\"}}";
	if (type == TYPE_GET)
		sprintf_s(buff, fmt1, msg_id, "getAccToken", appKey.c_str(), time(nullptr));// , TYPE_VERIFY, user_id, phone);
	else if (type == TYPE_HD)
		sprintf_s(buff, fmt1, msg_id, "getSmsSign", appKey.c_str(), time(nullptr));
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
		addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
		addr.sin_port = htons(port & 0xFFFF);

		// set the socket in non-blocking mode.
		unsigned long non_blocking_mode = 1;
		int result = ioctlsocket(s, FIONBIO, &non_blocking_mode);
		if (result != NO_ERROR) {
			JLOG(_T("ioctlsocket failed : %d\n"), result);
			JLOG(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(s);
			break;
		}

		int ret = connect(s, (struct sockaddr *) &addr,
						  sizeof(struct sockaddr));

		if (ret != -1) {
			JLOGA("connect to %s:%d failed\n", ip.c_str(), port);
			JLOG(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(s);
			break;
		}

		TIMEVAL tm = { 20, 0 };
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(s, &fdset);
		if (select(s + 1, nullptr, &fdset, nullptr, &tm) <= 0) {
			JLOGA("connect to %s:%d failed\n", ip.c_str(), port);
			JLOG(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(s);
			break;
		}

		int error, len;
		len = sizeof(int);
		getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
		if (error != NO_ERROR) {
			JLOGA("connect to %s:%d failed\n", ip.c_str(), port);
			JLOG(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(s);
			break;
		}

		// set the socket in blocking mode.
		non_blocking_mode = 0;
		result = ioctlsocket(s, FIONBIO, &non_blocking_mode);
		if (result != NO_ERROR) {
			JLOG(_T("ioctlsocket failed : %d\n"), result);
			JLOG(FormatWSAError(WSAGetLastError()));
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
			JLOGA("recv from %s:%d failed\n", ip.c_str(), port);
			JLOG(FormatWSAError(WSAGetLastError()));
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
				JLOG(L"TYPE_GET");
				Json::Value code = value["result"]["code"];
				if (!code.empty()) {
					JLOGA("code=%s", code.asCString());
					if (code.asString() == "10011") { // 
						Json::Value sign = value["szsign"];
						JLOGA("szsign=%s", sign.asCString());
						if (!sign.empty()) {
							std::string szsign = sign.asString();
							auto mgr = ezviz::sdk_mgr_ezviz::get_instance();
							ret = mgr->m_dll.GetAccessTokenSmsCode(szsign);
							if (ret != 0) { JLOG(L"GetAccessTokenSmsCode failed, ret=%d", ret); break; }
							CInputPasswdDlg dlg;
							dlg.m_title = GetStringFromAppResource(IDS_STRING_INPUT_PHONE_VERIFY_CODE);
							if (IDOK != dlg.DoModal()) { JLOG(L"User didnot input sms code."); break; }
							std::string verify_code = utf8::w2a((LPCTSTR)dlg.m_edit);
							ret = mgr->m_dll.VerifyAccessTokenSmsCode(verify_code, user_id.c_str(),
																	  phone.c_str(), appKey.c_str());
							if (ret != 0) {
								JLOG(L"VerifyAccessTokenSmsCode failed, ret=%d, %s",
									 ret, dlg.m_edit);
								auto get_error_msg = [](int code) {
									switch (code)
									{
									case 1011:
										return L"sms verify code error!";
										break;
									default:									
										break;
									}
								};
								break;
							}
							ok = get_accToken(ip, port , appKey, accToken, phone, user_id, TYPE_GET);
						}
					} else if (code.asString() == "200") {
						accToken = value["result"]["data"]["accessToken"].asString();
						JLOGA("accToken=%s", accToken.c_str());
						ok = true;
						break;
					}
				} else {
					JLOGA(R"(value["result"]["code"] is not valid.)");
				}
			} else {
				JLOG(L"TYPE_HD");
				JLOGA("szsign=%s", buff);
				std::string szsign = buff;
				auto mgr = ezviz::sdk_mgr_ezviz::get_instance();
				ret = mgr->m_dll.GetHdSignSmsCode(accToken, szsign);
				if (ret != 0) { JLOG(L"GetHdSignSmsCode failed, ret=%d", ret); break; }
				JLOG(L"GetHdSignSmsCode ok");
				CInputPasswdDlg dlg;
				dlg.m_title = GetStringFromAppResource(IDS_STRING_INPUT_PHONE_VERIFY_CODE);
				if (IDOK != dlg.DoModal()) { JLOG(L"User didnot input sms code."); break; }
				std::string verify_code = utf8::w2a((LPCWSTR)dlg.m_edit);
				JLOGA("verify_code=%s, userId=%s", verify_code.c_str(), phone.c_str());
				ret = mgr->m_dll.VerifyHdSignSmsCode(accToken, verify_code, /*value["params"]["userId"].asString(),*/
													 phone.c_str(), appKey.c_str());
				if (ret != 0) { JLOG(L"VerifyHdSignSmsCode failed, ret=%d, verify_code=%s", ret, utf8::a2w(verify_code).c_str()); break; }
				JLOG(L"VerifyHdSignSmsCode ok");
				ok = get_accToken(ip, port ,appKey, accToken, phone, user_id, TYPE_GET);				
				break;
			}
		}

	} while (0);

	CLOSESOCKET(s);

	JLOG(L"ok=%d", ok);
	return ok;
}






};};

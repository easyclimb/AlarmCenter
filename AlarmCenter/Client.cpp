﻿#include "stdafx.h"
#include "Client.h"
#include "ademco_func.h"
using namespace ademco;
#include "HistoryRecord.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "CsrInfo.h"
#include <memory>
#include <mutex>
#include <ctime>
#include "AlarmCenter.h"
#include "ConfigHelper.h"
#include "AlarmCenter.h"
#include "congwin_fe100_mgr.h"

namespace net {
namespace client {
#ifdef _DEBUG
	static const int LINK_TEST_GAP = 5000;
	static const int SUPPRESS_DISCONN_TIME = 30 * 1000; // 30 seconds
	static const int CHECK_RECVD_DATA_GAP = 15 * 1000; // 15 seconds
	static const int RECONNECT_SERVER_GAP = 15 * 1000; // 15 seconds
	//static const int CONNECT_SERVER_GAP = 3 * 1000; // 3 seconds 
#else
	static const int LINK_TEST_GAP = 5000;
	static const int SUPPRESS_DISCONN_TIME = 60 * 1000; // 1 minute
	static const int CHECK_RECVD_DATA_GAP = 15 * 1000; // 15 seconds
	static const int RECONNECT_SERVER_GAP = 15 * 1000; // 15 seconds
	//static const int CONNECT_SERVER_GAP = 3 * 1000; // 3 seconds 
#endif


CClient::CClient(bool main_client)
	: m_bClientServiceStarted(FALSE)
	, main_client_(main_client)
{}

CClientService::CClientService(bool main_client)
	: m_buff()
	, m_socket(INVALID_SOCKET)
	, connection_established_(false)
	, m_server_addr()
	, m_handler(nullptr)
	, m_server_ip()
	, m_server_port(0)
	, m_bShuttingDown(FALSE)
	, main_client_(main_client)
{
	AUTO_LOG_FUNCTION;

}


CClientService::~CClientService()
{
	AUTO_LOG_FUNCTION;
}


void CClientService::SetEventHandler(std::shared_ptr<CClientEventHandler> handler)
{
	AUTO_LOG_FUNCTION;
	if (handler) {
		m_handler = handler;
	}
}


BOOL CClientService::Start(const std::string& server_ip, unsigned int server_port)
{
	AUTO_LOG_FUNCTION;
	m_server_ip = server_ip;
	m_server_port = server_port;
	m_bShuttingDown = FALSE;
	
	if (!running_) {
		running_ = true;
		thread_ = std::thread(&CClientService::ThreadWorker, this);
	}

	return TRUE;
}

BOOL CClientService::Connect()
{
	AUTO_LOG_FUNCTION;
	if (connection_established_)
		return TRUE;
	do {
		memset(&m_server_addr, 0, sizeof(m_server_addr));
		m_server_addr.sin_family = AF_INET;
		m_server_addr.sin_addr.S_un.S_addr = inet_addr(m_server_ip.c_str());
		m_server_addr.sin_port = htons(static_cast<u_short>(m_server_port));

		if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			JLOG(_T("socket failed\n"));
			JLOG(FormatWSAError(WSAGetLastError()));
			break;
		}

		// set the socket in non-blocking mode.
		unsigned long non_blocking_mode = 1;
		int result = ioctlsocket(m_socket, FIONBIO, &non_blocking_mode);
		if (result != NO_ERROR) {
			JLOG(_T("ioctlsocket failed : %d\n"), result);
			JLOG(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		int ret = connect(m_socket, (struct sockaddr *) &m_server_addr, 
						  sizeof(struct sockaddr));
		
		if (ret != -1) {
			JLOGA("connect to %s:%d failed\n", m_server_ip.c_str(), m_server_port);
			JLOG(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		TIMEVAL tm;
		tm.tv_sec = 3;
		tm.tv_usec = 0;
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(m_socket, &fdset);
		if (select(m_socket + 1, nullptr, &fdset, nullptr, &tm) <= 0) {
			CLOSESOCKET(m_socket);
			break;
		}

		int error, len;
		len = sizeof(int);
		getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
		if (error != NO_ERROR) {
			JLOGA("connect to %s:%d failed\n", m_server_ip.c_str(), m_server_port);
			JLOG(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		// set the socket in blocking mode.
		non_blocking_mode = 0;
		result = ioctlsocket(m_socket, FIONBIO, &non_blocking_mode);
		if (result != NO_ERROR) {
			JLOG(_T("ioctlsocket failed : %d\n"), result);
			JLOG(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		connection_established_ = true;
		m_buff.Clear();
		last_recv_time_ = std::chrono::steady_clock::now();
		disconnected_time_ = std::chrono::steady_clock::now();

		return TRUE;
	} while (0);

	//Restart();

	return FALSE;
}

void CClientService::Stop()
{
	AUTO_LOG_FUNCTION;
	m_bShuttingDown = TRUE;

	if (running_) {
		running_ = false;
		thread_.join();
	}
	
	Disconnect();
}

void CClientService::Disconnect()
{
	AUTO_LOG_FUNCTION;
	if (connection_established_) {
		shutdown(m_socket, 2);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		disconnected_time_ = std::chrono::steady_clock::now();
		last_conn_time_ = std::chrono::steady_clock::now();
		connection_established_ = false;
	}
}


void CClientService::PrepairToSend(int /*ademco_id*/, const char* buff, size_t buff_size)
{
	std::lock_guard<std::mutex> lock(buffer_lock_);
	std::vector<char> buffer;
	buffer.reserve(buff_size);
	buffer.assign(buff, buff + buff_size);
	buffer_.push_back(buffer);
}


int CClientService::Send(const char* buff, size_t buff_size)
{
	//AUTO_LOG_FUNCTION;
	timeval tv = { 0, 10000 };
	fd_set fdWrite;
	
	int nRet = 0;

	do {
		FD_ZERO(&fdWrite);
		FD_SET(m_socket, &fdWrite);
		nRet = select(m_socket + 1, nullptr, &fdWrite, nullptr, &tv);
		if (!running_)
			break;
	} while (nRet <= 0 && !FD_ISSET(m_socket, &fdWrite));

	nRet = send(m_socket, buff, buff_size, 0);

	if (nRet <= 0) {
		JLOG(_T("CClientService::Send ret <= 0, ret %d"), nRet);
		Disconnect();
	}

	return nRet;
}


void CClientService::ThreadWorker()
{
	AUTO_LOG_FUNCTION;

	timeval tv = { 0, 10 };
	std::chrono::steady_clock::time_point last_send_link_test_time_ = {};
	DWORD dwCount = 0;
	DWORD stepCount = 2000;

	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (!running_) {
			break;
		}

		if (dwCount % stepCount == 0 && !connection_established_) {
			dwCount = 0;
			auto now = std::chrono::steady_clock::now();
			auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_conn_time_).count();
			if (milliseconds > RECONNECT_SERVER_GAP) {
				JLOG(L"%u seconds since last time try to connect transmit server %d", milliseconds / 1000, main_client() ? 1 : 2);
				Connect();
				last_conn_time_ = now;
				if (showed_disconnected_info_to_user_ && connection_established_) {
					//m_handler->OnConnectionEstablished(this);
					showed_disconnected_info_to_user_ = false;
				}
			}
		}

		if (dwCount % stepCount == 0) {
			dwCount = 0;
			auto now = std::chrono::steady_clock::now();
			auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - disconnected_time_).count();
			if (milliseconds > SUPPRESS_DISCONN_TIME) {
				if (m_handler) {
					JLOG(L"%u seconds since disconnected from transmit server %d", milliseconds / 1000, main_client() ? 1 : 2);
					if (!connection_established_) {
						if (!showed_disconnected_info_to_user_) {
							m_handler->OnConnectionLost(this);
							showed_disconnected_info_to_user_ = true;
						}
					}
				}
			}
		}

		// check timeup
		if (++dwCount % stepCount == 0 && connection_established_) {
			dwCount = 0;
			auto now = std::chrono::steady_clock::now();
			auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_recv_time_).count();
			if (milliseconds > CHECK_RECVD_DATA_GAP) {
				JLOG(L"%u seconds no data from transmit server %d", milliseconds / 1000, main_client() ? 1 : 2);
				Disconnect();
			}
		}

		if (!connection_established_) {
			continue;
		}

		if (m_bShuttingDown) {
			break;
		}

		int nRet = 0;
		fd_set fdRead, fdWrite;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_SET(m_socket, &fdRead);
		FD_SET(m_socket, &fdWrite);
		nRet = select(m_socket + 1, &fdRead, &fdWrite, nullptr, &tv);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (!running_) {
			break;
		}
		
		BOOL bRead = FD_ISSET(m_socket, &fdRead);
		BOOL bWrite = FD_ISSET(m_socket, &fdWrite);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (!running_) {
			break;
		}

		if (bRead) {
			char* temp = m_buff.buff + m_buff.wpos;
			DWORD dwLenToRead = BUFF_SIZE - m_buff.wpos;
			nRet = recv(m_socket, temp, dwLenToRead, 0);

			if (nRet <= 0) {
				JLOG(_T("ThreadRecv::recv ret <= 0, ret %d"), nRet);
				Disconnect();
				continue;
			} else if (m_handler) {
				m_buff.wpos += nRet;
				DWORD ret = RESULT_OK;
				ret = m_handler->OnRecv(this);

				while (1) {
					unsigned int bytes_not_commited = m_buff.wpos - m_buff.rpos;
					if (bytes_not_commited == 0) {
						if (m_buff.wpos == BUFF_SIZE) {
							m_buff.Clear();
						}
						break;
					}

					if (m_buff.wpos == BUFF_SIZE) {
						memmove_s(m_buff.buff, BUFF_SIZE,
								  m_buff.buff + m_buff.rpos,
								  bytes_not_commited);
						memset(m_buff.buff + bytes_not_commited,
							   0, BUFF_SIZE - bytes_not_commited);
						m_buff.wpos -= m_buff.rpos;
						m_buff.rpos = 0;
						ret = m_handler->OnRecv(this);
					} else {
						ret = m_handler->OnRecv(this);
					}

					if (ret == RESULT_NOT_ENOUGH) {
						break;
					}
				}

				last_recv_time_ = std::chrono::steady_clock::now();
			}
		}

		if (bWrite) {
			if (m_handler && connection_established_ && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_send_link_test_time_).count() >= LINK_TEST_GAP) {
				last_send_link_test_time_ = std::chrono::steady_clock::now();
				char buff[4096] = { 0 };
				DWORD dwLen = m_handler->GenerateLinkTestPackage(buff, sizeof(buff));
				if (dwLen > 0 && dwLen <= sizeof(buff)) {
					int nLen = Send(buff, dwLen);
					if (nLen <= 0) {
						continue;
					}
				}
			}

			// send data
			if (!buffer_.empty() && buffer_lock_.try_lock()) {
				std::lock_guard<std::mutex> lock(buffer_lock_, std::adopt_lock);
				auto buffer = buffer_.front();
				Send(&buffer[0], buffer.size());
				buffer_.pop_front();
			}
		}
	}
}


class CMyClientEventHandler : public CClientEventHandler
{
	enum DEAL_CMD_RET
	{
		DCR_NULL,
		DCR_ONLINE,
		DCR_ACK,
		DCR_DUH,
	};
	
public:
	
	CMyClientEventHandler() : m_conn_id(0xFFFFFFFF){}
	virtual ~CMyClientEventHandler() {}
	virtual void OnConnectionEstablished(CClientService* service)
	{
		AUTO_LOG_FUNCTION;
		PostMessageToMainWnd(WM_NETWORKSTARTUPOK, 1, service->main_client());
	}

	virtual void OnConnectionLost(CClientService* service)
	{
		AUTO_LOG_FUNCTION;
		auto iter = m_clientsMap.begin();
		while (iter != m_clientsMap.end()) {
			if (iter->second && iter->second->online) {
				iter = HandleOffline(iter->first);
			} else {
				iter = m_clientsMap.erase(iter);
			}
		}

		PostMessageToMainWnd(WM_NETWORKSTARTUPOK, 0, service->main_client());
		m_conn_id = 0xFFFFFFFF;
	}

	virtual DWORD OnRecv(CClientService* service);
	virtual DWORD GenerateLinkTestPackage(char* buff, size_t buff_len);
	DEAL_CMD_RET handle_cmd(CClientService* service);
	DEAL_CMD_RET handle_cmd_02(CClientService* service, DWORD conn_id);
	DEAL_CMD_RET handle_cmd_05(CClientService* service, DWORD conn_id);
	DEAL_CMD_RET handle_cmd_07(CClientService* service, DWORD conn_id);
	DEAL_CMD_RET handle_cmd_0b(CClientService* service, DWORD conn_id);
	DEAL_CMD_RET handle_cmd_0d(CClientService* service, DWORD conn_id);

	inline int GetConnID() const
	{
		return m_conn_id;
	}

protected:
	typedef struct _CLIENT_DATA
	{
		bool online;
		//int conn_id;
		int ademco_id;
		_CLIENT_DATA() : online(false), /*conn_id(-1), */ademco_id(-1) {}
	}CLIENT_DATA;

	typedef std::shared_ptr<CLIENT_DATA> ClientDataPtr;

	std::map<int, ClientDataPtr> m_clientsMap;

	DWORD OnRecv2(CClientService* service);
	std::map<int, ClientDataPtr>::iterator HandleOffline(int conn_id) {
		AUTO_LOG_FUNCTION;
		auto mgr = core::alarm_machine_manager::get_instance();
		auto iter = m_clientsMap.find(conn_id);
		if (iter != m_clientsMap.end() && iter->second && iter->second->online) {
			mgr->MachineOnline(_event_source, iter->second->ademco_id, FALSE);
			iter->second->online = false;
			core::alarm_machine_ptr machine = mgr->GetMachine(iter->second->ademco_id);
			if (machine) {
				if (_event_source == ES_TCP_SERVER1) {
					machine->SetPrivatePacketFromServer1(nullptr);
				} else if (_event_source == ES_TCP_SERVER2) {
					machine->SetPrivatePacketFromServer2(nullptr);
				} else {
					machine->SetPrivatePacketFromServer1(nullptr);
					machine->SetPrivatePacketFromServer2(nullptr);
				}
			}
			return m_clientsMap.erase(iter);
		}
		return m_clientsMap.end();
	}

	void HandleLinkTest(CClientService* service) {
		auto mgr = core::alarm_machine_manager::get_instance();
		auto t = time(nullptr);
		for (auto iter : m_clientsMap) {
			if (iter.second && iter.second->online) {
				mgr->MachineEventHandler(service->main_client() ? core::ES_TCP_SERVER1 : core::ES_TCP_SERVER2, 
										 iter.second->ademco_id, EVENT_LINK_TEST, 0, 0, t, t);
			}
		}
	}

	void HandleInvalidClient(int ademco_id) {
		auto now = time(nullptr);
		auto iter = invalid_client_conn_.find(ademco_id);
		if (iter == invalid_client_conn_.end()) {
			invalid_client_conn_[ademco_id] = now;
		} else {
			double diff = std::difftime(now, iter->second);
			if (diff < 6)
				return;
			else
				invalid_client_conn_[ademco_id] = now;
		}

		CString fm, rec;
		fm = TR(IDS_STRING_FM_KICKOUT_INVALID);
		rec.Format(fm, ademco_id);
		auto hr = core::history_record_manager::get_instance();
		hr->InsertRecord(ademco_id, 0, rec, now, core::RECORD_LEVEL_STATUS);
		JLOG(rec);
		JLOG(_T("Check acct-aid failed, pass.\n"));
	}
private:
	DWORD m_conn_id;
	AdemcoPacket m_packet1;
	PrivatePacket m_packet2;
	std::map<int, time_t> invalid_client_conn_;
};


BOOL CClient::Start(const std::string& server_ip, unsigned int server_port)
{
	AUTO_LOG_FUNCTION;
	if (m_bClientServiceStarted)
		return TRUE;

	try {
		if (nullptr == _client_service) {
			_client_service = std::make_shared<CClientService>(main_client_);
		}
		
		if (nullptr == _client_event_handler) {
			_client_event_handler = std::make_shared<CMyClientEventHandler>();
			_client_event_handler->set_event_source(_event_source);
		}

		_client_service->SetEventHandler(_client_event_handler);
		_client_service->Start(server_ip, server_port);

		m_bClientServiceStarted = TRUE;
	} catch (const char* err) {
		OutputDebugStringA(err);
	} catch (const wchar_t* err) {
		OutputDebugStringW(err);
	}

	return m_bClientServiceStarted;
}


void CClient::Stop()
{
	AUTO_LOG_FUNCTION;

	if (nullptr != _client_event_handler && nullptr != _client_service) {
		if(_client_service->connection_established_)
			_client_event_handler->OnConnectionLost(_client_service.get());
		_client_service->Stop();
		_client_service = nullptr;
		_client_event_handler = nullptr;
	}

	m_bClientServiceStarted = FALSE;
}


int CClient::SendToTransmitServer(int ademco_id, ADEMCO_EVENT ademco_event, int gg, int zone, 
								  const ademco::char_array_ptr& xdata, 
								  const ademco::char_array_ptr& cmd)
{
	AUTO_LOG_FUNCTION;
	if (_client_service) {
		char data[BUFF_SIZE] = { 0 };
		core::alarm_machine_ptr machine = core::alarm_machine_manager::get_instance()->GetMachine(ademco_id);
		if (machine) {
			static AdemcoPacket packet;
			const PrivatePacketPtr privatePacket = (_event_source == ES_TCP_SERVER1) ? machine->GetPrivatePacketFromServer1() : machine->GetPrivatePacketFromServer2();
			if (!privatePacket) {
				JLOGA("privatePacket is nullptr");
				return 0;
			}
			DWORD dwSize = 0;
			dwSize = packet.Make(data, sizeof(data), AID_HB, _client_service->get_cur_seq(),
									ademco::HexCharArrayToStr(privatePacket->_acct_machine, 9),
									ademco_id, ademco_event,
									gg, zone, xdata);
			char_array private_cmd;
			AppendConnIdToCharArray(private_cmd, GetConnIdFromCharArray(privatePacket->_cmd));
			static PrivatePacket packet2;
			bool default_0c_01 = false;
			if (is_machine_can_only_add_zone_by_retrieve(machine->get_machine_type())) {
				switch (ademco_event)
				{
				case EVENT_ENTER_SET_MODE:
					private_cmd.push_back(1); // 1 for enter set mode, 0 for leave set mode.
					dwSize += packet2.Make(data + dwSize, sizeof(data) - dwSize, 0x0a, 0x0b, private_cmd,
										   privatePacket->_acct_machine,
										   privatePacket->_passwd_machine,
										   privatePacket->_acct,
										   privatePacket->_level);
					break;

				case EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE:
					std::copy(cmd->begin(), cmd->end(), std::back_inserter(private_cmd));
					dwSize += packet2.Make(data + dwSize, sizeof(data) - dwSize, 0x0a, 0x0c, private_cmd,
										   privatePacket->_acct_machine,
										   privatePacket->_passwd_machine,
										   privatePacket->_acct,
										   privatePacket->_level);
					break;

				case EVENT_STOP_RETRIEVE:
					private_cmd.push_back(0); // 1 for enter set mode, 0 for leave set mode.
					dwSize += packet2.Make(data + dwSize, sizeof(data) - dwSize, 0x0a, 0x0b, private_cmd,
										   privatePacket->_acct_machine,
										   privatePacket->_passwd_machine,
										   privatePacket->_acct,
										   privatePacket->_level);
					break;

				default:
					default_0c_01 = true;
					break;
				}
			}
			
			if (default_0c_01) {
				if (ademco_event == EVENT_WHAT_IS_YOUR_TYPE) {
					dwSize += packet2.Make(data + dwSize, sizeof(data) - dwSize, 0x04, 0x05, private_cmd,
										   privatePacket->_acct_machine,
										   privatePacket->_passwd_machine,
										   privatePacket->_acct,
										   privatePacket->_level);
				} else {
					dwSize += packet2.Make(data + dwSize, sizeof(data) - dwSize, 0x0c, 0x00, private_cmd,
										   privatePacket->_acct_machine,
										   privatePacket->_passwd_machine,
										   privatePacket->_acct,
										   privatePacket->_level);
				}
				
			}
			
			_client_service->PrepairToSend(ademco_id, data, dwSize);
			return 1;
		} else {
			JLOGA("machine is nullptr");
		}
	} else {
		JLOGA("_client_service is nullptr");
	}
	return 0;
}

DWORD CMyClientEventHandler::GenerateLinkTestPackage(char* buff, size_t buff_len)
{
	//AUTO_LOG_FUNCTION;
	if (m_conn_id == -1)
		return 0;
	static int seq = 1;
	if (seq >= 9999)
		seq = 1;
	DWORD dwLen = m_packet1.Make(buff, buff_len, AID_NULL, seq++, nullptr, 0, ademco::EVENT_INVALID_EVENT, 0, 0);
	ConnID conn_id = m_conn_id;
	char_array cmd;
	AppendConnIdToCharArray(cmd, conn_id);
	static PrivatePacket packet2;
	char acct[9] = { 0 };
	NumStr2HexCharArray_N(util::CConfigHelper::get_instance()->get_csr_acct().c_str(), acct);
	dwLen += m_packet2.Make(buff + dwLen, buff_len - dwLen, 0x06, 0x00, cmd, nullptr, nullptr, 
							acct, 0);
	return dwLen;
}

DWORD CMyClientEventHandler::OnRecv(CClientService* service)
{
	size_t dwBytesCmted = 0;
	ParseResult result1 = m_packet1.Parse(service->m_buff.buff + service->m_buff.rpos,
										  service->m_buff.wpos - service->m_buff.rpos,
										  dwBytesCmted);

	if (result1 == RESULT_OK) {
		//core::congwin_fe100_mgr::get_instance()->add_event(&m_packet1._ademco_data);
		service->m_buff.rpos = (service->m_buff.rpos + dwBytesCmted);
		return OnRecv2(service);
	} else if (result1 == RESULT_NOT_ENOUGH) {
		return RESULT_NOT_ENOUGH;
	} else if (result1 == RESULT_DATA_ERROR) {
		DWORD result2 =  OnRecv2(service);
		if (result2 == RESULT_OK) {
			return RESULT_OK;
		} else if (result2 == RESULT_NOT_ENOUGH) {
			return RESULT_NOT_ENOUGH;
		} else if (result2 == RESULT_DATA_ERROR) {
			service->m_buff.Clear();
			return RESULT_OK;
		}
	} else {
		ASSERT(0);
		service->m_buff.Clear();
	}
	return RESULT_OK;
}


DWORD CMyClientEventHandler::OnRecv2(CClientService* service)
{
	size_t dwBytesCmted = 0;
	ParseResult result2 = m_packet2.Parse(service->m_buff.buff + service->m_buff.rpos,
										  service->m_buff.wpos - service->m_buff.rpos,
										  dwBytesCmted);
	if (RESULT_DATA_ERROR == result2) {
		ASSERT(0);
		service->m_buff.Clear();
		return RESULT_OK;
	} else if (RESULT_NOT_ENOUGH == result2) {
		return RESULT_NOT_ENOUGH;
	} else {
		service->m_buff.rpos = (service->m_buff.rpos + dwBytesCmted);
		
		char buff[1024] = { 0 };
		DEAL_CMD_RET dcr = handle_cmd(service);
		
		if (ademco::is_same_id(m_packet1._id, ademco::AID_DUH)) {
			CString record = _T("");
			record = TR(IDS_STRING_ILLEGAL_OP);
			JLOG(record);
#ifdef _DEBUG
			core::history_record_manager::get_instance()->InsertRecord(m_packet1._ademco_data._ademco_id, 0, record,
																	   time(nullptr), core::RECORD_LEVEL_STATUS);
#endif
		}

		char _seq[4];
		std::copy(m_packet1._seq.begin(), m_packet1._seq.end(), _seq);
		int seq = ademco::NumStr2Dec(_seq, 4);
		if (seq > 9999) seq = 1;
		
		if (dcr == DCR_ONLINE) {
			auto csr_acct = util::CConfigHelper::get_instance()->get_csr_acct();
			if (!csr_acct.empty() && csr_acct.length() <= 18) {
				size_t len = m_packet1.Make(buff, sizeof(buff), AID_HB, 0, nullptr,
											m_packet1._ademco_data._ademco_id, ademco::EVENT_INVALID_EVENT, 0, 0);
				char_array cmd;
				AppendConnIdToCharArray(cmd, ConnID(m_conn_id));
				char temp[9] = { 0 };
				NumStr2HexCharArray_N(csr_acct.c_str(), temp, 9);
				for (auto t : temp) { cmd.push_back(t); }
				len += m_packet2.Make(buff + len, sizeof(buff) - len, 0x06, 0x01, cmd,
									  m_packet2._acct_machine,
									  m_packet2._passwd_machine,
									  m_packet2._acct,
									  m_packet2._level);
				service->PrepairToSend(-1, buff, len);
			}
		} else if (dcr == DCR_ACK) {
			size_t len = m_packet1.Make(buff, sizeof(buff), AID_ACK, seq,
									    ademco::HexCharArrayToStr(m_packet2._acct_machine, 9),
										m_packet1._ademco_data._ademco_id, ademco::EVENT_INVALID_EVENT, 0, 0);
			char_array cmd;
			AppendConnIdToCharArray(cmd, GetConnIdFromCharArray(m_packet2._cmd));
			char csr_acct[9] = { 0 };
			auto acct = util::CConfigHelper::get_instance()->get_csr_acct();
			ademco::NumStr2HexCharArray_N(acct.c_str(), csr_acct, 9);
			len += m_packet2.Make(buff + len, sizeof(buff) - len, 0x0c, 0x01, cmd,
								  m_packet2._acct_machine,
								  m_packet2._passwd_machine,
								  csr_acct,
								  m_packet2._level);
			service->PrepairToSend(m_packet1._ademco_data._ademco_id, buff, len);
		} else if (dcr == DCR_DUH) {
			size_t len = m_packet1.Make(buff, sizeof(buff), AID_DUH, seq,
									  ademco::HexCharArrayToStr(m_packet2._acct_machine, 9),
										m_packet1._ademco_data._ademco_id, ademco::EVENT_INVALID_EVENT, 0, 0);
			char_array cmd;
			AppendConnIdToCharArray(cmd, GetConnIdFromCharArray(m_packet2._cmd));
			len += m_packet2.Make(buff + len, sizeof(buff) - len, 0x0c, 0x01, cmd,
								  m_packet2._acct_machine,
								  m_packet2._passwd_machine,
								  m_packet2._acct,
								  m_packet2._level);
			service->PrepairToSend(m_packet1._ademco_data._ademco_id, buff, len);
		}
		return RESULT_OK;
	}
}


CMyClientEventHandler::DEAL_CMD_RET CMyClientEventHandler::handle_cmd(CClientService* service)
{
	DWORD conn_id = GetConnIdFromCharArray(m_packet2._cmd).ToInt();
	JLOG(L"conn_id %d, 0x%02x 0x%02x", conn_id, m_packet2._big_type, m_packet2._lit_type);
	auto mgr = core::alarm_machine_manager::get_instance(); ASSERT(mgr);
	switch (m_packet2._big_type) 
	{
		case 0x02: // from machine
		{
			return handle_cmd_02(service, conn_id);
		} // end case 02
		break;
		
		case 0x05: // from machine
		{ 
			return handle_cmd_05(service, conn_id);
		} // end case 0x05
		break;
		
		case 0x07: // from Transmit server
		{			
			return handle_cmd_07(service, conn_id);
		} // end case 07
		break;

		case 0x0b: // from alarm machine
		{
			return handle_cmd_0b(service, conn_id);
		}
			break;
			 
		case 0x0d: // from Alarm Machine
		{	
			return handle_cmd_0d(service, conn_id);
		} // end 0x0d
		break;
	default:
		break;
	}

	return DCR_NULL;
}


CMyClientEventHandler::DEAL_CMD_RET CMyClientEventHandler::handle_cmd_02(CClientService* /*service*/, DWORD conn_id)
{
	auto mgr = core::alarm_machine_manager::get_instance();

	if (m_packet2._lit_type == 0x00) { // machine link test, check signal strength
		if (m_clientsMap[conn_id] && m_clientsMap[conn_id]->online) {
			auto machine = mgr->GetMachine(m_clientsMap[conn_id]->ademco_id);
			if (machine) {
				auto xdata = std::make_shared<char_array>();
				xdata->push_back(m_packet2._ip_csr[1]);
				mgr->MachineEventHandler(_event_source, m_clientsMap[conn_id]->ademco_id,
										 EVENT_SIGNAL_STRENGTH_CHANGED, 0,
										 0, time(nullptr), time(nullptr), xdata);
			}
		}
	} else if (m_packet2._lit_type == 0x06) {
		if (m_clientsMap[conn_id] && m_clientsMap[conn_id]->online) {
			auto machine = mgr->GetMachine(m_clientsMap[conn_id]->ademco_id);
			if (machine && m_packet2._cmd.size() >= 4) {
				bool sms_mode = m_packet2._cmd[3] == 0 ? false : true;
				if (sms_mode != machine->get_sms_mode()) {
					machine->set_sms_mode(sms_mode);
					CString txt = machine->get_formatted_name();
					if (sms_mode) {
						txt += TR(IDS_STRING_ENTER_SMS_MODE);
					} else {
						txt += TR(IDS_STRING_LEAVE_SMS_MODE);
					}
					core::history_record_manager::get_instance()->InsertRecord(m_clientsMap[conn_id]->ademco_id, 0, txt, time(nullptr), core::RECORD_LEVEL_STATUS);
				}
			}
		}
	}
	return DCR_NULL;
}

CMyClientEventHandler::DEAL_CMD_RET CMyClientEventHandler::handle_cmd_05(CClientService * /*service*/, DWORD conn_id)
{
	auto mgr = core::alarm_machine_manager::get_instance();
	if (m_packet2._lit_type == 0x00) {
		try {
			int ademco_id = m_packet1._ademco_data._ademco_id;
			int zone = m_packet1._ademco_data._zone;
			int subzone = m_packet1._ademco_data._gg;
			ADEMCO_EVENT ademco_event = m_packet1._ademco_data._ademco_event;

			JLOGA("alarm machine EVENT: 05 00 aid %04d event %04d zone %03d\n",
				  ademco_id, ademco_event, zone);

			bool ok = true;

			do {
				if (m_clientsMap[conn_id] && m_clientsMap[conn_id]->online && m_clientsMap[conn_id]->ademco_id != ademco_id) {
					HandleOffline(conn_id);
				}

				if (!m_clientsMap[conn_id]) {
					m_clientsMap[conn_id] = std::make_shared<CLIENT_DATA>();
				}

				if (!mgr->CheckIsValidMachine(ademco_id, /*acct, */zone)) {
					ok = FALSE; break;
				}

				core::alarm_machine_ptr machine = mgr->GetMachine(ademco_id);
				if (!machine) {
					JLOG(L"machine %04d is not created!", ademco_id);
					ok = false;
					break;
				}

				if (!m_clientsMap[conn_id]->online || (machine && !machine->get_online())) {
					char acct[64] = { 0 };
					std::copy(m_packet1._acct.begin(), m_packet1._acct.end(), acct);
					JLOGA("alarm machine: 05 00 aid %04d acct %s online.\n",
						  ademco_id, acct);

					if (machine) {
						auto csr_acct = util::CConfigHelper::get_instance()->get_csr_acct();
						char temp[9] = { 0 };
						NumStr2HexCharArray_N(csr_acct.c_str(), temp, 9);
						memcpy(m_packet2._acct, temp, 9);
						(_event_source == ES_TCP_SERVER1) ? machine->SetPrivatePacketFromServer1(&m_packet2) : machine->SetPrivatePacketFromServer2(&m_packet2);
					}

					m_clientsMap[conn_id]->online = true;
					m_clientsMap[conn_id]->ademco_id = ademco_id;
					mgr->MachineOnline(_event_source, ademco_id);

				}

				mgr->MachineEventHandler(_event_source, ademco_id, ademco_event, zone,
										 subzone, time(nullptr), time(nullptr),
										 m_packet1._xdata);
			} while (0);

			if (!ok) {
				HandleInvalidClient(ademco_id);
			}

			return ok ? DCR_ACK : DCR_DUH;

		} catch (...) {
			return DCR_DUH;
		} // end try
	} // end 05 00
	else if (m_packet2._lit_type == 0x04) { // machine type
		try {
			int ademco_id = m_packet1._ademco_data._ademco_id;
			int type = m_packet2._cmd.at(6);
			auto data = m_clientsMap[conn_id];

			if (data && data->online) {
				if (ademco_id != data->ademco_id)
					ademco_id = data->ademco_id;

				ADEMCO_EVENT ademco_event = EVENT_INVALID_EVENT;

				switch (type) {
				case 0: // WiFi主机
					ademco_event = EVENT_I_AM_WIFI_MACHINE;
					break;

				case 1: // 网络摄像机主机
					break;

				case 2: // 3G模块主机
					break;

				case 3: // 网络模块主机
					ademco_event = EVENT_I_AM_NET_MODULE;
					break;

				case 4: // 改进型卧室主机2505型
					ademco_event = EVENT_I_AM_EXPRESSED_GPRS_2050_MACHINE;
					break;

				case 5: // 液晶主机
					ademco_event = EVENT_I_AM_LCD_MACHINE;
					break;

				case 6: // 网线主机
					ademco_event = EVENT_I_AM_WIRE_MACHINE;
					break;

				default:
					ademco_event = EVENT_INVALID_EVENT;
					break;
				}

				if (ademco_event != EVENT_INVALID_EVENT) {
					JLOGA("alarm machine EVENT: 05 04 aid %04d type %d %s\n",
						  ademco_id, type, GetAdemcoEventStringEnglish(ademco_event).c_str());
					mgr->MachineEventHandler(_event_source, ademco_id, ademco_event, 0, 0, time(nullptr), time(nullptr));
					return DCR_NULL;
				}
			}
		} catch (...) {
			return DCR_DUH;
		}
	} // end 05 04
	else if (m_packet2._lit_type == 0x13) { // 05 13 machine restoring factory settings
		int ademco_id = m_packet1._ademco_data._ademco_id;

		auto data = m_clientsMap[conn_id];
		if (data && data->online) {
			if (ademco_id != data->ademco_id)
				ademco_id = data->ademco_id;

			auto t = time(nullptr);
			mgr->MachineEventHandler(_event_source, ademco_id, EVENT_RESTORE_FACTORY_SETTINGS, 0, 0, t, t);
			HandleOffline(conn_id);
		}
	}// end 05 13

	return DCR_NULL;
}

CMyClientEventHandler::DEAL_CMD_RET CMyClientEventHandler::handle_cmd_07(CClientService * service, DWORD conn_id)
{
	switch (m_packet2._lit_type) {
	case 0x00:	// link test responce
		JLOGA("07 00 Transmite server link test responce\n");
		HandleLinkTest(service);
		break;
	case 0x01:	// conn_id
		m_conn_id = conn_id;
		JLOGA("07 01 Transmite server responce my conn_id %d\n", conn_id);
		OnConnectionEstablished(service);
		return DCR_ONLINE;
		break;
	case 0x02:	// alarm machine connection lost
		JLOGA("07 02 Transmite server told me one machine offline, conn_id %d\n", conn_id);
		if (m_clientsMap[conn_id] && m_clientsMap[conn_id]->online) {
			HandleOffline(conn_id);
		}
		break;
	case 0x03: // same acct csr already online
	{
		JLOGA("07 03 Transmite server told me one csr with my acct already online\n");
		AfxMessageBox(TR(IDS_STRING_SAME_ACCT_CSR_ALREADY_ONLINE));
		QuitApplication(0);
	}
	break;
	default:
		break;
	}

	return DCR_NULL;
}

CMyClientEventHandler::DEAL_CMD_RET CMyClientEventHandler::handle_cmd_0b(CClientService * /*service*/, DWORD conn_id)
{
	auto mgr = core::alarm_machine_manager::get_instance();
	if (m_packet2._lit_type == 0x0b && 5 == m_packet2._cmd.size()) { // responce of enter set mode
		int ademco_id = m_packet1._ademco_data._ademco_id;
		bool b_enter = 1 == m_packet2._cmd.at(3);
		bool b_ok = 1 == m_packet2._cmd.at(4);
		ADEMCO_EVENT ademco_event = b_ok ? EVENT_ENTER_SET_MODE : EVENT_STOP_RETRIEVE;
		auto data = m_clientsMap[conn_id];

		if (data && data->online) {
			if (ademco_id != data->ademco_id)
				ademco_id = data->ademco_id;

			auto t = time(nullptr);

			char temp[9] = { 0 };
			auto csr_acct = util::CConfigHelper::get_instance()->get_csr_acct();
			NumStr2HexCharArray_N(csr_acct.c_str(), temp, 9);
			if (b_enter && memcmp(temp, m_packet2._acct, 9) == 0) {
				mgr->MachineEventHandler(_event_source, ademco_id, ademco_event, 0, 0, t, t);
			}

			// 2016-11-26 18:03:01 
			if (b_ok) {
				mgr->MachineEventHandler(_event_source, ademco_id, b_enter ? EVENT_ENTER_SETTING_MODE : EVENT_EXIT_SETTING_MODE, 0, 0, t, t);
			}
		}
	} else if (m_packet2._lit_type == 0x0c) { // responce of retrieve zone info
		int ademco_id = m_packet1._ademco_data._ademco_id;
		ADEMCO_EVENT ademco_event = EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE;

		auto data = m_clientsMap[conn_id];
		if (data && data->online) {
			if (ademco_id != data->ademco_id)
				ademco_id = data->ademco_id;

			char_array_ptr xdata = std::make_shared<char_array>(); // ademco xdata segment
			if (m_packet2._cmd.size() >= 14) { // 14 is the minimal length of a responce
				std::copy(m_packet2._cmd.begin() + 6, m_packet2._cmd.end(), std::back_inserter(*xdata));
				auto t = time(nullptr);
				mgr->MachineEventHandler(_event_source, ademco_id, ademco_event, 0, 0, t, t, xdata);
			}
		}
	}

	return DCR_NULL;
}

CMyClientEventHandler::DEAL_CMD_RET CMyClientEventHandler::handle_cmd_0d(CClientService * /*service*/, DWORD conn_id)
{
	auto mgr = core::alarm_machine_manager::get_instance();

	int ademco_id = m_packet1._ademco_data._ademco_id;
	int zone = m_packet1._ademco_data._zone;
	int subzone = m_packet1._ademco_data._gg;
	ADEMCO_EVENT ademco_event = m_packet1._ademco_data._ademco_event;

	if (m_packet2._lit_type == 0x00) {	// Alarm machine on/off line, event report.
		JLOGA("alarm machine EVENT: 0d 00 aid %04d event %04d zone %03d\n",
			  ademco_id, ademco_event, zone);

		BOOL ok = TRUE;
		do {
			if (m_clientsMap[conn_id] && m_clientsMap[conn_id]->online && m_clientsMap[conn_id]->ademco_id != ademco_id) {
				HandleOffline(conn_id);
				//ok = FALSE; break;
				//m_clients[conn_id].online = false;
			}

			if (!m_clientsMap[conn_id]) {
				m_clientsMap[conn_id] = std::make_shared<CLIENT_DATA>();
			}

			if (!mgr->CheckIsValidMachine(ademco_id, /*acct, */zone)) {
				ok = FALSE; break;
			}

			core::alarm_machine_ptr machine = mgr->GetMachine(ademco_id);

			if (!m_clientsMap[conn_id]->online || (machine && !machine->get_online())) {
				char acct[64] = { 0 };
				std::copy(m_packet1._acct.begin(), m_packet1._acct.end(), acct);
				JLOGA("alarm machine ONLINE:0d 00 aid %04d acct %s online.\n",
					  ademco_id, acct);

				if (machine) {
					(_event_source == ES_TCP_SERVER1) ? machine->SetPrivatePacketFromServer1(&m_packet2) : machine->SetPrivatePacketFromServer2(&m_packet2);
				}

				m_clientsMap[conn_id]->online = true;
				m_clientsMap[conn_id]->ademco_id = ademco_id;
				mgr->MachineOnline(_event_source, ademco_id);

			}

			mgr->MachineEventHandler(_event_source, ademco_id, ademco_event, zone,
									 subzone, time(nullptr), time(nullptr),
									 m_packet1._xdata);
		} while (0);

		if (!ok) {
			HandleInvalidClient(ademco_id);
		}

		return ok ? DCR_ACK : DCR_DUH;
	} else if (m_packet2._lit_type == 0x01) { // 0D 01
		try {
			if (m_clientsMap[conn_id] && m_clientsMap[conn_id]->online) {
				ademco_id = m_clientsMap[conn_id]->ademco_id;
				JLOGA("alarm machine EVENT:0d 01 aid %04d event %04d zone %03d %s\n",
					  ademco_id, ademco_event, zone, m_packet1._timestamp._data);
				auto cmd = m_packet2._cmd;
				static ADEMCO_EVENT cStatus[] = { EVENT_ARM, EVENT_HALFARM, EVENT_DISARM, EVENT_ENTER_SETTING_MODE };
				char machine_status = cmd[6];

				if (machine_status < 4) {
					mgr->MachineEventHandler(_event_source, ademco_id, cStatus[machine_status], zone,
											 subzone, time(nullptr), time(nullptr),
											 m_packet1._xdata);
				}
				return DCR_ACK;
			} else {
				return DCR_DUH;
			}
		} catch (...) {
			return DCR_DUH;
		}
	}	
	
	return DCR_NULL;
}

};};

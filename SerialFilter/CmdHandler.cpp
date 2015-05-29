// CmdHandler.cpp: implementation of the CCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CmdHandler.h"
#include "GenericBuffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define MY_SERIAL_NDX 0x03


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCmdHandler::CCmdHandler()
	: m_hEventExit(INVALID_HANDLE_VALUE)
	, m_hThreadDeal(INVALID_HANDLE_VALUE)
	, m_pRecvBuffer(NULL)
	, m_ucKeyboardAddr(MY_SERIAL_NDX)
{
	m_pRecvBuffer = new CGenericBuffer();
	ASSERT(m_pRecvBuffer);

	m_hEventExit = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadDeal = CreateThread(NULL, 0, ThreadDeal, this, 0, NULL);
	if (InitPort(NULL, 1, 9600)) {
		StartMonitoring();
	} else {
		AfxMessageBox(L"NO COM");
		::PostQuitMessage(0);
	}
}

CCmdHandler::~CCmdHandler()
{
#ifdef USE_TCP_NOT_SERIAL
	StopWork();
#else
	ClosePort();
#endif
	SAFEDELETEP(m_pRecvBuffer);
}

__forceinline static BYTE MakeSum(const char *data, int len)
{
	BYTE sum = 0;
	for (int i = 0; i < len - 1; i++) {
		sum = static_cast<BYTE>(sum + static_cast<BYTE>(*(data + i)));
	}
	return sum;
}

BOOL CCmdHandler::OnRecv(const char *cmd, WORD wLen)
{
	return m_pRecvBuffer->Write(cmd, wLen);
}


BOOL CCmdHandler::DealCmdKeyboard(const char *cmd, WORD wLen)
{
	do {
		const BYTE cmd_chk = static_cast<BYTE>(cmd[wLen - 1]);
		const BYTE compute_chk = MakeSum(cmd, wLen);
		if (cmd_chk != compute_chk)
			break;
		static const BYTE _EB = 0xeb;
		static const BYTE _AE = 0xae;
		if (_EB != static_cast<BYTE>(cmd[0]) || _AE != static_cast<BYTE>(cmd[1]))
			break;

		BYTE hundred = static_cast<BYTE>(cmd[5]);
		BYTE decade = static_cast<BYTE>(cmd[6]);
		BYTE single = static_cast<BYTE>(cmd[7]);

		int idArea = -1;
		static const BYTE _INVALID_KEYBOARD_ADDR = 0xfa;
		//if (_INVALID_KEYBOARD_ADDR != static_cast<BYTE>(cmd[2])) {
		//	m_ucKeyboardAddr = static_cast<BYTE>(cmd[2]);
		//}MY_SERIAL_NDX
		if (hundred == 0xff || decade == 0xff || single == 0xff) {
			idArea = -1;
		} else {
			idArea = (hundred & 0x0f) * 100 + (decade & 0x0f) * 10 + (single & 0x0f);
		}


		return TRUE;
	} while (0);
	return FALSE;
}

BOOL CCmdHandler::DealCmdComputer(const char* cmd, WORD wLen)
{
	do {
		const BYTE cmd_chk = static_cast<BYTE>(cmd[wLen - 1]);
		const BYTE compute_chk = MakeSum(cmd, wLen);
		if (cmd_chk != compute_chk) {
			CLog::WriteLog(_T("CCmdHandler::DealCmdComputer cheksum invalid**************************************\n"));
			//break;
			//ASSERT(0);
		}
		static const BYTE _EB = 0xeb;
		static const BYTE _B0 = 0xb1;
		if (_EB != static_cast<BYTE>(cmd[0]) || _B0 != static_cast<BYTE>(cmd[1])) {
			CLog::WriteLog(_T("CCmdHandler::DealCmdComputer bigclass litclass invalid\n"));
			break;
		}
		return TRUE;
	} while (0);
	return FALSE;
}



//DWORD WINAPI CCmdHandler::ThreadDeal(LPVOID lp)
//{
//	//return 0;
//	CLog::WriteLog(_T("core dump CCmdHandler::ThreadDeal tid %d"), GetCurrentThreadId());
//	CCmdHandler *pthis = static_cast<CCmdHandler*>(lp);
//	DWORD dwPrevTimePoint = GetTickCount();
//	DWORD dwPackageNum = 0;
//#define CMD_LEN_SERIAL_MAX 10
//	char cmd[CMD_LEN_SERIAL_MAX] = { 0 };
//	while (1) {
//		if (WaitForSingleObject(pthis->m_hEventExit, 1) == WAIT_OBJECT_0)
//			break;
//
//		if (pthis->m_pRecvBuffer->GetValidateLen() >= CMD_LEN_SERIAL_MIN) {
//			do {
//				if (pthis->m_pRecvBuffer->Read(cmd, 1) != 1)
//					break;
//				if (static_cast<BYTE>(cmd[0]) != 0xEB)
//					break;
//				if (pthis->m_pRecvBuffer->Read(cmd + 1, 1) != 1)
//					break;
//
//				switch (static_cast<BYTE>(cmd[1])) {
//					case 0xAE:	// 主机到键盘，长度为 CMD_LEN_SERIAL_KEYBOARD
//					{
//						if (WaitForSingleObject(pthis->m_hEventExit, 1) == WAIT_OBJECT_0)
//							break;
//						int recv = pthis->m_pRecvBuffer->Read(cmd + 2, CMD_LEN_SERIAL_KEYBOARD - 2);
//						while (recv != CMD_LEN_SERIAL_KEYBOARD - 2) {
//							if (WaitForSingleObject(pthis->m_hEventExit, 1) == WAIT_OBJECT_0)
//								break;
//							recv += pthis->m_pRecvBuffer->Read(cmd + 2 + recv, CMD_LEN_SERIAL_KEYBOARD - 2 - recv);
//						}
//						pthis->DealCmdKeyboard(cmd, CMD_LEN_SERIAL_KEYBOARD);
//					}
//						break;
//					case 0xB1:	// 主机到软件，长度为 CMD_LEN_SERIAL_COMPUTER
//						if (pthis->m_pRecvBuffer->Read(cmd + 2, CMD_LEN_SERIAL_COMPUTER - 2) == CMD_LEN_SERIAL_COMPUTER - 2)
//							pthis->DealCmdComputer(cmd, CMD_LEN_SERIAL_COMPUTER);
//						break;
//					case 0xFF:
//						CLog::WriteLog(_T("Recv HeartBeatPkg"));
//						break;
//					default:
//						break;
//				}
//
//				// 检查是否忙碌
//				if (GetTickCount() - dwPrevTimePoint >= TIME_RANGE_FOR_CHECK_BUSY) {
//					dwPrevTimePoint = GetTickCount();
//					dwPackageNum = 0;
//				} else {
//					pthis->m_bBusy = (++dwPackageNum >= PACKAGE_NUM_FOR_CHECK_BUSY);
//				}
//			} while (0);
//
//			// 超时恢复与处理
//			/*dwLastTimeRecv = GetTickCount();
//			if(bDisconnected)
//			{
//			pthis->OnConnectionEstablished();
//			bDisconnected = FALSE;
//			}*/
//		}
//	}
//	return 0;
//}


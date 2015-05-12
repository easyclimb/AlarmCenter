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



DWORD WINAPI CCmdHandler::ThreadDeal(LPVOID lp)
{
	//return 0;
	CLog::WriteLog(_T("core dump CCmdHandler::ThreadDeal tid %d"), GetCurrentThreadId());
	CCmdHandler *pthis = static_cast<CCmdHandler*>(lp);
	DWORD dwPrevTimePoint = GetTickCount();
	DWORD dwPackageNum = 0;
#define CMD_LEN_SERIAL_MAX 10
	char cmd[CMD_LEN_SERIAL_MAX] = { 0 };
	while (1) {
		if (WaitForSingleObject(pthis->m_hEventExit, 1) == WAIT_OBJECT_0)
			break;

		if (pthis->m_pRecvBuffer->GetValidateLen() >= CMD_LEN_SERIAL_MIN) {
			do {
				if (pthis->m_pRecvBuffer->Read(cmd, 1) != 1)
					break;
				if (static_cast<BYTE>(cmd[0]) != 0xEB)
					break;
				if (pthis->m_pRecvBuffer->Read(cmd + 1, 1) != 1)
					break;

				switch (static_cast<BYTE>(cmd[1])) {
					case 0xAE:	// 主机到键盘，长度为 CMD_LEN_SERIAL_KEYBOARD
					{
						if (WaitForSingleObject(pthis->m_hEventExit, 1) == WAIT_OBJECT_0)
							break;
						int recv = pthis->m_pRecvBuffer->Read(cmd + 2, CMD_LEN_SERIAL_KEYBOARD - 2);
						while (recv != CMD_LEN_SERIAL_KEYBOARD - 2) {
							if (WaitForSingleObject(pthis->m_hEventExit, 1) == WAIT_OBJECT_0)
								break;
							recv += pthis->m_pRecvBuffer->Read(cmd + 2 + recv, CMD_LEN_SERIAL_KEYBOARD - 2 - recv);
						}
						pthis->DealCmdKeyboard(cmd, CMD_LEN_SERIAL_KEYBOARD);
					}
						break;
					case 0xB1:	// 主机到软件，长度为 CMD_LEN_SERIAL_COMPUTER
						if (pthis->m_pRecvBuffer->Read(cmd + 2, CMD_LEN_SERIAL_COMPUTER - 2) == CMD_LEN_SERIAL_COMPUTER - 2)
							pthis->DealCmdComputer(cmd, CMD_LEN_SERIAL_COMPUTER);
						break;
					case 0xFF:
						CLog::WriteLog(_T("Recv HeartBeatPkg"));
						break;
					default:
						break;
				}

				// 检查是否忙碌
				if (GetTickCount() - dwPrevTimePoint >= TIME_RANGE_FOR_CHECK_BUSY) {
					dwPrevTimePoint = GetTickCount();
					dwPackageNum = 0;
				} else {
					pthis->m_bBusy = (++dwPackageNum >= PACKAGE_NUM_FOR_CHECK_BUSY);
				}
			} while (0);

			// 超时恢复与处理
			/*dwLastTimeRecv = GetTickCount();
			if(bDisconnected)
			{
			pthis->OnConnectionEstablished();
			bDisconnected = FALSE;
			}*/
		}
	}
	return 0;
}

void CCmdHandler::SendDirect(const BYTE *cmd, int len)
{
	if (IsBadReadPtr(cmd, len)) {
		CLog::WriteLog(_T("CCmdHandler::SendDirect invalid ptr cmd %p ***************************************\n"), cmd);
		return;
	}
	m_pSendBuffer->Write((const char*)cmd, len);
}

DWORD WINAPI CCmdHandler::ThreadRetrieveAreaInfo(LPVOID lp)
{
	CLog::WriteLog(_T("CCmdHandler::ThreadRetrieveAreaInfo init\n"));
	//新主机
	CWnd *pWnd = AfxGetApp()->m_pMainWnd;
	CCmdHandler *pCmdHandler = (CCmdHandler*)lp;
	pCmdHandler->m_bRetrievingAreaInfo = TRUE;
	BYTE cmd[CMD_LEN_SERIAL_COMPUTER] = { 0 };
	cmd[0] = 0xEB;
	cmd[1] = 0xB5;
	cmd[2] = CMD_LEN_SERIAL_COMPUTER;
	cmd[3] = HIBYTE(0);
	cmd[4] = LOBYTE(0);
	cmd[5] = CT_RETREIVE;
	cmd[6] = AREA_SLAVE_EMERGENCY;
	cmd[7] = 0;

	for (WORD idArea = 0; idArea < AREA_PER_HOST; ++idArea) {
		cmd[3] = HIBYTE(idArea);
		cmd[4] = LOBYTE(idArea);
		cmd[7] = MakeSum((const char*)cmd, CMD_LEN_SERIAL_COMPUTER);
		pCmdHandler->m_idAreaRetrieving = idArea;
		DWORD dwStart = GetTickCount();
RESEND:
		if (GetTickCount() - dwStart > 3000) {
			//::MessageBox(CStaticVarable::m_hWndRetrieveProgress, _T("No serial connection!"),
			//			 _T("Error"), MB_OK | MB_ICONERROR);
			break;
		}
		if (WaitForSingleObject(pCmdHandler->m_hEventExit, 0) == WAIT_OBJECT_0)
			goto ThreadRetrieveAreaInfoExit;
		pCmdHandler->SendDirect(cmd, CMD_LEN_SERIAL_COMPUTER);
		if (WAIT_OBJECT_0 != WaitForSingleObject(pCmdHandler->m_hEventRetrieveAnAreaOk, 50))
			goto RESEND;
	}
	::PostMessage(CStaticVarable::m_hWndRetrieveProgress, WM_CLOSE, 0, 0);
	//Sleep(500);
	pWnd = AfxGetApp()->m_pMainWnd; if (!pWnd)	goto ThreadRetrieveAreaInfoExit;
	CLog::WriteLog(_T("CCmdHandler::ThreadRetrieveAreaInfo PostMessage WM_RETRIEVEOVER\n"));
	pWnd->PostMessage(WM_RETRIEVEOVER);
	pCmdHandler->m_bRetrievingAreaInfo = FALSE;
	CLOSEHANDLE(pCmdHandler->m_hThreadRetrievingSlaveInfo);
ThreadRetrieveAreaInfoExit:
	CLog::WriteLog(_T("CCmdHandler::ThreadRetrieveAreaInfo exit\n"));
	return 0;
}

void CCmdHandler::WaitForRetrieveResult(int idSlave)
{
	if (m_bRetrievingSlaveInfo)
		return;
	m_bRetrievingSlaveInfo = TRUE;
	m_idSlaveRetrieving = idSlave;
	if (m_hEventRetrievingSlaveInfo == INVALID_HANDLE_VALUE)
		m_hEventRetrievingSlaveInfo = CreateEvent(NULL, FALSE, FALSE, NULL);
	HANDLE hThread = CreateThread(NULL, 0, ThreadWaitForRetrieveSlaveInfoResult, this, 0, NULL);
	WaitTillThreadActive(hThread);
	CLOSEHANDLE(hThread);
	CWinApp *app = AfxGetApp();
	CWnd* pWnd = app->GetMainWnd();
	HWND hWnd = pWnd->m_hWnd;
	::PostMessage(hWnd, WM_RETRIEVESLAVEINFORESULT, 0, idSlave);
}

DWORD WINAPI CCmdHandler::ThreadWaitForRetrieveSlaveInfoResult(LPVOID lp)
{
	CCmdHandler *pThis = (CCmdHandler*)lp;
	DWORD now = GetTickCount();
	while (GetTickCount() - now < WAIT_TIME_FOR_RETRIEVE_SLAVE_INFO) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(pThis->m_hEventRetrievingSlaveInfo, 1000)) {
			// 成功
			::PostMessage(CStaticVarable::m_hWndRetrieveSlaveInfoResult, WM_RETRIEVESLAVEINFORESULT, 1, 0);
			goto EXIT_THREAD;
		} else {
			// 失败，但继续等待
			::PostMessage(CStaticVarable::m_hWndRetrieveSlaveInfoResult, WM_RETRIEVESLAVEINFORESULT, 0, 1);
		}
	}

	// 失败，退出
	::PostMessage(CStaticVarable::m_hWndRetrieveSlaveInfoResult, WM_RETRIEVESLAVEINFORESULT, 0, 0);
EXIT_THREAD:
	pThis->m_bRetrievingSlaveInfo = FALSE;
	CLOSEHANDLE(pThis->m_hEventRetrievingSlaveInfo);
	return 0;
}

BOOL CCmdHandler::CheckIsBusy() const
{
	return m_bBusy;
}

// CmdHandler.h: interface for the CCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CMDHANDLER_H__2D01303C_CB99_49B7_BB95_E09674DE0F56__INCLUDED_)
#define AFX_CMDHANDLER_H__2D01303C_CB99_49B7_BB95_E09674DE0F56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGenericBuffer;

#ifdef USE_TCP_NOT_SERIAL
#include "TcpServer.h"
class CCmdHandler : public CTcpServer
#else
#include "SerialPort.h"
class CCmdHandler : public CSerialPort
#endif
{
public:
	CCmdHandler();
	virtual ~CCmdHandler();
protected:
	virtual void OnConnectionEstablished();
	virtual BOOL OnSend(IN char* cmd, IN WORD wLen, OUT WORD& wRealLen);
	virtual BOOL OnRecv(const char *cmd, WORD wLen);
	BOOL DealCmdKeyboard(const char* cmd, WORD wLen);
	BOOL DealCmdComputer(const char* cmd, WORD wLen);
private:
	HANDLE m_hEventExit, m_hThreadDeal;
	CGenericBuffer *m_pRecvBuffer;
	BYTE m_ucKeyboardAddr;
	static DWORD WINAPI ThreadDeal(LPVOID lp);
};

#endif // !defined(AFX_CMDHANDLER_H__2D01303C_CB99_49B7_BB95_E09674DE0F56__INCLUDED_)

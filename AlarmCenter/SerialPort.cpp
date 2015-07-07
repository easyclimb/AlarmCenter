/*
**	FILENAME			CSerialPort.cpp
**
**	PURPOSE				This class can read, write and watch one serial port.
**						It sends messages to its owner when something happends on the port
**						The class creates a thread for reading and writing so the main
**						program is not blocked.
**
**	CREATION DATE		15-09-1997
**	LAST MODIFICATION	12-11-1997
**
**	AUTHOR				Remon Spekreijse
**
**
*/

#include "stdafx.h"
#include "SerialPort.h"

#include <assert.h>

//
// Constructor
//
CSerialPort::CSerialPort()
{
	m_pOwner = NULL;
	m_hComm = INVALID_HANDLE_VALUE;

	// initialize overlapped structure members to zero
	m_ov.Offset = 0;
	m_ov.OffsetHigh = 0;

	// create events
	m_ov.hEvent = INVALID_HANDLE_VALUE;
	m_hWriteEvent = INVALID_HANDLE_VALUE;
	m_hShutdownEvent = INVALID_HANDLE_VALUE;
	m_hThreadComm = INVALID_HANDLE_VALUE;
	m_hThreadSend = INVALID_HANDLE_VALUE;
	m_hEventSent = INVALID_HANDLE_VALUE;

	m_dataWriteBuffer = NULL;
	m_bFirstTimeRecv = TRUE;
	//m_bThreadAlive = FALSE;
}

//
// Delete dynamic memory
//
CSerialPort::~CSerialPort()
{
	AUTO_LOG_FUNCTION;
	if (m_hThreadComm != INVALID_HANDLE_VALUE) {
		SetEvent(m_hShutdownEvent);
		WaitForSingleObject(m_hThreadComm, INFINITE);
		WaitForSingleObject(m_hThreadSend, INFINITE);
		CLOSEHANDLE(m_hThreadComm);
		CLOSEHANDLE(m_hThreadSend);
	}
	CLOSEHANDLE(m_hEventSent);
	CLOSEHANDLE(m_hShutdownEvent);
	CLOSEHANDLE(m_hComm);
	CLOSEHANDLE(m_hWriteEvent);
	CLOSEHANDLE(m_ov.hEvent);
	DeleteCriticalSection(&m_csCommunicationSync);
	SAFEDELETEARR(m_dataWriteBuffer);
}

//
// Initialize the port. This can be port 1 to 4.
//
BOOL CSerialPort::InitPort(CWnd* pPortOwner,	// the owner (CWnd) of the port (receives message)
						   UINT  portnr,		// portnumber (1..4)
						   UINT  baud,			// baudrate
						   char  parity,		// parity 
						   UINT  databits,		// databits 
						   UINT  stopbits,		// stopbits 
						   DWORD dwCommEvents,	// EV_RXCHAR, EV_CTS etc
						   UINT  writebuffersize)	// size to the writebuffer
{
	assert(portnr > 0 && portnr < 5);
	//assert(pPortOwner != NULL);

	// if the thread is alive: Kill
	if (m_hThreadComm != INVALID_HANDLE_VALUE) {
		SetEvent(m_hShutdownEvent);
		WaitForSingleObject(m_hThreadComm, INFINITE);
		WaitForSingleObject(m_hThreadSend, INFINITE);
		CLOSEHANDLE(m_hThreadComm);
		CLOSEHANDLE(m_hThreadSend);
	}

	// create events
	if (m_ov.hEvent != INVALID_HANDLE_VALUE)
		ResetEvent(m_ov.hEvent);
	m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hWriteEvent != INVALID_HANDLE_VALUE)
		ResetEvent(m_hWriteEvent);
	m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hShutdownEvent != INVALID_HANDLE_VALUE)
		ResetEvent(m_hShutdownEvent);
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// initialize the event objects
	m_hEventArray[0] = m_hShutdownEvent;	// highest priority
	m_hEventArray[1] = m_ov.hEvent;
	m_hEventArray[2] = m_hWriteEvent;

	// initialize critical section
	InitializeCriticalSection(&m_csCommunicationSync);

	// set buffersize for writing and save the owner
	m_pOwner = pPortOwner;

	if (m_dataWriteBuffer != NULL)
		delete[] m_dataWriteBuffer;
	m_dataWriteBuffer = new char[writebuffersize];

	m_nPortNr = portnr;

	m_nWriteBufferSize = writebuffersize;
	m_dwCommEvents = dwCommEvents;

	TCHAR szPort[128] = { 0 };
	TCHAR szBaud[128] = { 0 };

	// now it critical!
	EnterCriticalSection(&m_csCommunicationSync);

	// if the port is already opened: close it
	if (m_hComm != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}

	// prepare port strings
	_stprintf_s(szPort, _T("COM%d"), portnr);
	_stprintf_s(szBaud, _T("baud=%d parity=%c data=%d stop=%d"),
				baud, parity, databits, stopbits);

	// get a handle to the port
	m_hComm = CreateFile(szPort,						// communication port string (COMX)
						 GENERIC_READ | GENERIC_WRITE,	// read/write types
						 0,								// comm devices must be opened with exclusive access
						 NULL,							// no security attributes
						 OPEN_EXISTING,					// comm devices must use OPEN_EXISTING
						 FILE_FLAG_OVERLAPPED,			// Async I/O
						 0);							// template must be 0 for comm devices

	if (m_hComm == INVALID_HANDLE_VALUE) {
		// port not found
		return FALSE;
	}

	// set the timeout values
	m_CommTimeouts.ReadIntervalTimeout = 1000;
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.ReadTotalTimeoutConstant = 1000;
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;

	// configure
	if (SetCommTimeouts(m_hComm, &m_CommTimeouts)) {
		if (SetCommMask(m_hComm, dwCommEvents)) {
			if (GetCommState(m_hComm, &m_dcb)) {
				m_dcb.fRtsControl = RTS_CONTROL_ENABLE;		// set RTS bit high!
				if (BuildCommDCB(szBaud, &m_dcb)) {
					if (SetCommState(m_hComm, &m_dcb))
						; // normal operation... continue
					else
						ProcessErrorMessage("SetCommState()");
				} else
					ProcessErrorMessage("BuildCommDCB()");
			} else
				ProcessErrorMessage("GetCommState()");
		} else
			ProcessErrorMessage("SetCommMask()");
	} else
		ProcessErrorMessage("SetCommTimeouts()");

	// flush the port
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	// release critical section
	LeaveCriticalSection(&m_csCommunicationSync);

	CLog::WriteLog(_T("Initialisation for communicationport %d completed.\nUse Startmonitor to communicate.\n"), portnr);

	return TRUE;
}

//
//  The CommThread Function.
//
DWORD WINAPI CSerialPort::CommThread(LPVOID pParam)
{
	CLog::WriteLog(_T("core dump CSerialPort::CommThread tid %d"), GetCurrentThreadId());
	// Cast the void pointer passed to the thread back to
	// a pointer of CSerialPort class
	CSerialPort *port = (CSerialPort*)pParam;

	// Set the status variable in the dialog class to
	// TRUE to indicate the thread is running.
	//port->m_bThreadAlive = TRUE;	

	// Misc. variables
	DWORD Event = 0;
	DWORD CommEvent = 0;
	DWORD dwError = 0;
	static COMSTAT comstat;
	BOOL  bResult = TRUE;

	// Clear comm buffers at startup
	if (port->m_hComm)		// check if the port is opened
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	// begin forever loop.  This loop will run as long as the thread is alive.
	for (;;) {

		// Make a call to WaitCommEvent().  This call will return immediatly
		// because our port was created as an async port (FILE_FLAG_OVERLAPPED
		// and an m_OverlappedStructerlapped structure specified).  This call will cause the 
		// m_OverlappedStructerlapped element m_OverlappedStruct.hEvent, which is part of the m_hEventArray to 
		// be placed in a non-signeled state if there are no bytes available to be read,
		// or to a signeled state if there are bytes available.  If this event handle 
		// is set to the non-signeled state, it will be set to signeled when a 
		// character arrives at the port.

		// we do this for each port!

		bResult = WaitCommEvent(port->m_hComm, &Event, &port->m_ov);

		if (!bResult) {
			// If WaitCommEvent() returns FALSE, process the last error to determin
			// the reason..
			switch (dwError = GetLastError()) {
				case ERROR_IO_PENDING:
					// This is a normal return value if there are no bytes
					// to read at the port.
					// Do nothing and continue
					break;
				case 87:
					// Under Windows NT, this value is returned for some reason.
					// I have not investigated why, but it is also a valid reply
					// Also do nothing and continue.
					break;
				default:
					// All other error codes indicate a serious error has
					// occured.  Process this error.
					port->ProcessErrorMessage("WaitCommEvent()");
					break;
			}
		} else {
			// If WaitCommEvent() returns TRUE, check to be sure there are
			// actually bytes in the buffer to read.  
			//
			// If you are reading more than one byte at a time from the buffer 
			// (which this program does not do) you will have the situation occur 
			// where the first byte to arrive will cause the WaitForMultipleObjects() 
			// function to stop waiting.  The WaitForMultipleObjects() function 
			// resets the event handle in m_OverlappedStruct.hEvent to the non-signelead state
			// as it returns.  
			//
			// If in the time between the reset of this event and the call to 
			// ReadFile() more bytes arrive, the m_OverlappedStruct.hEvent handle will be set again
			// to the signeled state. When the call to ReadFile() occurs, it will 
			// read all of the bytes from the buffer, and the program will
			// loop back around to WaitCommEvent().
			// 
			// At this point you will be in the situation where m_OverlappedStruct.hEvent is set,
			// but there are no bytes available to read.  If you proceed and call
			// ReadFile(), it will return immediatly due to the async port setup, but
			// GetOverlappedResults() will not return until the next character arrives.
			//
			// It is not desirable for the GetOverlappedResults() function to be in 
			// this state.  The thread shutdown event (event 0) and the WriteFile()
			// event (Event2) will not work if the thread is blocked by GetOverlappedResults().
			//
			// The solution to this is to check the buffer with a call to ClearCommError().
			// This call will reset the event handle, and if there are no bytes to read
			// we can loop back through WaitCommEvent() again, then proceed.
			// If there are really bytes to read, do nothing and proceed.

			bResult = ClearCommError(port->m_hComm, &dwError, &comstat);

			if (comstat.cbInQue == 0)
				continue;
		}	// end if bResult

		// Main wait function.  This function will normally block the thread
		// until one of nine events occur that require action.
		Event = WaitForMultipleObjects(3, port->m_hEventArray, FALSE, INFINITE);

		switch (Event) {
			case 0:
				// Shutdown event.  This is event zero so it will be
				// the higest priority and be serviced first.

				//port->m_bThreadAlive = FALSE;

				// Kill this thread.  break is not needed, but makes me feel better.
				//AfxEndThread(100);
				//AfxMessageBox("ExitThread");
				ExitThread(0);
				break;
			case 1:	// read event
				GetCommMask(port->m_hComm, &CommEvent);
				if (CommEvent & EV_CTS && port->m_pOwner)
					::SendMessage(port->m_pOwner->m_hWnd, WM_COMM_CTS_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
				if (CommEvent & EV_RXFLAG && port->m_pOwner)
					::SendMessage(port->m_pOwner->m_hWnd, WM_COMM_RXFLAG_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
				if (CommEvent & EV_BREAK && port->m_pOwner)
					::SendMessage(port->m_pOwner->m_hWnd, WM_COMM_BREAK_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
				if (CommEvent & EV_ERR && port->m_pOwner)
					::SendMessage(port->m_pOwner->m_hWnd, WM_COMM_ERR_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
				if (CommEvent & EV_RING && port->m_pOwner)
					::SendMessage(port->m_pOwner->m_hWnd, WM_COMM_RING_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);

				if (CommEvent & EV_RXCHAR) {
					// Receive character event from port.
					ReceiveChar(port, comstat);
					/*if(port->m_bFirstTimeRecv)
					{
					port->m_bFirstTimeRecv = FALSE;
					port->OnConnectionEstablished();
					}*/
				}
				break;
			case 2: // write event
				// Write character event from port
				WriteChar(port);
				break;

		} // end switch

	} // close forever loop

}

//
// start comm watching
//
BOOL CSerialPort::StartMonitoring()
{
	m_hThreadComm = CreateThread(NULL, 0, CommThread, this, 0, NULL);
	m_hEventSent = CreateEvent(NULL, TRUE, TRUE, NULL);
	m_hThreadSend = CreateThread(NULL, 0, ThreadSend, this, 0, NULL);
	OnConnectionEstablished();
	return TRUE;
}

//
// Restart the comm thread
//
BOOL CSerialPort::RestartMonitoring()
{
	ResumeThread(m_hThreadComm);
	return TRUE;
}


//
// Suspend the comm thread
//
BOOL CSerialPort::StopMonitoring()
{
	SuspendThread(m_hThreadComm);
	return TRUE;
}


//
// If there is a error, give the right message
//
void CSerialPort::ProcessErrorMessage(char* ErrorText)
{
	TCHAR Temp[260] = { 0 };
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
		);
#if defined(_UNICODE) || defined(UNICODE)
	wchar_t *wt = AnsiToUtf16(ErrorText);
	_stprintf_s(Temp, _T("WARNING:  %s Failed with the following error: \n%s\nPort: %d\n"),
				wt, lpMsgBuf, m_nPortNr);
	MessageBox(NULL, Temp, _T("Application Error"), MB_ICONSTOP);
#else
	_stprintf_s(Temp, _T("WARNING:  %s Failed with the following error: \n%s\nPort: %d\n"),
				ErrorText, lpMsgBuf, m_nPortNr);
	MessageBox(NULL, Temp, _T("Application Error"), MB_ICONSTOP);
#endif
	LocalFree(lpMsgBuf);
}

//
// Write a character.
//
void CSerialPort::WriteChar(CSerialPort* port)
{
	BOOL bWrite = TRUE;
	BOOL bResult = TRUE;
	DWORD BytesSent = 0;

	ResetEvent(port->m_hWriteEvent);

	EnterCriticalSection(&port->m_csCommunicationSync);

	if (bWrite) {
		port->m_ov.Offset = 0;
		port->m_ov.OffsetHigh = 0;

		// Clear buffer
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

		bResult = WriteFile(port->m_hComm,				// Handle to COMM Port
							port->m_dataWriteBuffer,	// Pointer to message buffer in calling finction
							port->m_dataLen,			// Length of message to send
							&BytesSent,					// Where to store the number of bytes sent
							&port->m_ov);				// Overlapped structure

		// deal with any error codes
		if (!bResult) {
			DWORD dwError = GetLastError();
			switch (dwError) {
				case ERROR_IO_PENDING:
					// continue to GetOverlappedResults()
					BytesSent = 0;
					bWrite = FALSE;
					break;
				default:
					// all other error codes
					port->ProcessErrorMessage("WriteFile()");
			}
		} else {
			LeaveCriticalSection(&port->m_csCommunicationSync);
		}
	} // end if(bWrite)

	if (!bWrite) {
		bWrite = TRUE;

		bResult = GetOverlappedResult(port->m_hComm,	// Handle to COMM port 
									  &port->m_ov,		// Overlapped structure
									  &BytesSent,		// Stores number of bytes sent
									  TRUE); 			// Wait flag

		LeaveCriticalSection(&port->m_csCommunicationSync);

		// deal with the error code 
		if (!bResult) {
			port->ProcessErrorMessage("GetOverlappedResults() in WriteFile()");
		}
	} // end if (!bWrite)

	// Verify that the data size send equals what we tried to send
	if (BytesSent != port->m_dataLen) {
		//CString err = _T("");
		//err.Format(_T("WARNING: WriteFile() error.. Bytes Sent: %d; Message Length: %d\n"), BytesSent, port->m_dataLen);
		//AfxMessageBox(err);
		SetEvent(port->m_hEventSent);
	} else {
		SetEvent(port->m_hEventSent);
		/*
		CString t = _T("");
		CString s = _T(" ");
		CString c = _T("");
		for(UINT i = 0; i < BytesSent; i++)
		{
		c.Format(_T("%02x "), static_cast<BYTE>(port->m_dataWriteBuffer[i]));
		s += c;
		}
		t.Format(_T("WriteFile() success.. Bytes Sent: %d; Message Length: %d msg:%s\n"), BytesSent, port->m_dataLen, s);
		CLog::WriteLog(t);
		*/
	}
}

//
// Character received. Inform the owner
//
void CSerialPort::ReceiveChar(CSerialPort* port, COMSTAT comstat)
{
	BOOL  bRead = TRUE;
	BOOL  bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesRead = 0;
	//unsigned char RXBuff;
	char data[128];

	for (;;) {
		// Gain ownership of the comm port critical section.
		// This process guarantees no other part of this program 
		// is using the port object. 

		EnterCriticalSection(&port->m_csCommunicationSync);

		// ClearCommError() will update the COMSTAT structure and
		// clear any other errors.

		bResult = ClearCommError(port->m_hComm, &dwError, &comstat);

		LeaveCriticalSection(&port->m_csCommunicationSync);

		// start forever loop.  I use this type of loop because I
		// do not know at runtime how many loops this will have to
		// run. My solution is to start a forever loop and to
		// break out of it when I have processed all of the
		// data available.  Be careful with this approach and
		// be sure your loop will exit.
		// My reasons for this are not as clear in this sample 
		// as it is in my production code, but I have found this 
		// solutiion to be the most efficient way to do this.

		if (comstat.cbInQue == 0) {
			// break out when all bytes have been read
			break;
		}

		EnterCriticalSection(&port->m_csCommunicationSync);

		if (bRead) {
			memset(data, 0, sizeof(data));
			bResult = ReadFile(port->m_hComm,		// Handle to COMM port 
							   data,				// RX Buffer Pointer
							   comstat.cbInQue,		// Read one byte
							   &BytesRead,			// Stores number of bytes read
							   &port->m_ov);		// pointer to the m_ov structure
			// deal with the error code 
			if (!bResult) {
				switch (dwError = GetLastError()) {
					case ERROR_IO_PENDING:
						// asynchronous i/o is still in progress 
						// Proceed on to GetOverlappedResults();
						bRead = FALSE;
						break;
					default:
						// Another error has occured.  Process this error.
						port->ProcessErrorMessage("ReadFile()");
						break;
				}
			} else {
				// ReadFile() returned complete. It is not necessary to call GetOverlappedResults()
				bRead = TRUE;
			}
		}  // close if (bRead)

		if (!bRead) {
			bRead = TRUE;
			bResult = GetOverlappedResult(port->m_hComm,	// Handle to COMM port 
										  &port->m_ov,		// Overlapped structure
										  &BytesRead,		// Stores number of bytes read
										  TRUE); 			// Wait flag

			// deal with the error code 
			if (!bResult) {
				port->ProcessErrorMessage("GetOverlappedResults() in ReadFile()");
			}
		}  // close if (!bRead)
		
		LeaveCriticalSection(&port->m_csCommunicationSync);

		// notify parent that a byte was received
		LOGB(data, BytesRead);
		port->OnRecv(static_cast<const char*>(data), static_cast<WORD>(BytesRead));
		//::SendMessage((port->m_pOwner)->m_hWnd, WM_COMM_RXCHAR, (WPARAM) RXBuff, (LPARAM) port->m_nPortNr);
	} // end forever loop

}

//
// Write a string to the port
//
void CSerialPort::WriteToPort(const char* data, int nLen)
{
	assert(m_hComm != INVALID_HANDLE_VALUE);

	memset(m_dataWriteBuffer, 0, sizeof(m_dataWriteBuffer));
	//_tcscpy_s(m_dataWriteBuffer, string);
	memcpy(m_dataWriteBuffer, data, nLen);
	m_dataLen = nLen;
	// set event for write
	SetEvent(m_hWriteEvent);
}

//
// Return the device control block
//
DCB CSerialPort::GetDCB()
{
	return m_dcb;
}

//
// Return the communication event masks
//
DWORD CSerialPort::GetCommEvents()
{
	return m_dwCommEvents;
}

//
// Return the output buffer size
//
DWORD CSerialPort::GetWriteBufferSize()
{
	return m_nWriteBufferSize;
}

void CSerialPort::ClosePort()
{
	if (m_hThreadComm != INVALID_HANDLE_VALUE) {
		SetEvent(m_hShutdownEvent);
		WaitForSingleObject(m_hThreadComm, INFINITE);
		WaitForSingleObject(m_hThreadSend, INFINITE);
		CLOSEHANDLE(m_hThreadComm);
		CLOSEHANDLE(m_hThreadSend);
	}


	if (m_hComm != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}

	if (m_hShutdownEvent != INVALID_HANDLE_VALUE)
		ResetEvent(m_hShutdownEvent);

	if (m_ov.hEvent != INVALID_HANDLE_VALUE)
		ResetEvent(m_ov.hEvent);

	if (m_hWriteEvent != INVALID_HANDLE_VALUE)
		ResetEvent(m_hWriteEvent);
}

DWORD WINAPI CSerialPort::ThreadSend(LPVOID lp)
{
	AUTO_LOG_FUNCTION;
	CSerialPort *port = static_cast<CSerialPort*>(lp);
	while (1) {
		DWORD dwRet = WaitForSingleObject(port->m_hShutdownEvent, 50);
		if (dwRet == WAIT_OBJECT_0)
			break;
		dwRet = WaitForSingleObject(port->m_hEventSent, 0);
		if (dwRet == WAIT_OBJECT_0) {
			char sendBuff[128] = { 0 };
			WORD wCmdLen = 0;
			if (port->OnSend(sendBuff, sizeof(sendBuff), wCmdLen) && wCmdLen > 0) {
				ResetEvent(port->m_hEventSent);
				port->WriteToPort(sendBuff, wCmdLen);
			}
		}
	}
	return 0;
}

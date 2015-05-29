// AntLine.cpp: implementation of the CAntLine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AntLine.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

namespace gui {
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static const int	WAITTIME = 500;
//static const char* EVENTEXIT = "EventExit";
//static const char* EVENTREBUILD = "EventRebuild";
//static int cCounter = 0;

CAntLine::CAntLine()
	: m_hDC(NULL)
	, m_hThread(INVALID_HANDLE_VALUE)
	, m_hEventExit(INVALID_HANDLE_VALUE)
	//m_hEventRebuild(INVALID_HANDLE_VALUE),
	, m_bShowing(FALSE)//, m_nIndex(0)
{
	//CLog::WriteLog("CAntLine %s\n", m_name);
	m_hEventExit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	//m_hEventRebuild = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	InitializeCriticalSection(&m_cs);
}

CAntLine::~CAntLine()
{
	AUTO_LOG_FUNCTION;
	StopThread();
	CLOSEHANDLE(m_hEventExit);
	//CLOSEHANDLE(m_hEventRebuild);
	DeleteAllLine();
	DeleteCriticalSection(&m_cs);
}

void CAntLine::ShowAntLine(HDC hDC, BOOL bShow)
{
	CLocalLock lock(&m_cs);
	CLog::WriteLog(_T("ShowAntLine hDC %p bShow %d"), hDC, bShow);
	if (m_LineList.size() == 0)
		return;
	m_bShowing = bShow;
	if (m_bShowing) {
		//::SuspendThread(m_hThread);
		m_hDC = hDC;
		//::SetEvent(m_hEventRebuild);
		//DWORD dw = 0;
		//while((dw = ::ResumeThread(m_hThread)) > 0){}
		StartThread();
	} else {
		//::DeleteDC(hDC);
		//m_hDC = NULL;
		//::SuspendThread(m_hThread);
		StopThread();
	}
	CLog::WriteLog(_T("ShowAntLine hDC %p bShow %d okCAntLine::AddLine"), hDC, bShow);
}

void CAntLine::AddLine(int x1, int y1, int x2, int y2, DWORD data)
{
	CLog::WriteLog(_T("CAntLine::AddLine(int x1 %d, int y1 %d, int x2 %d, int y2 %d, DWORD data 0x%x)\n"),
		  x1, y1, x2, y2, data);
	CLocalLock lock(&m_cs);
	StopThread();

	PLINE pLine = new LINE(x1, y1, x2, y2, data);
	m_LineList.push_back(pLine);
	if (m_bShowing) {
		//::SetEvent(m_hEventRebuild);
		//DWORD dw = 0;
		//while((dw = ::ResumeThread(m_hThread)) > 0){}
		//m_hThread = ::CreateThread(NULL, 0, ThreadShow, this, CREATE_SUSPENDED, NULL);
		StartThread();
	}
}

void CAntLine::AddLine(const CPoint& pt1, const CPoint& pt2, DWORD data)
{
	AddLine(pt1.x, pt1.y, pt2.x, pt2.y, data);
}

void CAntLine::SetPixelExEx(HDC hDC, int x, int y, COLORREF clr)
{
	/*
	CPoint pt(x, y);
	CPoint pts[8];
	for(int i = 0; i < 8; i++)
	pts[i] = pt;

	pts[0].x = pt.x-1;
	pts[0].y = pt.y-1;

	pts[1].x = pt.x;
	pts[1].y = pt.y-1;

	pts[2].x = pt.x+1;
	pts[2].y = pt.y-1;

	pts[3].x = pt.x-1;
	pts[3].y = pt.y;

	pts[4].x = pt.x+1;
	pts[4].y = pt.y;

	pts[5].x = pt.x-1;
	pts[5].y = pt.y+1;

	pts[6].x = pt.x;
	pts[6].y = pt.y+1;

	pts[7].x = pt.x+1;
	pts[7].y = pt.y+1;
	*/
	//pDC->SetPixel(pt, clr);
	SetPixelV(hDC, x, y, clr);
	//for(i = 0; i < 8; i++)
	//SetPixel(pts[i], clr);
	//	SetPixelV(hDC, pts[i].x, pts[i].y, clr);
}

void CAntLine::DrawAntLine(HDC hDC, int &cnt, int x1, int y1, int x2, int y2)
{
	int xDis = x2 - x1;
	int yDis = y2 - y1;
	double maxStep = max(abs(xDis), abs(yDis));
	double xUnitLen = xDis / maxStep;
	double yUnitLen = yDis / maxStep;
	double x = x1;
	double y = y1;

	for (long i = 1; i < maxStep; i++) {
		x += xUnitLen;	y += yUnitLen;

		cnt = ++cnt % 8;
		if (cnt >= 8)
			cnt = 0;

		if ((cnt)>2)
			SetPixelExEx(hDC, static_cast<int>(x), static_cast<int>(y), RGB(255, 0, 0));
		else
			SetPixelExEx(hDC, static_cast<int>(x), static_cast<int>(y), RGB(255, 200, 200));
		//::SetPixelExEx(hDC, static_cast<int>(x), static_cast<int>(y), RGB(255,150,150));
	}

}

VOID CALLBACK CAntLine::LineDDAProc(int X, int Y, LPARAM lpData)
{
	CAntLine *pAL = (CAntLine*)lpData;
	//pAL->m_CurLine._cnt = ++pAL->m_CurLine._cnt % 8;
	//if(pAL->m_CurLine._cnt >= 8)
	//	pAL->m_CurLine._cnt = 0;

	if ((pAL->m_CurLine._cnt) == 0 || (pAL->m_CurLine._cnt > 0x000000f0))
		pAL->m_CurLine._cnt = 1;
	pAL->m_CurLine._cnt = pAL->m_CurLine._cnt << 1;
	if ((pAL->m_CurLine._cnt & 0x000000f0) > 0) {
		SetPixel(pAL->m_hDC, X, Y, RGB(255, 0, 0));
	} else {
		SetPixel(pAL->m_hDC, X, Y, RGB(255, 200, 200));
	}
}
DWORD WINAPI CAntLine::ThreadShow(LPVOID lp)
{
	AUTO_LOG_FUNCTION;
	CAntLine *pAL = (CAntLine*)lp;
	static const COLORREF cClrRed = RGB(255, 0, 0);
	static const COLORREF cClrGap = RGB(255, 200, 200);
	srand(static_cast<unsigned int>(time(NULL)));
	while (1) {
		if (WAIT_OBJECT_0 == ::WaitForSingleObject(pAL->m_hEventExit, WAITTIME)) {
			::ResetEvent(pAL->m_hEventExit);	break;
		}

		//POSITION pos = pAL->m_LineList.GetHeadPosition();
		std::list<PLINE>::iterator iter = pAL->m_LineList.begin();
		int random = (int)(rand() % 2);
		while (iter != pAL->m_LineList.end()) {
			if (WAIT_OBJECT_0 == ::WaitForSingleObject(pAL->m_hEventExit, 0)) {
				break;
			}
			//if(WAIT_OBJECT_0 == ::WaitForSingleObject(pAL->m_hEventRebuild, 0))
			//{	::ResetEvent(pAL->m_hEventRebuild);	break;	}

			PLINE pLine = *iter++;
			int xDis = pLine->_x2 - pLine->_x1;
			int yDis = pLine->_y2 - pLine->_y1;
			int maxStep = max(abs(xDis), abs(yDis));
			if (maxStep == 0)
				break;
			double xUnitLen = (double)xDis / (double)maxStep;
			double yUnitLen = (double)yDis / (double)maxStep;
			double x = pLine->_x2;	double y = pLine->_y2;

			int sMod = 0;
			while (sMod < 11) sMod = rand() % 14;
			//while(((int)maxStep % sMod) == 0)	sMod ++;
			int mod_half = sMod / 2;

			static const int scMagic = 1;
			for (long i = 1; i < maxStep; i += scMagic) {
				x -= xUnitLen * scMagic;	y -= yUnitLen * scMagic;
				pLine->_cnt = ++pLine->_cnt % sMod;

				if ((pLine->_cnt) > mod_half)
					SetPixel(pAL->m_hDC, static_cast<int>(x), static_cast<int>(y), cClrRed);
				else
					SetPixel(pAL->m_hDC, static_cast<int>(x), static_cast<int>(y), cClrGap);
			}
			//pLine->_cnt = rand() % sMod;
			pLine->_cnt += random;
		}
	}

	return 0;
}

void CAntLine::DeleteLine(DWORD dwData)
{
	CLocalLock lock(&m_cs);
	if (m_LineList.size() == 0) {
		//PAREA_ID p = reinterpret_cast<PAREA_ID>(dwData);
		//SAFEDELETEP(p);
		return;
	}
	StopThread();

	std::list<PLINE>::iterator iter = m_LineList.begin();
	while (iter != m_LineList.end()) {
		PLINE pLine = *iter;
		if (pLine->_data == dwData) {
			SAFEDELETEP(pLine);
			m_LineList.erase(iter);
			iter = m_LineList.begin();
			continue;
		}
		iter++;
	}

	if (m_LineList.size() == 0) {
		SetEvent(m_hEventExit);
		WaitTillThreadExited(m_hThread);
	}
	//::ResetEvent(m_hEventRebuild);
	if (m_bShowing) {
		//::SetEvent(m_hEventRebuild);
		//DWORD dw = 0;
		//while((dw = ::ResumeThread(m_hThread)) > 0){}
		StartThread();
	}

	//PAREA_ID p = reinterpret_cast<PAREA_ID>(dwData);
	//SAFEDELETEP(p);
}

void CAntLine::DeleteAllLine()
{
	CLocalLock lock(&m_cs);
	StopThread();

	std::list<PLINE>::iterator iter = m_LineList.begin();
	while (iter != m_LineList.end()) {
		PLINE pLine = *iter++;
		//PAREA_ID p = reinterpret_cast<PAREA_ID>(pLine->_data);
		//SAFEDELETEP(p);
		SAFEDELETEP(pLine);
		//m_LineList.GetNext(pos);
	}
	m_LineList.clear();
}

void CAntLine::StartThread()
{
	if (m_hThread == INVALID_HANDLE_VALUE) {
		ResetEvent(m_hEventExit);
		m_hThread = ::CreateThread(NULL, 0, ThreadShow, this, 0, NULL);
	}
}

void CAntLine::StopThread()
{
	if (m_hThread != INVALID_HANDLE_VALUE) {
		SetEvent(m_hEventExit);
		WaitForSingleObject(m_hThread, INFINITE);
		CLOSEHANDLE(m_hThread);
	}
}

NAMESPACE_END

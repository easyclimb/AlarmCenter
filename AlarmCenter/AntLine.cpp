// AntLine.cpp: implementation of the CAntLine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AntLine.h"
#include <math.h>
#include "C:/dev/Global/win32/MtVerify.h"

namespace gui {

static const int	WAITTIME = 500;

CAntLine::CAntLine()
	: m_hDC(nullptr)
	, m_bShowing(FALSE)//, m_nIndex(0)
{
}

CAntLine::~CAntLine()
{
	AUTO_LOG_FUNCTION;
	StopThread();
	DeleteAllLine();
}

void CAntLine::ShowAntLine(HDC hDC, BOOL bShow)
{
	std::lock_guard<std::mutex> lock(m_cs);
	JLOG(_T("ShowAntLine hDC %p bShow %d"), hDC, bShow);
	if (m_LineList.size() == 0)
		return;
	m_bShowing = bShow;
	if (m_bShowing) {
		m_hDC = hDC;
		StartThread();
	} else {
		StopThread();
	}
	JLOG(_T("ShowAntLine hDC %p bShow %d okCAntLine::AddLine"), hDC, bShow);
}

void CAntLine::AddLine(int x1, int y1, int x2, int y2, core::CDetectorWeakPtr data)
{
	JLOG(_T("CAntLine::AddLine(int x1 %d, int y1 %d, int x2 %d, int y2 %d, DWORD data 0x%x)\n"),
		  x1, y1, x2, y2, data);
	std::lock_guard<std::mutex> lock(m_cs);
	StopThread();

	AntLinePtr pLine = std::make_shared<AntLine>(x1, y1, x2, y2, data);
	m_LineList.push_back(pLine);
	if (m_bShowing) {
		StartThread();
	}
}

void CAntLine::AddLine(const CPoint& pt1, const CPoint& pt2, core::CDetectorWeakPtr data)
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

		if ((cnt) > 1) {
			SetPixelExEx(hDC, static_cast<int>(x), static_cast<int>(y), RGB(255, 0, 0));
		} else {
			SetPixelExEx(hDC, static_cast<int>(x), static_cast<int>(y), RGB(255, 200, 200));
		}
		
		//::SetPixelExEx(hDC, static_cast<int>(x), static_cast<int>(y), RGB(255,150,150));
	}

}

VOID CALLBACK CAntLine::LineDDAProc(int X, int Y, LPARAM lpData)
{
	CAntLine *pAL = (CAntLine*)lpData;
	//pAL->m_CurLine._cnt = ++pAL->m_CurLine._cnt % 8;
	//if(pAL->m_CurLine._cnt >= 8)
	//	pAL->m_CurLine._cnt = 0;

	if ((pAL->m_CurLine->_cnt) == 0 || (pAL->m_CurLine->_cnt > 0x000000f0))
		pAL->m_CurLine->_cnt = 1;
	pAL->m_CurLine->_cnt = pAL->m_CurLine->_cnt << 1;
	if ((pAL->m_CurLine->_cnt & 0x000000f0) > 0) {
		SetPixel(pAL->m_hDC, X, Y, RGB(255, 0, 0));
	} else {
		SetPixel(pAL->m_hDC, X, Y, RGB(255, 200, 200));
	}
}

void CAntLine::ThreadShow()
{
	AUTO_LOG_FUNCTION;

	static const COLORREF cClrRed = RGB(255, 0, 0);
	static const COLORREF cClrGap = RGB(255, 200, 200);
	srand(static_cast<unsigned int>(time(nullptr)));
	
	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(WAITTIME));
		if (!running_) {
			break;
		}

		auto iter = m_LineList.begin();
		int random = (int)(rand() % 2);
		while (iter != m_LineList.end()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
			if (!running_) {
				break;
			}

			AntLinePtr pLine = *iter++;
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
			int mod_half = sMod / 4;

			static const int scMagic = 1;
			for (long i = 1; i < maxStep; i += scMagic) {
				x -= xUnitLen * scMagic;	y -= yUnitLen * scMagic;
				pLine->_cnt = ++pLine->_cnt % sMod;

				if ((pLine->_cnt) > mod_half)
					SetPixel(m_hDC, static_cast<int>(x), static_cast<int>(y), cClrRed);
				else
					SetPixel(m_hDC, static_cast<int>(x), static_cast<int>(y), cClrGap);
			}

			pLine->_cnt += random;
		}
	}
}

void CAntLine::DeleteLine(const core::CDetectorPtr& data)
{
	std::lock_guard<std::mutex> lock(m_cs);
	if (m_LineList.size() == 0) {
		return;
	}
	StopThread();

	auto iter = m_LineList.begin();
	while (iter != m_LineList.end()) {
		AntLinePtr pLine = *iter;
		if (!pLine->_data.expired() && pLine->_data.lock() == data) {
			m_LineList.erase(iter);
			iter = m_LineList.begin();
			continue;
		}
		iter++;
	}

	if (m_bShowing) {
		StartThread();
	}
}

void CAntLine::DeleteAllLine()
{
	std::lock_guard<std::mutex> lock(m_cs);
	StopThread();

	m_LineList.clear();
	m_CurLine = nullptr;
}

void CAntLine::StartThread()
{
	if (!running_) {
		running_ = true;
		thread_ = std::thread(&CAntLine::ThreadShow, this);
	}
}

void CAntLine::StopThread()
{
	if (running_) {
		running_ = false;
		thread_.join();
	}
}

};

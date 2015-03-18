// AntLine.h: interface for the CAntLine class.
//
//////////////////////////////////////////////////////////////////////


#pragma once

//static const int NumLines = 300;
#include <list>

namespace gui {
class CAntLine  
{
protected:
	typedef struct _LINE
	{
		int _cnt;
		int _x1;
		int _y1;
		int _x2;
		int _y2;
		DWORD _data;
		_LINE() 
			: _cnt(1), _x1(0), _y1(0), _x2(0), _y2(0), _data(0)
		{}
		_LINE(int x1, int y1, int x2, int y2, DWORD data)
			: _cnt(1), _x1(x1), _y1(y1), _x2(x2), _y2(y2), _data(data)
		{}
		_LINE& operator = (const _LINE& rhs)
		{
			_cnt = rhs._cnt;
			_x1 = rhs._x1;
			_y1 = rhs._y1;
			_x2 = rhs._x2;
			_y2 = rhs._y2;
			_data = rhs._data;
			return *this;
		}
	}LINE, *PLINE;

	//typedef CList<LINE*, LINE*&> LINELIST;
public:
	void DeleteAllLine();
	void DeleteLine(DWORD dwData);
	void AddLine(const CPoint& pt1, const CPoint& pt2, DWORD data);
	void AddLine(int x1, int y1, int x2, int y2, DWORD data);
	void ShowAntLine(HDC hDC, BOOL bShow = TRUE);
	CAntLine();
	virtual ~CAntLine();
protected:
	void StopThread();
	void StartThread();
	static __forceinline VOID CALLBACK LineDDAProc(int X, int Y, LPARAM lpData);
	static void DrawAntLine(HDC hDC, int &cnt, int x1, int y1, int x2, int y2);
	__forceinline static void SetPixelExEx(HDC hDC, int x, int y, COLORREF clr);
	static DWORD WINAPI ThreadShow(LPVOID lp);
private:
	HDC m_hDC;
	HANDLE m_hThread;
	HANDLE m_hEventExit;//, m_hEventRebuild;
	//LINE m_lines[NumLines];
	//LINELIST m_LineList;
	std::list<PLINE> m_LineList;
	BOOL m_bShowing;
	//int m_nIndex;
	//CString m_name;
	LINE m_CurLine;
	CRITICAL_SECTION m_cs;
};

NAMESPACE_END

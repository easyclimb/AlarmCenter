// BmpEx.h: interface for the CBmpEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BMPEX_H__01B73DDA_2FD8_48CF_A772_180591002A4F__INCLUDED_)
#define AFX_BMPEX_H__01B73DDA_2FD8_48CF_A772_180591002A4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace gui
{
	namespace control
	{
		class CBmpEx
		{
		public:
			CBmpEx();
			virtual ~CBmpEx();
			static HBITMAP CBmpEx::GetHBitmapThumbnail(const CString& bmpPath, int width, int height);
		};
	};
};

#endif // !defined(AFX_BMPEX_H__01B73DDA_2FD8_48CF_A772_180591002A4F__INCLUDED_)

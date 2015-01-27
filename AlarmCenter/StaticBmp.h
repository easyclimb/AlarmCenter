#if !defined(AFX_STATICBMP_H__427E7DDA_3A82_47C9_A6A0_4BE79C2350C8__INCLUDED_)
#define AFX_STATICBMP_H__427E7DDA_3A82_47C9_A6A0_4BE79C2350C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StaticBmp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStaticBmp window

namespace gui
{
	namespace control
	{
		class CStaticBmp : public CStatic
		{
			// Construction
		public:
			CStaticBmp();

			// Attributes
		public:

			// Operations
		public:

			// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CStaticBmp)
			//}}AFX_VIRTUAL

			// Implementation
		public:
			void ShowBmp(const CString& bmpPath);
			virtual ~CStaticBmp();

			// Generated message map functions
		protected:
			//{{AFX_MSG(CStaticBmp)
			afx_msg void OnPaint();
			afx_msg void OnDestroy();
			//}}AFX_MSG

			DECLARE_MESSAGE_MAP()
		private:
			HBITMAP	m_hBitmap;
			CString m_bmpPath;
		};

	};
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATICBMP_H__427E7DDA_3A82_47C9_A6A0_4BE79C2350C8__INCLUDED_)

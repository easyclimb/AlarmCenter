#if !defined(AFX_STATICEX_H__079AD9B7_5A72_4B1D_9DE7_891CE95B1996__INCLUDED_)
#define AFX_STATICEX_H__079AD9B7_5A72_4B1D_9DE7_891CE95B1996__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StaticEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStaticEx window

namespace gui
{
	namespace control
	{
		class CStaticEx : public CStatic
		{
			// Construction
		public:
			CStaticEx();

			// Attributes
		public:

			// Operations
		public:
			void SetTooltipText(LPCTSTR lpszText, BOOL bActivate);
			void SetTooltipText(int nText, BOOL bActivate);
			// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CStaticEx)
		public:
			virtual BOOL PreTranslateMessage(MSG* pMsg);
		protected:
			virtual void PreSubclassWindow();
			//}}AFX_VIRTUAL

			// Implementation
		public:
			virtual ~CStaticEx();

			// Generated message map functions
		protected:
			void InitToolTip();
			//{{AFX_MSG(CStaticEx)
			//}}AFX_MSG

			DECLARE_MESSAGE_MAP()
		private:
			CToolTipCtrl m_ToolTip;
		};

	};
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATICEX_H__079AD9B7_5A72_4B1D_9DE7_891CE95B1996__INCLUDED_)

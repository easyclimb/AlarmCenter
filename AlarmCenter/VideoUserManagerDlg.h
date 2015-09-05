#pragma once
#include "afxcmn.h"
#include "afxwin.h"

namespace core { namespace video { 
	namespace ezviz { class CVideoUserInfoEzviz; };
	namespace normal { class CVideoUserInfoNormal; };
}; };
// CVideoUserManagerDlg dialog

class CVideoUserManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVideoUserManagerDlg)

public:
	CVideoUserManagerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoUserManagerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MGR_VIDEO_USER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	void InitUserList();
	void InsertUserList(core::video::ezviz::CVideoUserInfoEzviz* userInfo);
	void InsertUserList(core::video::normal::CVideoUserInfoNormal* userInfo);
public:
	CListCtrl m_listUser;
	CListCtrl m_listDevice;
	CStatic m_groupDevice;
	virtual BOOL OnInitDialog();
};

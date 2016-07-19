#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "../video/video.h"

// CChooseZoneDlg dialog
namespace core { class group_info; };
#include "core.h"

class CChooseZoneDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChooseZoneDlg)

public:
	CChooseZoneDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CChooseZoneDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CHOOSE_ZONE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	void TraverseGroup(HTREEITEM hItemGroup, core::group_info_ptr group);
	void ResetCurselZoneUudi();
	core::alarm_machine_ptr m_machine;
	core::zone_info_ptr m_zoneInfo;
public:
	CButton m_btnOk;
	CTreeCtrl m_tree;
	CListBox m_listZone;
	CListBox m_listSubMachine;
	CStatic m_staticNote;
	video::zone_uuid m_zone;
	virtual BOOL OnInitDialog();
	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLbnSelchangeListZone();
	afx_msg void OnLbnSelchangeListSubzone();
	afx_msg void OnBnClickedOk();
};

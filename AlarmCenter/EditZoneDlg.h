#pragma once
#include "afxcmn.h"
#include "afxwin.h"


#include "core.h"
class CAlarmMachineDlg;
class CEditZoneDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditZoneDlg)

	class db_updated_observer;
	std::shared_ptr<db_updated_observer> db_updated_observer_ = nullptr;

public:
	CEditZoneDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEditZoneDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_EDIT_ZONE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
	void Init();
	void ExpandWindow(bool expand = true);
	void SelectItem(int zone_value);
	static int __stdcall MyTreeCompareProc(LPARAM lp1, LPARAM lp2, LPARAM lpSort);
	
	bool ChangeDetectorImage(const core::zone_info_ptr& zoneInfo, int newType);
	bool DeleteSubMachine(const core::zone_info_ptr& zoneInfo);
	void AddZone(int zone_value);
	void AddZone(int zoneValue, int gg, int sp, WORD addr);
	bool CreateSubMachine(const core::zone_info_ptr& zoneInfo, bool let_machine_online = false);

public:
	CTreeCtrl m_tree;
	CEdit m_zone;
	CComboBox m_type;
	CEdit m_alias;
	core::alarm_machine_ptr m_machine;
	virtual BOOL OnInitDialog();
	CStatic m_groupSubMachine;
	afx_msg void OnBnClickedButtonAddzone();
	afx_msg void OnBnClickedButtonDelzone();
	afx_msg void OnCbnSelchangeComboZoneType();
	afx_msg void OnEnChangeEditAlias();
	CEdit m_contact;
	CEdit m_addr;
	CEdit m_phone;
	CEdit m_phone_bk;
	HTREEITEM m_rootItem;
	BOOL m_bNeedReloadMaps;
	CAlarmMachineDlg* m_machineDlg;
	afx_msg void OnTvnSelchangedTreeZone(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditContact();
	afx_msg void OnEnChangeEditAddress();
	afx_msg void OnEnChangeEditPhone();
	afx_msg void OnEnChangeEditPhoneBk();
	afx_msg void OnBnClickedButtonEditDetector();
	static void FormatZoneInfoText(const core::alarm_machine_ptr&,
								   core::zone_info_ptr zoneInfo, 
								   CString& txt);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonManageSubmachineExpireTime();
	CButton m_btnAutoRetrieveZoneInfo;
	afx_msg void OnBnClickedButtonAutoRetrieve();
	CButton m_btnDeleteZone;
	CButton m_chk_report_status;
	CButton m_chk_report_status_bk;
	CButton m_chk_report_exception;
	CButton m_chk_report_exception_bk;
	CButton m_chk_report_alarm;
	CButton m_chk_report_alarm_bk;
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheck5();
	afx_msg void OnBnClickedCheck6();
	afx_msg void OnBnClickedButtonManullyAddZoneWriteToMachine();
	CButton m_btnManualyAddZoneWrite2Machine;
	CEdit m_pyisic_addr;
	CButton m_btnBindOrUnbindVideoDevice;
	afx_msg void OnBnClickedButtonBindOrUnbindVideoDevice();
	CButton m_chkAutoPlayVideoOnAlarm;
	afx_msg void OnBnClickedCheckAutoPlayVideoOnAlarm();
protected:
	afx_msg LRESULT OnVideoInfoChanged(WPARAM wParam, LPARAM lParam);
public:
	CButton m_btnPreview;
	afx_msg void OnBnClickedButtonPreview();
	CEdit m_editDevInfo;
	CButton m_btnAddZone;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton m_btn_save;
	afx_msg void OnBnClickedButtonSave();
};

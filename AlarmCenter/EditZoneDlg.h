#pragma once
#include "afxcmn.h"
#include "afxwin.h"


namespace core { class CAlarmMachine; class CZoneInfo; };
// CEditZoneDlg dialog

class CAlarmMachineDlg;
class CEditZoneDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditZoneDlg)

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
	void SelectItem(DWORD_PTR zoneInfo);
	static int __stdcall MyTreeCompareProc(LPARAM lp1, LPARAM lp2, LPARAM lpSort);
	
	bool ChangeDetectorImage(core::CZoneInfo* zoneInfo, int newType);
	bool DeleteSubMachine(core::CZoneInfo* zoneInfo);
	void AddZone(int zone_value);
	void AddZone(int zoneValue, int gg, int sp, WORD addr);
public:
	CTreeCtrl m_tree;
	CEdit m_zone;
	CComboBox m_type;
	CEdit m_alias;
	core::CAlarmMachine* m_machine;
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
	static void FormatZoneInfoText(const core::CAlarmMachine* const machine,
								   const core::CZoneInfo* const zoneInfo, 
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
};

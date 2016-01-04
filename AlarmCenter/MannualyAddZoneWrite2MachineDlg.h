#pragma once
#include "afxwin.h"

#include "core.h"

// CMannualyAddZoneWrite2MachineDlg dialog

class CMannualyAddZoneWrite2MachineDlg : public CDialogEx
{
	typedef ademco::AdemcoEventObserver<CMannualyAddZoneWrite2MachineDlg> ObserverType;
	std::shared_ptr<ObserverType> m_observer;
	DECLARE_DYNAMIC(CMannualyAddZoneWrite2MachineDlg)

public:
	CMannualyAddZoneWrite2MachineDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMannualyAddZoneWrite2MachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MANUALY_ADD_ZONE_WRITE_2_MACHINE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	bool unique_addr(WORD addr);
public:
	afx_msg void OnBnClickedOk();
	core::CAlarmMachinePtr m_machine;
	CComboBox m_cmbZone;
	CEdit m_addr;
	virtual BOOL OnInitDialog();
	CComboBox m_type;
	CComboBox m_property;
	CComboBox m_buglar_property;
	BOOL m_bRestoreSuccess;
	int m_zone;
	int m_gg;
	WORD m_waddr;
	int m_zs;
	DWORD m_dwStartTime;
	afx_msg void OnCbnSelchangeComboType();
	afx_msg void OnCbnSelchangeComboProperty();
	void OnAdemcoEventResult(ademco::AdemcoEventPtr ademcoEvent);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
};

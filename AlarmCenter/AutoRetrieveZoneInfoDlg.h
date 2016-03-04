#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "core.h"

class CAutoRetrieveZoneInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAutoRetrieveZoneInfoDlg)
	typedef ademco::AdemcoEventObserver<CAutoRetrieveZoneInfoDlg> ObserverType;
	std::shared_ptr<ObserverType> m_observer;
public:
	CAutoRetrieveZoneInfoDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CAutoRetrieveZoneInfoDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_AUTO_RETRIEVE_ZONE_INFO };
	core::CAlarmMachinePtr m_machine;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	
	DWORD m_dwStartTime;
	BOOL m_bRetrieving = FALSE;
	std::list<ademco::AdemcoEventPtr> m_event_list;
	std::mutex m_mutex;
protected:
	bool RetrieveZoneInfo(int zoneValue, CString& msg);
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	CButton m_btnStart;
	CProgressCtrl m_progress;
	CStatic m_staticProgress;
	CStatic m_staticTime;
	afx_msg void OnBnClickedButtonStart();
	CListBox m_listctrl;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent);
};

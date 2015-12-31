#pragma once
#include "afxwin.h"
#include "afxbutton.h"
#include "afxcmn.h"
#include "ListBoxEx.h"
#include <list>
#include "BtnST.h"


namespace core { class CAlarmMachine; class CUserInfo; class HistoryRecord; };
// CAlarmMachineDlg dialog

//namespace gui {

class CMapView;
class CAlarmMachineContainerDlg;
class CVideoContainerDlg;
class CAlarmMachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmMachineDlg)
	friend void __stdcall OnNewRecord(void* udata, const core::HistoryRecord* record);
public:
	CAlarmMachineDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CAlarmMachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MACHINE };
	CStringList m_recordList;
	CLock m_lock4RecordList;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_groupControlPanel;
	CStatic m_groupContent;
	void SetMachineInfo(core::CAlarmMachine* machine);
	//void SetSubMachineInfo(core::CSubMachineInfo* subMachine);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	void OnAdemcoEventResult(ademco::AdemcoEventPtr ademcoEvent);
	void HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);
	int GetAdemcoID() const;
	int GetZoneValue() const;
private:
	/*union 
	{
		core::CAlarmMachine* machine;
		core::CSubMachineInfo* subMachine;
	}m_machine;*/
	core::CAlarmMachine* m_machine;
	int m_maxHistory2Show;
	//int m_machineType;
	int m_nRemoteControlTimeCounter;
	int m_curRemoteControlCommand;
	CString m_strBtn1;
	CString m_strBtn2;
	CString m_strBtn3;
	typedef struct TabViewWithNdx
	{
		CWnd* _tabView;
		LONG _ndx;
		TabViewWithNdx() : _tabView(nullptr), _ndx(-1) {}
		TabViewWithNdx(CWnd* tabView, LONG ndx) : _tabView(tabView), _ndx(ndx) {}
	}TabViewWithNdx;
	std::list<TabViewWithNdx*> m_tabViewList;
	CAlarmMachineContainerDlg* m_container;
	CVideoContainerDlg* m_videoContainerDlg;

	std::list<ademco::AdemcoEvent*> _ademcoEventList;
	CLock m_lock4AdemcoEventList;
protected:
	//void DispatchAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);
	void ClearMsg();
	void LoadMaps();
	void ReleaseMaps();
	void UpdateBtn123();
	void CheckIfExpire();
	void UpdateCaption();
	//void OnQueryResult(const ademco::AdemcoEvent* ademcoEvent);
public:
	CButton m_btn1;
	CButton m_btn2;
	CButton m_btn3;
	CButton m_btnClearMsg;
	CStatic m_staticNet;
	CStatic m_staticStatus;
	CTabCtrl m_tab;
	gui::control::CListBoxEx m_listHistory;
	CStatic m_groupHistory;
	CButton m_btnEditVideoInfo;
	CStatic m_staticConn;
	CStatic m_staticMachineStatus;
	gui::control::CButtonSTUrlLike m_btnSeeMoreHr;
	CButton m_btnManageExpire;
public:
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButtonClearmsg();
	afx_msg LRESULT OnNewrecordResult(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonEditZone();
	afx_msg LRESULT OnInversionControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonEditMap();
	afx_msg void OnBnClickedButtonEditDetector();
	afx_msg void OnBnClickedButtonMoreHr();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonSeeBaiduMap();
	afx_msg void OnBnClickedButtonManageExpire();
	CButton m_btnEditZone;
	CButton m_btnEditMap;
	CButton m_btnEditDetector;
	void OnCurUserChangedResult(const core::CUserInfo* user);
	afx_msg void OnBnClickedButtonMgrCameraIcon();
};






//NAMESPACE_END

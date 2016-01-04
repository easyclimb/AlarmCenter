#pragma once
#include "afxwin.h"
#include "afxbutton.h"
#include "afxcmn.h"
#include "ListBoxEx.h"
#include <list>
#include <memory>
#include "BtnST.h"

#include "core.h"


class CMapView;
class CAlarmMachineContainerDlg;
class CVideoContainerDlg;
class CAlarmMachineDlg : public CDialogEx
{
	class CurUserChangedObserver : public dp::observer<core::CUserInfoPtr>
	{
	public:
		explicit CurUserChangedObserver(CAlarmMachineDlg* dlg) : _dlg(dlg) {}
		void on_update(const core::CUserInfoPtr& ptr) {
			if (_dlg) {
				_dlg->OnCurUserChangedResult(ptr);
			}
		}
	private:
		CAlarmMachineDlg* _dlg;
	};

	std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;

	class NewRecordObserver;
	std::shared_ptr<NewRecordObserver> m_new_record_observer;

	std::shared_ptr<ademco::AdemcoEventObserver<CAlarmMachineDlg>> m_observer;
	DECLARE_DYNAMIC(CAlarmMachineDlg)
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
	void SetMachineInfo(const core::CAlarmMachinePtr& machine);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	void OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent);
	void HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent);
	int GetAdemcoID() const;
	int GetZoneValue() const;
private:
	core::CAlarmMachinePtr m_machine;
	int m_maxHistory2Show;
	int m_nRemoteControlTimeCounter;
	int m_curRemoteControlCommand;
	CString m_strBtn1;
	CString m_strBtn2;
	CString m_strBtn3;

	typedef struct TabViewWithNdx
	{
		core::CWndPtr _tabView;
		LONG _ndx;
		TabViewWithNdx() : _tabView(), _ndx(-1) {}
		TabViewWithNdx(core::CWndPtr tabView, LONG ndx) : _tabView(tabView), _ndx(ndx) {}
	}TabViewWithNdx;
	typedef std::shared_ptr<TabViewWithNdx> TabViewWithNdxPtr;
	std::list<TabViewWithNdxPtr> m_tabViewList;

	std::shared_ptr<CAlarmMachineContainerDlg> m_container;

	std::list<ademco::AdemcoEventPtr> _ademcoEventList;
	CLock m_lock4AdemcoEventList;
protected:
	void ClearMsg();
	void LoadMaps();
	void ReleaseMaps();
	void UpdateBtn123();
	void CheckIfExpire();
	void UpdateCaption();
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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonEditZone();
	void OnInversionControl(core::CWndPtr wnd, core::InversionControlMapCommand cmd);
	afx_msg void OnBnClickedButtonEditMap();
	afx_msg void OnBnClickedButtonEditDetector();
	afx_msg void OnBnClickedButtonMoreHr();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonSeeBaiduMap();
	afx_msg void OnBnClickedButtonManageExpire();
	CButton m_btnEditZone;
	CButton m_btnEditMap;
	CButton m_btnEditDetector;
	void OnCurUserChangedResult(const core::CUserInfoPtr& user);
	afx_msg void OnBnClickedButtonMgrCameraIcon();
};






//NAMESPACE_END

#pragma once
#include "afxwin.h"
#include "afxbutton.h"
#include "afxcmn.h"
#include <list>

namespace core { class CAlarmMachine; /*class CSubMachineInfo;*/ };
// CAlarmMachineDlg dialog

//namespace gui {

class CMapView;
class CAlarmMachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmMachineDlg)

public:
	CAlarmMachineDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmMachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MACHINE };

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
	void OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent);
	int GetAdemcoID() const;
private:
	/*union 
	{
		core::CAlarmMachine* machine;
		core::CSubMachineInfo* subMachine;
	}m_machine;*/
	core::CAlarmMachine* m_machine;
	//int m_machineType;
	
	typedef struct MapViewWithNdx
	{
		CMapView* _mapView;
		LONG _ndx;
		MapViewWithNdx() : _mapView(NULL), _ndx(-1) {}
		MapViewWithNdx(CMapView* mapView, LONG ndx) : _mapView(mapView), _ndx(ndx) {}
	}MapViewWithNdx;
	std::list<MapViewWithNdx*> m_mapViewList;
protected:
	void DispatchAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);
	void ClearMsg();
public:
	CMFCButton m_btnArm;
	CMFCButton m_btnDisarm;
	CMFCButton m_btnEmergency;
	CMFCButton m_btnClearMsg;
	CStatic m_staticNet;
	CStatic m_staticStatus;
	CTabCtrl m_tab;
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	afx_msg LRESULT OnDispatchevent(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedButtonArm();
	afx_msg void OnBnClickedButtonDisarm();
	afx_msg void OnBnClickedButtonEmergency();
	afx_msg void OnBnClickedButtonClearmsg();
protected:
	afx_msg LRESULT OnNewrecordResult(WPARAM wParam, LPARAM lParam);
public:
	CListBox m_listHistory;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_groupHistory;
	CButton m_btnEditVideoInfo;
	afx_msg void OnBnClickedButtonEditZone();
protected:
	afx_msg LRESULT OnNewalarmtext(WPARAM wParam, LPARAM lParam);
public:
	CStatic m_staticConn;
};






//NAMESPACE_END

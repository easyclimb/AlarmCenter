#pragma once
#include "afxwin.h"
#include "ListBoxST.h"
#include "ToolTipButton.h"
#include <list>

namespace core { class CDetectorInfo;};
#include "core.h"
// CEditDetectorDlg dialog

class CEditDetectorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditDetectorDlg)

public:
	CEditDetectorDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEditDetectorDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_EDIT_DETECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	std::list<core::CDetectorInfo*> m_detList;
	std::list<core::CDetectorInfo*> m_bindList;
	std::list<core::CDetectorInfo*> m_unbindList;
	CImageList m_ImageList;
	CImageList m_ImageListRotate;
	core::CMapInfoPtr m_prevSelMapInfo;
	core::CZoneInfo* m_prevSelZoneInfo;
protected:
	void LoadDetectors(std::list<core::CDetectorInfo*>& list);
	void FormatDetectorText(const core::CDetectorInfo* const detectorInfo, CString& txt);
	void InitComboSeeAndDetList();
	void DisableRightUi();
	core::CZoneInfo* ChooseNoDetZoneInfo(const CPoint& pt);
	void RotateDetector(int step);
	void ChangeDistance(bool bFar = true);

	enum DetectorMoveDirection
	{
		DMD_UP,
		DMD_DOWN,
		DMD_LEFT,
		DMD_RIGHT,
	};
	void MoveWithDirection(DetectorMoveDirection dmd);
public:
	gui::control::CListBoxST m_list;
	CComboBox m_cmbSee;
	CEdit m_editZone;
	CEdit m_editMap;
	CButton m_btnBindMap;
	CButton m_btnUnbindMap;
	CButton m_btnBindZone;
	CButton m_btnUnbindZone;
	CComboBox m_cmbAngle;
	gui::control::CToolTipButton m_btnRotateUnticlock;
	gui::control::CToolTipButton m_btnRotateClock;
	gui::control::CToolTipButton m_btnDistanceFar;
	gui::control::CToolTipButton m_btnDistanceNear;
	gui::control::CToolTipButton m_btnMoveUp;
	gui::control::CToolTipButton m_btnMoveLeft;
	gui::control::CToolTipButton m_btnMoveDown;
	gui::control::CToolTipButton m_btnMoveRight;
	core::CAlarmMachine* m_machine;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeComboSee();
	afx_msg void OnLbnSelchangeListDetector();
	afx_msg void OnBnClickedButtonBindZone();
	afx_msg void OnBnClickedButtonUnbindZone();
	afx_msg void OnBnClickedButtonEditZone();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonBindMap();
	afx_msg void OnBnClickedButtonUnbindMap();
	afx_msg void OnBnClickedButtonAddDetector();
	afx_msg void OnBnClickedButtonEditMap();
	afx_msg void OnBnClickedButtonDelDetector();
	afx_msg void OnBnClickedButtonMoveUp();
	afx_msg void OnBnClickedButtonMoveDown();
	afx_msg void OnBnClickedButtonMoveLeft();
	afx_msg void OnBnClickedButtonMoveRight();
	afx_msg void OnBnClickedButtonDistanceFar();
	afx_msg void OnBnClickedButtonDistanceNear();
	afx_msg void OnBnClickedButtonRotateUnticlock();
	afx_msg void OnBnClickedButtonRotateClock();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CButton m_btnDeleteDetector;
};

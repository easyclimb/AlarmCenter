#pragma once
#include "afxwin.h"
#include "ListBoxST.h"
#include <list>

namespace core { class CAlarmMachine; class CDetectorInfo; };
// CEditDetectorDlg dialog

class CEditDetectorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditDetectorDlg)

public:
	CEditDetectorDlg(CWnd* pParent = NULL);   // standard constructor
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
protected:
	void LoadDetectors(std::list<core::CDetectorInfo*>& list);
	void FormatDetectorText(const core::CDetectorInfo* const detectorInfo, CString& txt);
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
	CButton m_btnRotateUnticlock;
	CButton m_btnRotateClock;
	CButton m_btnDistanceFar;
	CButton m_btnDistanceNear;
	CButton m_btnMoveUp;
	CButton m_btnMoveLeft;
	CButton m_btnMoveDown;
	CButton m_btnMoveRight;
	core::CAlarmMachine* m_machine;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeComboSee();
	afx_msg void OnLbnSelchangeListDetector();
	afx_msg void OnBnClickedButtonBindZone();
	afx_msg void OnBnClickedButtonUnbindZone();
	afx_msg void OnBnClickedButtonEditZone();
	
};

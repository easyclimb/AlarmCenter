#pragma once
#include "afxwin.h"
#include "ListBoxST.h"
#include "ToolTipButton.h"
#include <list>

namespace core { class CAlarmMachine; class CDetectorInfo; class CMapInfo; class CCameraInfo; };
// CEditCameraDlg dialog

class CEditCameraDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditCameraDlg)

public:
	CEditCameraDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEditCameraDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_EDIT_CAMERA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	std::list<core::CCameraInfo*> m_cameraList;
	CImageList m_ImageList;
	CImageList m_ImageListRotate;
	core::CMapInfo* m_prevSelMapInfo;
	core::CCameraInfo* m_prevSelCameraInfo;
protected:
	void LoadCameras(std::list<core::CCameraInfo*>& cameraList);
	void FormatText(const core::CCameraInfo* const camera, CString& txt);
	void InitComboSeeAndDetList();
	void DisableRightUi();
	bool DoJudgeType();
	void RotateDetector(int step);

	enum CameraMoveDirection
	{
		CMD_UP,
		CMD_DOWN,
		CMD_LEFT,
		CMD_RIGHT,
	};
	void MoveWithDirection(CameraMoveDirection cmd);
public:
	gui::control::CListBoxST m_list;
	CComboBox m_cmbSee;
	CComboBox m_cmbAngle;
	gui::control::CToolTipButton m_btnRotateUnticlock;
	gui::control::CToolTipButton m_btnRotateClock;
	gui::control::CToolTipButton m_btnMoveUp;
	gui::control::CToolTipButton m_btnMoveLeft;
	gui::control::CToolTipButton m_btnMoveDown;
	gui::control::CToolTipButton m_btnMoveRight;
	core::CAlarmMachine* m_machine;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeComboSee();
	afx_msg void OnLbnSelchangeListCamera();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonAddCamera();
	afx_msg void OnBnClickedButtonDelCamera();
	afx_msg void OnBnClickedButtonMoveUp();
	afx_msg void OnBnClickedButtonMoveDown();
	afx_msg void OnBnClickedButtonMoveLeft();
	afx_msg void OnBnClickedButtonMoveRight();
	afx_msg void OnBnClickedButtonRotateUnticlock();
	afx_msg void OnBnClickedButtonRotateClock();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CButton m_btnDeleteCamera;
};

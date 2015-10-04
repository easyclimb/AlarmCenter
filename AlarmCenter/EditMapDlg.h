#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "StaticBmp.h"


namespace core { class CAlarmMachine; class CMapInfo; };
// CEditMapDlg dialog

class CEditMapDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditMapDlg)

public:
	CEditMapDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEditMapDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_EDIT_MAP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	core::CMapInfo* m_prevSelMapInfo;
protected:
	void FormatMapText(core::CMapInfo* mapInfo, CString& txt);
	BOOL OpenFile(CString& path);
public:
	CEdit m_alias;
	CEdit m_file;
	CTreeCtrl m_tree;
	gui::control::CStaticBmp m_preview;
	HTREEITEM m_rootItem;
	BOOL m_bNeedReloadMaps;
	core::CAlarmMachine* m_machine;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonAddMap();
	afx_msg void OnBnClickedButtonDelMap();
	afx_msg void OnEnChangeEditAlias();
	afx_msg void OnBnClickedButtonChangeFile();
	virtual BOOL OnInitDialog();
	afx_msg void OnTvnSelchangedTreeMap(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
	CButton m_btnDeleteMap;
};

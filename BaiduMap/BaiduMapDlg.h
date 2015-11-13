
// BaiduMapDlg.h : 头文件
//

#pragma once
#include <string>

// CBaiduMapDlg 对话框
class CBaiduMapDlg : public CDHtmlDialog
{
// 构造
public:
	CBaiduMapDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_BAIDUMAP_DIALOG, IDH = IDR_HTML_BAIDUMAP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;
	CString m_title;
	std::wstring m_url;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	bool CBaiduMapDlg::GenerateHtml(std::wstring& url,
									double x,
									double y,
									int zoomLevel,
									const CString& title);
};

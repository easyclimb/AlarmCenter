
// BaiduMapDlg.h : ͷ�ļ�
//

#pragma once
#include <string>

// CBaiduMapDlg �Ի���
class CBaiduMapDlg : public CDHtmlDialog
{
// ����
public:
	CBaiduMapDlg(CWnd* pParent = nullptr);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_BAIDUMAP_DIALOG, IDH = IDR_HTML_BAIDUMAP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;
	CString m_title;
	std::wstring m_url;

	// ���ɵ���Ϣӳ�亯��
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

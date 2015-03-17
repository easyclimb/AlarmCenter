// ChooseDetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ChooseDetDlg.h"
#include "afxdialogex.h"
#include "DetectorLib.h"
#include "BmpEx.h"

using namespace core;
// CChooseDetDlg dialog

IMPLEMENT_DYNAMIC(CChooseDetDlg, CDialogEx)

CChooseDetDlg::CChooseDetDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChooseDetDlg::IDD, pParent)
	, m_detType2Show(DT_MAX)
{

}

CChooseDetDlg::~CChooseDetDlg()
{
}

void CChooseDetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_staticNote);
	DDX_Control(pDX, IDC_LIST2, m_list);
}


BEGIN_MESSAGE_MAP(CChooseDetDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CChooseDetDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChooseDetDlg message handlers


void CChooseDetDlg::OnBnClickedOk()
{

	CDialogEx::OnOK();
}


BOOL CChooseDetDlg::OnInitDialog()
{
	using namespace gui::control;
	CDialogEx::OnInitDialog();

	CDetectorLib* lib = CDetectorLib::GetInstance();
	std::list<CDetectorLibData*> list;
	lib->GetAllLibData(list);
	m_ImageList.SetImageCount(list.size());
	m_ImageListRotate.SetImageCount(list.size());

	CString path = _T("");
	std::list<CDetectorLibData*>::iterator iter = list.begin();
	int ndx = 0;
	while (iter != list.end()) {
		CDetectorLibData* data = *iter++;
		path = data->get_path();
		HBITMAP hBitmap = CBmpEx::GetHBitmapThumbnail(path, THUMBNAILWIDTH, THUMBNAILWIDTH);
		if (hBitmap) {
			CBitmap *pImage = new CBitmap();
			pImage->Attach(hBitmap);
			m_ImageList.Replace(ndx, pImage, NULL);
			if (data->get_type() == DT_DOUBLE) {
				HBITMAP hBitmapPair = CBmpEx::GetHBitmapThumbnail(data->get_path_pair(),
																  THUMBNAILWIDTH,
																  THUMBNAILWIDTH);
				CBitmap *pImagePair = new CBitmap();
				pImagePair->Attach(hBitmapPair);
				m_ImageListRotate.Replace(ndx, pImagePair, NULL);
				delete pImagePair;	pImagePair = NULL;
			} else {
				m_ImageListRotate.Replace(ndx, pImage, NULL);
			}
			delete pImage; pImage = NULL;
			ndx++;
		}
	}

	m_list.SetImageList(&m_ImageList, &m_ImageListRotate);

	CString txt = _T("");
	ndx = 0;
	iter = list.begin();
	while (iter != list.end()) {
		CDetectorLibData* data = *iter++;
		txt.Format(L"%s", data->get_detector_name());
		m_list.InsertString(ndx, txt, ndx);
		m_list.SetItemData(ndx, data->get_id());
		ndx++;
	}
	m_list.Invalidate();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

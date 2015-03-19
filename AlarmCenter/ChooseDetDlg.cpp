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
	, m_chosenDetectorID(-1)
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
	ON_LBN_SELCHANGE(IDC_LIST2, &CChooseDetDlg::OnLbnSelchangeListDetector)
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
	m_ImageList.Create(THUMBNAILWIDTH, THUMBNAILWIDTH, ILC_COLOR24, 0, 1);
	m_ImageListRotate.Create(THUMBNAILWIDTH, THUMBNAILWIDTH, ILC_COLOR24, 0, 1);

	bool bSettedType = (m_detType2Show < DT_MAX);

	CString path = _T("");
	std::list<CDetectorLibData*>::iterator iter = list.begin();
	int ndx = 0;
	
	while (iter != list.end()) {
		CDetectorLibData* data = *iter++;
		if (bSettedType && ((data->get_type() & m_detType2Show) == 0))
			continue;
		ndx++;
	}
	m_ImageList.SetImageCount(ndx);
	m_ImageListRotate.SetImageCount(ndx);

	ndx = 0;
	iter = list.begin();
	while (iter != list.end()) {
		CDetectorLibData* data = *iter++;
		if (bSettedType && ((data->get_type() & m_detType2Show) == 0))
			continue;
		
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

	ndx = 0;
	iter = list.begin();
	while (iter != list.end()) {
		CDetectorLibData* data = *iter++;
		if (bSettedType && ((data->get_type() & m_detType2Show) == 0))
			continue;
		m_list.InsertString(ndx, data->get_detector_name(), ndx);
#ifdef DEBUG
		CString	sText;
		int textLen = m_list.GetTextLen(ndx);
		m_list.GetText(ndx, sText.GetBuffer(textLen));
		sText.ReleaseBuffer();
		LOG(L"%s\n", sText);
#endif
		m_list.SetItemData(ndx, data->get_id());
		ndx++;
	}
	m_list.Invalidate();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CChooseDetDlg::OnLbnSelchangeListDetector()
{
	int ndx = m_list.GetCurSel();
	DWORD id = m_list.GetItemData(ndx);
	CDetectorLib* lib = CDetectorLib::GetInstance();
	const CDetectorLibData* libData = lib->GetDetectorLibData(id);
	if (libData) {
		m_chosenDetectorID = libData->get_id();
	}
}
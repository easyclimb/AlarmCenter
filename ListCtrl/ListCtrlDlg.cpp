// ListCtrlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ListCtrl.h"
#include "ListCtrlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCtrlDlg dialog

CListCtrlDlg::CListCtrlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CListCtrlDlg::IDD)
{
	//{{AFX_DATA_INIT(CListCtrlDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_iItemCount = 0;
}

void CListCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CListCtrlDlg)
	DDX_Control(pDX, IDC_LIST1, m_List);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CListCtrlDlg, CDialog)
	//{{AFX_MSG_MAP(CListCtrlDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_LST_DELETE, OnLstDelete)
	ON_COMMAND(ID_LST_INSERT, OnLstInsert)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, OnRclickList)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_VALIDATE, OnEndLabelEditVariableCriteria)
	ON_MESSAGE(WM_SET_ITEMS, PopulateComboList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCtrlDlg message handlers

BOOL CListCtrlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	// Make the necessary initializations
	m_List.InsertColumn(0, L"Col1", LVCFMT_LEFT, 150);
	m_List.InsertColumn(1, L"Col1", LVCFMT_LEFT, 150);
	m_List.InsertColumn(2, L"Col1", LVCFMT_LEFT, 150);

	m_List.SetReadOnlyColumns(0);
	m_List.SetComboColumns(1);

	CString strValidChars = "0123456789.";
	m_List.SetValidEditCtrlCharacters(strValidChars);
	m_List.EnableVScroll(); 

	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CListCtrlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CListCtrlDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CListCtrlDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CListCtrlDlg::OnLstDelete() 
{
	// TODO: Add your command handler code here
	// Get the current selected item
	POSITION SelectionPos = m_List.GetFirstSelectedItemPosition();
	int iCurSel = -1;

	// Delete the selected item from the list control 
	while (NULL != SelectionPos)
	{
		SelectionPos = m_List.GetFirstSelectedItemPosition();
		iCurSel = m_List.GetNextSelectedItem(SelectionPos);
		m_List.DeleteItem(iCurSel);
	}
}

void CListCtrlDlg::OnLstInsert() 
{
	// TODO: Add your command handler code here
	// Insert a new row below the selected row or at the end
	CString strResource;
	strResource.Format(L"%d", ++m_iItemCount);

	int iCount = m_List.GetItemCount();

	int iRow = -1;

	POSITION SelectedItemPos = m_List.GetFirstSelectedItemPosition();
	int iIndex = -1;
	int iLastSelectedIndex = -1;

	while (SelectedItemPos)
	{
		iIndex = m_List.GetNextSelectedItem(SelectedItemPos);

		if ((LVIS_SELECTED == m_List.GetItemState(iIndex, LVIS_SELECTED)) && 
			(LVIS_FOCUSED == m_List.GetItemState(iIndex, LVIS_FOCUSED)))
		{
			iRow = iIndex;
		}
		m_List.SetItemState(iIndex, 0, LVIS_SELECTED|LVIS_FOCUSED);
	}


	if (-1 == iRow)
	{
		m_List.InsertItem(LVIF_TEXT|LVIF_STATE, iCount, 
			strResource, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED, 0, 0);
	}
	else
	{
		m_List.InsertItem(LVIF_TEXT|LVIF_STATE, iRow, 
			strResource, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED, 0, 0);
	}
}

void CListCtrlDlg::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	// Create the pop up menu
	CMenu obMenu;
	obMenu.LoadMenu(IDR_LIST_POPUP); 

	CMenu* pPopupMenu = obMenu.GetSubMenu(0);
	ASSERT(pPopupMenu); 
	
	// Get the cursor position
	CPoint obCursorPoint = (0, 0);
	
	GetCursorPos(&obCursorPoint);

	if (0 >= m_List.GetSelectedCount())
	{
		pPopupMenu->EnableMenuItem(ID_LST_DELETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED); 
	}
	else
	{
		pPopupMenu->EnableMenuItem(ID_LST_DELETE, MF_BYCOMMAND | MF_ENABLED);
	}

	// Track the popup menu
	pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, obCursorPoint.x, 
									obCursorPoint.y, this);

	*pResult = 0;
}

LRESULT CListCtrlDlg::OnEndLabelEditVariableCriteria(WPARAM wParam, LPARAM lParam) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
	// TODO: Add your control notification handler code here
	
	if (wParam == IDC_LIST)
	{
		// Update the item text with the new text
		CString strUpdatedTxt = pDispInfo->item.pszText;  
		
		// Validate the format of the new string for the edit columns
		// If invalid then
		// Set pResult to 1 
		switch(pDispInfo->item.iSubItem)
		{
		case 0:
		case 1:
			break;
		default:
			{
				if (!strUpdatedTxt.IsEmpty()) 
				{
					// Get the left most non numeral characters
					// Get the remaining numerals
					// If the left most part does not contain the mandatory conditions 
					// Or the rest contains the mandatory conditions 
					// Display an error and set focus back onto the control
					int iFirstNumeralOccurrance = strUpdatedTxt.FindOneOf(L"0123456789.");
					CString strNonNumerals = strUpdatedTxt.Left(iFirstNumeralOccurrance);
					CString strNumerals = strUpdatedTxt.Mid(iFirstNumeralOccurrance);
					
					strNonNumerals.TrimLeft(); 
					strNonNumerals.TrimRight();
					strNumerals.TrimLeft();
					strNumerals.TrimRight();  

					int iDecimalIndex = strNumerals.Find(L".");
					
					if ((-1 != iDecimalIndex) && (iDecimalIndex != (strNumerals.GetLength() - 1)))  
					{
						iDecimalIndex = strNumerals.Find(L".", iDecimalIndex + 1);
					}

					// Check that the condition is either
					// Not empty or "=" or "!=" or "<" or ">" or "<=" or ">="
					// The Numerals do not contain 
					// Space or more than 1 "." or any of the conditions
					if (
						(!strNonNumerals.IsEmpty()) || 
						(-1 != strNumerals.Find(L" ")) ||
						(-1 != iDecimalIndex)
						)
					{
						AfxMessageBox(L"Invalid text entered. The text will be reset");
						m_List.SetItemText(pDispInfo->item.iItem, pDispInfo->item.iSubItem, L"");
					}
					return 0;
				}
			}
			break;
		}
	}

	return 1;
}

LRESULT CListCtrlDlg::PopulateComboList(WPARAM wParam, LPARAM lParam)
{
	// Get the Combobox window pointer
	CComboBox* pInPlaceCombo = static_cast<CComboBox*> (GetFocus());

	// Get the inplace combbox top left
	CRect obWindowRect;

	pInPlaceCombo->GetWindowRect(&obWindowRect);
	
	CPoint obInPlaceComboTopLeft(obWindowRect.TopLeft()); 
	
	// Get the active list
	// Get the control window rect
	// If the inplace combobox top left is in the rect then
	// The control is the active control
	m_List.GetWindowRect(&obWindowRect);
	
	int iColIndex = wParam;
	
	CStringList* pComboList = reinterpret_cast<CStringList*>(lParam);
	pComboList->RemoveAll(); 

	if (obWindowRect.PtInRect(obInPlaceComboTopLeft)) 
	{
		if (1 == iColIndex)
		{
			pComboList->AddTail("Item 1");
			pComboList->AddTail("Item 2");
			pComboList->AddTail("Item 3");
		}
	}
	
	return true;
}

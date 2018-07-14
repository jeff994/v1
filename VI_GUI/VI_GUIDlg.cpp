
// VI_GUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VI_GUI.h"
#include "VI_GUIDlg.h"
#include "afxdialogex.h"
#include "Utility.h"
#include <functional>
#include <iostream>
#include <chrono>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::string sconfig("config.json");

// CVIGUIDlg dialog

std::mutex pt_lock; 

CVIGUIDlg::CVIGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VI_GUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bConnected = false; 
	m_eAntena1PanStatus = AntenaStatus::Idle;
	m_eAntena1TiltStatus = AntenaStatus::Idle;
	m_eAntena2PanStatus = AntenaStatus::Idle;
	m_eAntena2TiltStatus = AntenaStatus::Idle;
	m_bEnableHome = true; 
	m_bEnableLeft = true;
	m_bEnableUp = true;
	m_bEnableDown = true;
	m_bEnableRight = true;
	m_bExecute = true; 
}

void CVIGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO_CONNECT, m_radioConnectionStatus);
	DDX_Control(pDX, IDC_EDIT_PAN_ANTENA_ONE, m_editPanAntenaOne);
	DDX_Control(pDX, IDC_EDIT_TILT_ANTENA_ONE, m_editTiltAntenaOne);
	DDX_Control(pDX, IDC_EDIT_PAN_ANTENA_TWO, m_editPanAntenaTwo);
	DDX_Control(pDX, IDC_EDIT_TILT_ANTENA_TWO, m_editTiltAntenaTwo);
	//DDX_Control(pDX, IDC_BUTTON_RIGHT, m_buttonRight);
	//DDX_Control(pDX, IDC_BUTTON_DOWN, m_buttonDown);
	//DDX_Control(pDX, IDC_BUTTON_HOME, m_buttonHome);
	//DDX_Control(pDX, IDC_BUTTON_UP, m_buttonUp);
	//DDX_Control(pDX, IDC_BUTTON_LEFT, m_buttonLeft);
}

BEGIN_MESSAGE_MAP(CVIGUIDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CVIGUIDlg::OnBnClickedButtonRight)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CVIGUIDlg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CVIGUIDlg::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CVIGUIDlg::OnBnClickedButtonDown)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(TTN_SHOW, 0, OnToolTipTextAboutToShow)
	ON_BN_CLICKED(IDC_BUTTON_HOME, &CVIGUIDlg::OnBnClickedButtonHome)
	ON_WM_INITMENUPOPUP()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CVIGUIDlg message handlers

BOOL CVIGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	EnableToolTips(TRUE);
	if (!m_toolTip.Create(this))
	{
		TRACE0("Unable to create the ToolTip!");
	}
	else
	{
		// Add tool tips to the controls, either by hard coded string 
		// or using the string table resource
		//m_ToolTip.AddTool(&m_bu, _T("This is a tool tip!"));
		//m_ToolTip.AddTool(&m_myEdit, IDS_MY_EDIT);m_toolTip
		m_toolTip.AddTool(GetDlgItem(IDC_BUTTON_RIGHT), L"Turn right");
		m_toolTip.AddTool(GetDlgItem(IDC_BUTTON_LEFT), L"Turn left");
		m_toolTip.AddTool(GetDlgItem(IDC_BUTTON_HOME), L"Go to home position");
		m_toolTip.AddTool(GetDlgItem(IDC_BUTTON_UP), L"Go up");
		m_toolTip.AddTool(GetDlgItem(IDC_BUTTON_DOWN), L"Go down");
		m_toolTip.Activate(TRUE);
	}

	// TODO: Add extra initialization here
	m_pEditBkBrush_Yellow = new CBrush(RGB(255, 255, 0));
	m_pEditBkBrush_Green = new CBrush(RGB(0, 255, 0));
	m_pEditBkBrush_Red = new CBrush(RGB(255, 0, 0));
	m_radioConnectionStatus.SetCheck(0);
	m_editPanAntenaOne.SetWindowTextW(L"0");
	m_editTiltAntenaOne.SetWindowTextW(L"0");
	m_editPanAntenaTwo.SetWindowTextW(L"0");
	m_editTiltAntenaTwo.SetWindowTextW(L"0");
	::SetWindowTheme(GetDlgItem(IDC_RADIO_CONNECT)->GetSafeHwnd(), L"wstr", L"wstr");
	
	Utility::_ReadConfigJson(sconfig, m_sServerAddrss, m_sPortLeft, m_sPortRight);
	m_tcpClientLeft.SetAddress(m_sServerAddrss);
	m_tcpClientLeft.SetPort(m_sPortLeft);
	m_tcpClientRight.SetAddress(m_sServerAddrss);
	m_tcpClientRight.SetPort(m_sPortRight);
	Reconnect();
	EnableDisableButtons();
	/*std::string t("? F 2\rF=-100\r"); 
	int nValue; 
	Utility::_ParseQueryFeedback(t, nValue);*/
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void ReciveFeedback(CVIGUIDlg * dlg)
{
	while (dlg->m_bConnected)
	{
		std::string sFeedbackPanOne, sFeedbackPanTwo;
		std::string sFeedbackTiltOne, sFeedbackTiltTwo;
		std::string sQueryPan("?F 1\r"), sQueryTilt("?F 2\r"); 
		bool bRet = true; 
		// Send query 
		pt_lock.lock();
		bRet = dlg->m_tcpClientLeft.SendString(sQueryPan);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		bRet = dlg->m_tcpClientLeft.ReceiveString(sFeedbackPanOne);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		pt_lock.unlock();

		pt_lock.lock();
		bRet = dlg->m_tcpClientLeft.SendString(sQueryTilt);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		bRet = dlg->m_tcpClientLeft.ReceiveString(sFeedbackTiltTwo);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		pt_lock.unlock();

		// Parse the feedback and update 
		pt_lock.lock();
		bRet = dlg->m_tcpClientRight.SendString(sQueryPan);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		bRet = dlg->m_tcpClientRight.ReceiveString(sFeedbackPanTwo);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		pt_lock.unlock();

		pt_lock.lock();
		bRet = dlg->m_tcpClientRight.SendString(sQueryTilt);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		bRet = dlg->m_tcpClientRight.ReceiveString(sFeedbackTiltTwo);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		pt_lock.unlock();

		int nPanOne, nPanTwo, nTiltOne, nTiltTwo; 
		// Parsing values 
		Utility::_ParseQueryFeedback(sFeedbackTiltOne, nTiltOne);
		Utility::_ParseQueryFeedback(sFeedbackTiltTwo, nTiltTwo);
		Utility::_ParseQueryFeedback(sFeedbackPanOne, nPanOne);
		Utility::_ParseQueryFeedback(sFeedbackPanTwo, nPanTwo);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		
		dlg->m_nCurrentPanOne = (nPanOne * 90) / 1000; 
		dlg->m_nCurrentPanTwo = (nPanTwo * 90) / 1000;

		dlg->m_nCurrentTiltOne = (nTiltOne * 90) / 1000;
		dlg->m_nCurrentTiltTwo = (nTiltTwo * 90) / 1000;
	}
	// parse the feedback and update 
}

void SendCommands(CVIGUIDlg * dlg)
{
	// Check whether it's connected 
	while(dlg->m_bConnected)
	{
		// Send command 
		std::string sFeedbackLeft, sFeedbackRight; 
		int nPanOne = (dlg->m_nRequestedPanOne * 1000) / 90; 
		int nPanTwo = (dlg->m_nCurrentPanTwo * 1000) / 90; 
		int nTiltOne = (dlg->m_nCurrentTiltOne * 1000) / 90;
		int nTiltTwo = (dlg->m_nCurrentTiltTwo * 1000) / 90; 
		std::string sPanCommandOne("!G 1 " + std::to_string(nPanOne) + "\r");
		std::string sTiltCommandOne("!G 2 " + std::to_string(nTiltOne) + "\r");
		std::string sPanCommandTwo("!G 1 " + std::to_string(nPanTwo) + "\r");
		std::string sTiltCommandTwo("!G 2 " + std::to_string(nTiltTwo) + "\r");
		bool bRet = true; 

		pt_lock.lock();
		bRet = dlg->m_tcpClientLeft.SendString(sPanCommandOne);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		bRet = dlg->m_tcpClientLeft.ReceiveString(sFeedbackLeft);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		pt_lock.unlock();
		
		pt_lock.lock();
		bRet = dlg->m_tcpClientLeft.SendString(sTiltCommandOne);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		bRet = dlg->m_tcpClientLeft.ReceiveString(sFeedbackLeft);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		pt_lock.unlock();
		
		pt_lock.lock();
		bRet = dlg->m_tcpClientRight.SendString(sPanCommandTwo);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		dlg->m_tcpClientRight.ReceiveString(sFeedbackRight);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		pt_lock.unlock();

		pt_lock.lock();
		bRet = dlg->m_tcpClientRight.SendString(sTiltCommandTwo);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		dlg->m_tcpClientRight.ReceiveString(sFeedbackRight);
		if (!bRet) { pt_lock.unlock(); dlg->m_bConnected = false; break; }
		pt_lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		BOOST_LOG_TRIVIAL(info) << "Not connected to nport" << std::endl;
		return;
	}
	//dlg->m_tcpClientLeft
	
}

void Connect(CVIGUIDlg * dlg)
{
	while (dlg->m_bExecute)
	{
		bool bLeft = false, bRight = false;
		std::thread t1([dlg, &bLeft]() {
			bLeft = dlg->m_tcpClientLeft.Connect();
		});
		std::thread t2([dlg, &bRight]() {
			bRight = dlg->m_tcpClientRight.Connect();
		});
		t1.join();
		t2.join();
		if (bLeft && bRight)
		{
			dlg->m_bConnected = true;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

void CVIGUIDlg::Reconnect()
{
	std::thread t([this]() {
		while(this->m_bExecute)
		{ 
			this->CheckStatus();
			this->EnableDisableButtons();
			wchar_t buf1[100];
			wchar_t buf2[100];
			wsprintf(buf1, L"%d:%d", this->m_nCurrentPanOne, this->m_nRequestedPanOne);
			wsprintf(buf2, L"%d:%d", this->m_nCurrentTiltOne, this->m_nRequestedTiltOne);
			if (m_bConnected)
			{
				int nStatus = this->m_radioConnectionStatus.GetCheck();
				this->m_radioConnectionStatus.SetCheck(1);
				if (nStatus != BST_CHECKED)
					this->m_radioConnectionStatus.Invalidate();
			}
			else 
			{ 
				int nStatus = this->m_radioConnectionStatus.GetCheck();
				this->m_radioConnectionStatus.SetCheck(0);
				if (nStatus != BST_UNCHECKED)
					this->m_radioConnectionStatus.Invalidate();
			}
			this->m_editPanAntenaOne.SetWindowTextW(buf1);
			this->m_editTiltAntenaOne.SetWindowTextW(buf2);
			if (this->m_bConnected)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue; 
			}
			std::thread tConnect([this]()
			{
				Connect(this);
			});
			tConnect.join();

			//std::thread tSend([this]()
			//{
			//	SendCommands(this); 
			//});
			//tSend.detach(); 
			std::thread tReceive([this]()
			{
				ReciveFeedback(this);
			});
			tReceive.detach(); 
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	});
	t.detach();

}

void CVIGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVIGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CVIGUIDlg::OnBnClickedButtonRight()
{
	CheckStatus();
	if (m_bEnableRight) m_nRequestedPanOne += STEP_PAN;
	EnableDisableButtons();
	// TODO: Add your control notification handler code here
}


void CVIGUIDlg::OnBnClickedButtonUp()
{
	CheckStatus();
	if(m_bEnableUp) m_nRequestedTiltOne += STEP_TILT;
	EnableDisableButtons();
	// TODO: Add your control notification handler code here
}


void CVIGUIDlg::OnBnClickedButtonLeft()
{
	CheckStatus();
	if(m_bEnableLeft) m_nRequestedPanOne -= STEP_PAN;
	EnableDisableButtons();
	// TODO: Add your control notification handler code here
}

void CVIGUIDlg::EnableDisableButtons()
{
	CheckStatus();
	GetDlgItem(IDC_BUTTON_RIGHT)->EnableWindow(m_bEnableRight);
	GetDlgItem(IDC_BUTTON_LEFT)->EnableWindow(m_bEnableLeft);
	GetDlgItem(IDC_BUTTON_HOME)->EnableWindow(m_bEnableHome);
	GetDlgItem(IDC_BUTTON_UP)->EnableWindow(m_bEnableUp);
	GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(m_bEnableDown);
	//m_buttonDown.EnableWindow(m_bEnableDown);
	//m_buttonUp.EnableWindow(m_bEnableUp);
	//m_buttonHome.EnableWindow(m_bEnableHome);
	//m_buttonLeft.EnableWindow(m_bEnableLeft);
	//m_buttonRight.EnableWindow(m_bEnableRight);
}

void  CVIGUIDlg::CheckStatus()
{
	if(m_nRequestedTiltOne > MIN_TILT)
		m_bEnableDown = true;
	else 
		m_bEnableDown = false;

	if (m_nRequestedTiltOne < MAX_TILT)
		m_bEnableUp = true;
	else
		m_bEnableUp = false;

	if (m_nRequestedPanOne < MAX_PAN)
		m_bEnableRight = true;
	else
		m_bEnableRight = false;
	if (m_nRequestedPanOne > MIN_PAN)
		m_bEnableLeft = true;
	else
		m_bEnableLeft = false;
}

void CVIGUIDlg::OnBnClickedButtonDown()
{
	CheckStatus();
	if (m_bEnableDown) m_nRequestedTiltOne -= STEP_TILT;
	
	EnableDisableButtons();
	// TODO: Add your control notification handler code here
}

CBrush * CVIGUIDlg::SelectBrush(AntenaStatus &status)
{
	CBrush * tmpBrush = nullptr;
	switch (status)
	{
	case AntenaStatus::Idle:
		tmpBrush = m_pEditBkBrush_Green;
		break;
	case AntenaStatus::Moving:
		tmpBrush = m_pEditBkBrush_Yellow;
		break;
	case AntenaStatus::Error:
		tmpBrush = m_pEditBkBrush_Red;
		break;
	}
	return tmpBrush;
}

HBRUSH CVIGUIDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_PAN_ANTENA_ONE)
	{
		CBrush * tmpBrush = SelectBrush(m_eAntena1PanStatus);
		return (HBRUSH)(tmpBrush->GetSafeHandle());
	}
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_TILT_ANTENA_ONE)
	{
		CBrush * tmpBrush = SelectBrush(m_eAntena1TiltStatus);
		return (HBRUSH)(tmpBrush->GetSafeHandle());
	}
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_TILT_ANTENA_TWO)
	{
		CBrush * tmpBrush = SelectBrush(m_eAntena2TiltStatus);
		return (HBRUSH)(tmpBrush->GetSafeHandle());
	}
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_PAN_ANTENA_TWO)
	{
		CBrush * tmpBrush = SelectBrush(m_eAntena2PanStatus);
		return (HBRUSH)(tmpBrush->GetSafeHandle());
	}
	if (pWnd->GetDlgCtrlID() == IDC_RADIO_CONNECT)
	{
		// Set the text color to red
		if (m_bConnected)
		{
			//m_radioConnectionStatus.SetWindowTextW(L"Connected");
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(0, 255, 0));
		}
		else
		{
			//m_radioConnectionStatus.SetWindowTextW(L"Disconnected"); 
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(255, 0, 0));
		}
		// Set the background mode for text to transparent  
		// so background will show thru.
		

		// Return handle to our CBrush object
		//return (HBRUSH)(m_pEditBkBrush_Pan1->GetSafeHandle());
	}
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


void CVIGUIDlg::OnBnClickedButtonHome()
{
	CheckStatus();
	m_nRequestedTiltOne = 0; 
	m_nRequestedPanOne = 0; 
	m_bEnableDown = 0;
	EnableDisableButtons();
	// TODO: Add your control notification handler code here
}


BOOL CVIGUIDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	m_toolTip.RelayEvent(pMsg);
	return CDialogEx::PreTranslateMessage(pMsg);
}


BOOL CVIGUIDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialogEx::OnNotify(wParam, lParam, pResult);
}

void CVIGUIDlg::RefreshToolTipText()
{
	//m_toolTip.UpdateTipText(L"updated runtime text");
	//m_toolTip.Update();
}

void CVIGUIDlg::OnToolTipTextAboutToShow(NMHDR * pNotifyStruct, LRESULT* result)
{
	if (pNotifyStruct->hwndFrom == m_toolTip.m_hWnd)
		RefreshToolTipText();
}

void CVIGUIDlg::OnUpdateButtonStateRight(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bEnableRight);
}

void CVIGUIDlg::OnUpdateButtonStateLeft(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bEnableLeft);
}

void CVIGUIDlg::OnUpdateButtonStateUp(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bEnableUp);
}

void CVIGUIDlg::OnUpdateButtonStateDown(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bEnableDown);
}

void CVIGUIDlg::OnUpdateButtonStateHome(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bEnableHome);
}

void CVIGUIDlg::OnUpdateButtonStateRadioBtn(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bConnected);
}

void CVIGUIDlg::OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	ASSERT(pPopupMenu != NULL);
	// Check the enabled state of various menu items.

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// Determine if menu is popup in top-level menu and set m_pOther to
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu; // Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue; // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE); // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}


void CVIGUIDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	m_bExecute = false; 
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	CDialogEx::OnClose();
}


void CVIGUIDlg::OnDestroy()
{
	m_bExecute = false;
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
}


// VI_GUIDlg.h : header file
//

#pragma once
#include <string>
#include "TcpClient.h"
#define MAX_PAN 90
#define MIN_PAN -90

#define MAX_TILT 90
#define MIN_TILT 0 

#define STEP_PAN 10 
#define STEP_TILT 10 
#define BUFFER_LENGTH 1024

// CVIGUIDlg dialog

class CVIGUIDlg : public CDialogEx
{
// Construction
public:
	enum AntenaStatus { Idle, Moving, Error };
	CVIGUIDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VI_GUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CBrush* m_pEditBkBrush_Yellow;
	CBrush* m_pEditBkBrush_Green; 
	CBrush* m_pEditBkBrush_Red;
	bool m_bReversePan; 
	CToolTipCtrl m_toolTip;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonRight();
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnBnClickedButtonDown();
	afx_msg void OnBnClickedButtonHome();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void CVIGUIDlg::OnToolTipTextAboutToShow(NMHDR * pNotifyStruct, LRESULT* result);
	afx_msg void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex, BOOL bSysMenu);
private: 
	void EnableDisableButtons();
	void RefreshToolTipText();
	CBrush * SelectBrush(AntenaStatus &status);
	void Reconnect(); 
public:
	AntenaStatus m_eAntena1PanStatus; 
	AntenaStatus m_eAntena1TiltStatus;
	AntenaStatus m_eAntena2PanStatus; 
	AntenaStatus m_eAntena2TiltStatus;
	std::string m_sServerAddrss;
	std::string m_sPortLeft;
	std::string m_sPortRight; 
	TcpClient m_tcpClientLeft; 
	TcpClient m_tcpClientRight; 

	int m_nRequestedPanOne; 
	int m_nRequestedTiltOne;
	int m_nRequestedPanTwo; 
	int m_nRequestedTiltTwo; 
	int m_nCurrentPanTwo; 
	int m_nCurrentTiltTwo; 
	int m_nCurrentPanOne;
	int m_nCurrentTiltOne; 

	CButton m_radioConnectionStatus;
	CEdit m_editPanAntenaOne;
	CEdit m_editTiltAntenaOne;
	CEdit m_editPanAntenaTwo;
	CEdit m_editTiltAntenaTwo;
	bool m_bConnected; 
	bool m_bOnOff; 
	bool m_bEnabled; // for roboteq 
	bool m_bExecute; 
	bool m_bEnableRight, m_bEnableHome, m_bEnableLeft, m_bEnableUp, m_bEnableDown; 

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void CVIGUIDlg::CheckStatus()
/*	CButton m_buttonLeft;
	CButton m_buttonDown;
	CButton m_buttonHome;
	CButton m_buttonUp;
	CButton m_buttonRight*/;
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonOnOff();
	CButton m_btnOnOff;
};

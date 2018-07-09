// OCFDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Colour.h"
#include "Globals.h"


// C_OCFDlg dialog
class C_OCFDlg : public CDialog
{
// Construction
public:
	C_OCFDlg(CWnd* pParent = NULL);	// standard constructor
	~C_OCFDlg() ;

// Dialog Data
	enum { IDD = IDD_OCF_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	void SetupConfiguration( void ) ;
	int SetupStatus( int, int ) ;
	void SetType( int ) ;
	void UpdateStatus( int ) ;
	void UpdateStatusAll( void ) ;
	void OpenResourceManager( void ) ;
	void CloseResourceManager( void ) ;
	void ListResources( void ) ;
	void SetTitle( void ) ;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CWnd	cwndParent[N_WNDS] ;
	CWnd	cwndTraffic[N_WNDS] ;
	CWnd	cwndNetwork[N_WNDS] ;
	CWnd	cwndVISA[N_WNDS] ;

	CColour    csDeviceName[N_WNDS] ;
	CColour    csDeviceVISA[N_WNDS] ;

	CColour    *csDeviceNet[N_WNDS][4] ;

	CColour csDeviceStatus[N_WNDS] ;
	CColour csDeviceNetworked[N_WNDS] ;
	CColour csDeviceDescription[N_WNDS] ;

	CColour    csDeviceLocalPort[N_WNDS] ;
	CColour    csDeviceHostName[N_WNDS] ;
	CColour    csDeviceHostIP[N_WNDS] ;
	CColour    csDeviceHostPort[N_WNDS] ;

	CColour    csDeviceCommand[N_WNDS] ;
	CColour    csDeviceReply[N_WNDS] ;

	CButton m_SelectTraffic;
	CButton m_SelectNetwork;
	CButton m_SelectVISA;

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CString m_str_VISA;
protected:
	CString m_str_Time;
public:
	afx_msg void OnBnClickedSelectTraffic();
	afx_msg void OnBnClickedSelectNetwork();
	afx_msg void OnBnClickedSelectVISA();
	afx_msg void OnBnClickedList();
};

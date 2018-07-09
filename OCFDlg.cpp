// OCFDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Globals.h"
#include "OCF.h"
#include "OCFDlg.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static CFont f_status ;
static CFont f_text ;
static CFont f_title ;
static CFont f_clock ;
static CFont f_bold ;
static CFont f_desc ;

static CPen cPenRed ;
static CPen cPenGreen ;
static CBrush cBrushRed ;
static COLORREF gRed = RGB( 205, 102, 0 ) ;


int iTopDevices = 0 ;

// C_OCFDlg dialog

static void ReadConfiguration( void ) ;
static void SetupConfiguration( void ) ;

C_OCFDlg::C_OCFDlg(CWnd* pParent /*=NULL*/)
	: CDialog(C_OCFDlg::IDD, pParent)
	, m_str_Time(_T(""))
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    for( int iD = 0 ; iD < N_DEVS ; iD++ ) {
	InitializeDeviceStruct( iD ) ;
    }

    ReadConfiguration() ;

    cPenRed.CreatePen( PS_SOLID, 1, RGB( 238, 80, 80 ) ) ;
    cPenGreen.CreatePen( PS_SOLID, 1, RGB( 0, 128, 0 ) ) ;
    cBrushRed.CreateSolidBrush( RGB( 238, 80, 80 ) ) ;

    f_text.CreateFont(
	16, 0, 0, 0, FW_NORMAL,
	0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, "Courier New" );

    f_status.CreateFont(
	14, 0, 0, 0, FW_DONTCARE,
	0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, "Courier New" );

    f_title.CreateFont(
	48, 0, 0, 0, FW_BOLD,
	0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_ROMAN, "Times New Roman" );

    f_clock.CreateFont(
	14, 0, 0, 0, FW_DONTCARE,
	0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, "Courier New" );

    f_bold.CreateFont(
	16, 0, 0, 0, FW_BOLD,
	0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, "Courier New" );

    f_desc.CreateFont(
	14, 0, 0, 0, FW_DONTCARE,
	TRUE, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, "Courier New" );

}

void C_OCFDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TIME, m_str_Time);
	DDX_Control(pDX, IDC_SELECT_TRAFFIC, m_SelectTraffic ) ;
	DDX_Control(pDX, IDC_SELECT_NETWORK, m_SelectNetwork ) ;
	DDX_Control(pDX, IDC_SELECT_VISA, m_SelectVISA ) ;
}

C_OCFDlg::~C_OCFDlg()
{
    int i, j ;

    BeginWaitCursor() ;

    bRunning = FALSE ;
    Sleep( 5000 ) ;

    CloseResourceManager() ;

    for( i = 0 ; i < iNDevs ; i++ ) {
	if( s_Devices[i].cDevice != NULL ) {
	    free( s_Devices[i].cDevice ) ;
	    s_Devices[i].cDevice = NULL ;
	}

	if( s_Devices[i].cName != NULL ) {
	    free( s_Devices[i].cName ) ;
	    s_Devices[i].cName = NULL ; 
	}

	if( s_Devices[i].cDescription != NULL ) {
	    free( s_Devices[i].cDescription ) ;
	    s_Devices[i].cDescription = NULL ; 
	}

	if( s_Devices[i].cVISA != NULL ) {
	    free( s_Devices[i].cVISA ) ;
	    s_Devices[i].cVISA = NULL ; 
	}
	if( s_Devices[i] . bEnabled == FALSE ) continue ;
    }

    for( i = 0 ; i < iNWnds ; i++ ) {
	for( j = 0 ; j < 4 ; j++ ) {
	    delete csDeviceNet[i][j] ;
	}
    }

    EndWaitCursor() ;
}

BEGIN_MESSAGE_MAP(C_OCFDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SELECT_TRAFFIC, &C_OCFDlg::OnBnClickedSelectTraffic)
	ON_BN_CLICKED(IDC_SELECT_NETWORK, &C_OCFDlg::OnBnClickedSelectNetwork)
	ON_BN_CLICKED(IDC_SELECT_VISA, &C_OCFDlg::OnBnClickedSelectVISA)
	ON_BN_CLICKED(IDC_LIST, &C_OCFDlg::OnBnClickedList)
END_MESSAGE_MAP()


// C_OCFDlg message handlers

void C_OCFDlg::SetTitle( void )
{
    char *sTitle = "OCF Device Network Server" ;
    CDC *pDC = GetDC() ;
    if( pDC == NULL ) {
	GetDlgItem( IDC_TITLE ) -> SetWindowText( "failed" ) ;
	return ;
    }
    CFont *of = pDC->SelectObject( &f_title ) ;
    CSize csStrSize = pDC->GetTextExtent( sTitle, strlen(sTitle) ) ;
    pDC->SelectObject( of ) ;

    CRect crTitle;
    GetDlgItem( IDC_TITLE ) -> GetWindowRect( &crTitle ) ;
    ScreenToClient( &crTitle ) ;

    CRect crParent;
    GetClientRect(&crParent);

    int x = (crParent.right - crParent.left) / 2 - csStrSize.cx / 2 ;
    int y = crTitle.top ;

    GetDlgItem( IDC_TITLE ) -> SetFont( &f_title ) ;
    GetDlgItem( IDC_TITLE ) -> SetWindowText( sTitle ) ;

    GetDlgItem( IDC_TITLE ) -> MoveWindow( x, y, csStrSize.cx, csStrSize.cy, TRUE ) ;
    GetDlgItem( IDC_TITLE ) -> GetWindowRect( &crTitle ) ;
    ScreenToClient( &crTitle ) ;
    iTopDevices = crTitle.bottom ;
}

BOOL C_OCFDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // ShowWindow(SW_MAXIMIZE);
    SetTitle() ;
    GetDlgItem( IDC_TIME ) -> SetFont( &f_clock ) ;

    bRunning = TRUE ;

    m_SelectTraffic.SetCheck( 0 ) ;
    m_SelectNetwork.SetCheck( 0 ) ;
    m_SelectVISA.SetCheck( 1 ) ;

    SetupConfiguration() ;

    SetTimer( 1, iTiming, NULL ) ;

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void C_OCFDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR C_OCFDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


static void ReadConfiguration( void )
{
    char valString[65536] ;
    char filename[MAX_PATH] ;
    char sCurrWD[MAX_PATH] ;
    int i ;
    char *p ;
    DWORD dwTemp ;

    char cEnableDefault[MAX_PATH] ;
    char cTimeoutDefault[MAX_PATH] ;
    char cRxFlushDefault[MAX_PATH] ;
    char cRxReadCharsDefault[MAX_PATH] ;
    char cRxTermCharsDefault[MAX_PATH] ;
    u_char ucTxWriteOnlyCharDefault[MAX_PATH] ;
    char cTxAppendCharsDefault[MAX_PATH] ;
    char cBaudRateDefault[MAX_PATH] ;
    char cDelayAfterWriteDefault[MAX_PATH] ;
    char cBinaryDefault[MAX_PATH] ;
    char cMaxErrorsDefault[MAX_PATH] ;

    _getcwd( sCurrWD, MAX_PATH ) ;

    // Devices.ini file is always kept in the Applications
    // starting directory. Through a shortcut, this does not
    // need to be the same place as the executeable.

    _getcwd( filename, MAX_PATH ) ;
    strncat_s( filename, "\\Devices.ini", MAX_PATH ) ;

    DWORD profile_status ;
    // LPSTR p_str ;

    // We want the list of all the blocks.
    profile_status = GetPrivateProfileString( NULL, NULL, NULL, valString, sizeof(valString), filename ) ;
    if( profile_status <= 0 ) {
	Alert( "Could not open Devices.ini file." ) ;
	return ;
    }

    p = valString ;
    for( iNDevs = 0 ; iNDevs < N_DEVS ; ) {
	if( strlen( p ) == 0 ) break ;
	if( _stricmp( p, "default" ) != 0 ) {
            if( s_Devices[iNDevs].cDevice ) free( s_Devices[iNDevs].cDevice ) ;
            s_Devices[iNDevs].cDevice = _strdup( p ) ;
            if( s_Devices[iNDevs].cDevice == NULL ) break ;
	    iNDevs++ ;
	}
	p = p + strlen(p) + 1 ;
    }

    profile_status = GetPrivateProfileString( "Default", "Enable", "0", cEnableDefault, sizeof(cEnableDefault), filename ) ;
    profile_status = GetPrivateProfileString( "Default", "BaudRate", "9600", cBaudRateDefault, sizeof(cBaudRateDefault), filename ) ;
    profile_status = GetPrivateProfileString( "Default", "RxFlush", "0", cRxFlushDefault, sizeof(cRxFlushDefault), filename ) ;
    profile_status = GetPrivateProfileString( "Default", "TxAppendChars", "0x0a", cTxAppendCharsDefault, sizeof(cTxAppendCharsDefault), filename ) ;
    profile_status = GetPrivateProfileString( "Default", "DelayAfterWrite", "0", cDelayAfterWriteDefault, sizeof(cDelayAfterWriteDefault), filename ) ;
    profile_status = GetPrivateProfileString( "Default", "RxReadChars", "0", cRxReadCharsDefault, sizeof(cRxReadCharsDefault), filename ) ;
    profile_status = GetPrivateProfileString( "Default", "RxTermChars", "0", cRxTermCharsDefault, sizeof(cRxTermCharsDefault), filename ) ;
    profile_status = GetPrivateProfileString( "Default", "TxWriteOnlyChar", "/", (char*)ucTxWriteOnlyCharDefault, sizeof(ucTxWriteOnlyCharDefault), filename ) ;
    profile_status = GetPrivateProfileString( "Default", "Timeout", "2000", cTimeoutDefault, sizeof(cTimeoutDefault), filename ) ;
    profile_status = GetPrivateProfileString( "Default", "Binary", "0", cBinaryDefault, sizeof(cBinaryDefault), filename ) ;
    profile_status = GetPrivateProfileString( "Default", "MaxErrors", "4", cMaxErrorsDefault, sizeof(cMaxErrorsDefault), filename ) ;

    for( i = 0 ; i < iNDevs ; i++ ) {
	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "Name", "", valString, sizeof(valString), filename ) ;
	s_Devices[i].cName = _strdup( valString ) ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "Description", "", valString, sizeof(valString), filename ) ;
	s_Devices[i].cDescription = _strdup( valString ) ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "VISA", "", valString, sizeof(valString), filename ) ;
	s_Devices[i].cVISA = _strdup( valString ) ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "Port", "0", valString, sizeof(valString), filename ) ;
	s_Devices[i].iPort = strtol( valString, NULL, 0 ) ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "Enable", cEnableDefault, valString, sizeof(valString), filename ) ;
	s_Devices[i].bEnabled = strtol( valString, NULL, 0 ) ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "BaudRate", cBaudRateDefault, valString, sizeof(valString), filename ) ;
	s_Devices[i].iBaudRate = strtol( valString, NULL, 0 ) ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "RxFlush", cRxFlushDefault, valString, sizeof(valString), filename ) ;
	s_Devices[i].bRxFlush = strtol( valString, NULL, 0 ) ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "TxAppendChars", cTxAppendCharsDefault, valString, sizeof(valString), filename ) ;
	dwTemp = strtoul( valString, NULL, 0 ) ;
	memcpy( s_Devices[i].cTxAppendChars, (void*)&dwTemp, 4 ) ;
	s_Devices[i].lenTxAppendChars = strnlen( s_Devices[i].cTxAppendChars, 4 ) ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "TxWriteOnlyChar", (char*)ucTxWriteOnlyCharDefault, valString, sizeof(valString), filename ) ;
	if( strlen(valString) == 0 ) {
	    s_Devices[i].ucTxWriteOnlyChar = 0 ;
	} else if( strlen(valString) == 1 ) {
	    s_Devices[i].ucTxWriteOnlyChar = valString[0] ;
	} else {
	    dwTemp = strtoul( valString, NULL, 0 ) ;
	    s_Devices[i].ucTxWriteOnlyChar = (u_char)(0xFF & dwTemp) ;
	}

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "DelayAfterWrite", cDelayAfterWriteDefault, valString, sizeof(valString), filename ) ;
	s_Devices[i].iDelayAfterWrite = strtol( valString, NULL, 0 ) ;
	if( s_Devices[i].iDelayAfterWrite < 0 ) s_Devices[i].iDelayAfterWrite = 100 ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "Timeout", cTimeoutDefault, valString, sizeof(valString), filename ) ;
	s_Devices[i].iTimeOut = strtol( valString, NULL, 0 ) ;
	if( s_Devices[i].iTimeOut <= 0 ) s_Devices[i].iTimeOut = 100 ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "RxReadChars", cRxReadCharsDefault, valString, sizeof(valString), filename ) ;
	s_Devices[i].iRxReadChars = strtol( valString, NULL, 0 ) ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "RxTermChars", cRxTermCharsDefault, valString, sizeof(valString), filename ) ;
	dwTemp = strtoul( valString, NULL, 0 ) ;
	memcpy( s_Devices[i].cRxTermChars, (void*)&dwTemp, 4 ) ;
	s_Devices[i].iRxTrimChars = strnlen( s_Devices[i].cRxTermChars, 4 ) ;

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "Binary", cBinaryDefault, valString, sizeof(valString), filename ) ;
	s_Devices[i].bBinary = (strtol( valString, NULL, 0 ) != 0);

	profile_status = GetPrivateProfileString( s_Devices[i].cDevice, "MaxErrors", cMaxErrorsDefault, valString, sizeof(valString), filename ) ;
	s_Devices[i].iMaxErrors = strtol( valString, NULL, 0 ) ;
    }
}

void C_OCFDlg::OpenResourceManager( void )
{
    if( ! bRMInitialized ) {
	viOpenDefaultRM( &DefaultRM ) ;
    }
    bRMInitialized = TRUE ;
}

void C_OCFDlg::CloseResourceManager( void )
{
    if( bRMInitialized ) {
	viClose( DefaultRM ) ;
    }
    bRMInitialized = FALSE ;
}

#define D_MAX_LIST_N	(1024)
#define D_MAX_LIST_B	(16384)

static int iNList = 0 ;
static int *iListPtrs ;
static int iList[D_MAX_LIST_N] ;
static char *xTmp  ;

static int scmp( const void *vin1, const void *vin2 )
{
    int in1 = *(int*)vin1 ;
    int in2 = *(int*)vin2 ;


    return( strcmp( xTmp + iListPtrs[in1], xTmp + iListPtrs[in2] ) ) ;
}

void C_OCFDlg::ListResources( void )
{
    ViFindList vfList ;
    ViUInt32 vuiReturnCount ;
    ViRsrc vrDescriptor ;
    ViString vsExpr = ViString( "?*" ) ;
    ViStatus viStatus ;

    vrDescriptor = (ViRsrc)malloc( D_MAX_LIST_B ) ;

    xTmp = (char*)malloc( D_MAX_LIST_B ) ;
    iListPtrs = (int*)malloc( D_MAX_LIST_N * sizeof(int*) ) ;

    viStatus = viFindRsrc( DefaultRM, vsExpr, &vfList, &vuiReturnCount, (ViPRsrc)vrDescriptor ) ;
    if( viStatus != VI_SUCCESS )
	return ;

    int M = 0 ;
    iNList = 0 ;

    while( 1 ) {
	if( iNList >= D_MAX_LIST_N )
	    break ;

	iListPtrs[iNList] = M ;
	iList[iNList] = iNList ;
	iNList++ ;

	M += sprintf_s( xTmp+M, 16384-M, "%s", vrDescriptor) ;
	xTmp[M] = '\0' ; M++ ;
	
	viStatus = viFindNext( vfList, (ViPRsrc)vrDescriptor ) ;
	if( viStatus != VI_SUCCESS ) break ;
    }
    viClose( vfList ) ;
    free( vrDescriptor ) ;

    qsort( iList, iNList, sizeof(int), scmp ) ;

    LPSTR p_str = m_str_VISA.GetBuffer( 16384 ) ;
    M = 0 ;
    M += sprintf_s( p_str+M, 16384-M, "Devices found = %d\n", vuiReturnCount ) ;

    int i ;
    for( i = 0 ; i < iNList ; i++ ) {
	int n = iList[i] ;
	M += sprintf_s( p_str+M, 16384-M, "%s\n", xTmp + iListPtrs[n] ) ;
    }
    m_str_VISA.ReleaseBuffer() ;
    free( iListPtrs ) ;
    free( xTmp ) ;

}

void C_OCFDlg::SetupConfiguration( void )
{
    int iD ;

    OpenResourceManager() ;
    ListResources() ;

    int iW = 0 ;
    int iMax = SetupStatus( -1, iW ) ; iW++ ;

    for( iD = 0 ; iD < iNDevs ; iD++ ) {
	s_Devices[iD].iDevice = iD ;
	s_Devices[iD].hMutex = CreateMutex( NULL, FALSE, NULL ) ;

	SetType( iD ) ;

	if( s_Devices[iD] . bEnabled == FALSE ) continue ;
	iMax = SetupStatus( iD, iW ) ; iW++ ;

        s_Devices[iD].hThreadN = CreateThread(
            NULL,
            0,
            &fThread_N,
            &(s_Devices[iD] . iDevice),
            0,
            &(s_Devices[iD] . dwThreadID)
	    ) ;
    }
    iNWnds = iW ;

    RECT rect ;
    this->GetWindowRect( &rect ) ;

    rect.bottom = iMax + 50 ;
    this->MoveWindow( &rect ) ;


}
void C_OCFDlg::SetType( int iD )
{
    if( _strnicmp( s_Devices[iD] . cVISA, "ASRL", 4 ) == 0 ) {
	s_Devices[iD] . eType = eSerial ;
    } else if( _strnicmp( s_Devices[iD] . cVISA, "GPIB", 4 ) == 0 ) {
	s_Devices[iD] . eType = eGPIB ;
    } else {
	s_Devices[iD] . eType = eUnknown ;
    }
}

void C_OCFDlg::UpdateStatusAll( void )
{
    int iD ;

    for( iD = 0 ; iD < iNDevs ; iD++ ) {
	if( s_Devices[iD] . bEnabled == FALSE ) continue ;
	UpdateStatus( iD ) ;
    }

    int iW ;
    for( iW = 0 ; iW < iNWnds ; iW++ ) {
    if( m_SelectTraffic.GetCheck() ) {
	cwndTraffic[iW] . ShowWindow( SW_SHOW ) ;
	cwndNetwork[iW] . ShowWindow( SW_HIDE ) ;
	cwndVISA[iW] . ShowWindow( SW_HIDE ) ;
    } else if( m_SelectNetwork.GetCheck() ) {
	cwndTraffic[iW] . ShowWindow( SW_HIDE ) ;
	cwndNetwork[iW] . ShowWindow( SW_SHOW ) ;
	cwndVISA[iW] . ShowWindow( SW_HIDE ) ;
    } else if ( m_SelectVISA.GetCheck() ) {
	cwndTraffic[iW] . ShowWindow( SW_HIDE ) ;
	cwndNetwork[iW] . ShowWindow( SW_HIDE ) ;
	cwndVISA[iW] . ShowWindow( SW_SHOW ) ;
    } else {
	cwndTraffic[iW] . ShowWindow( SW_HIDE ) ;
	cwndNetwork[iW] . ShowWindow( SW_HIDE ) ;
	cwndVISA[iW] . ShowWindow( SW_HIDE ) ;
    }
    }

}

void C_OCFDlg::UpdateStatus( int iD )
{
    char temp[256] ;

    if( s_Devices[iD] . bEnabled == FALSE )
	return ;

    WaitForSingleObject(  s_Devices[iD] . hMutex, INFINITE ) ;
    int iW = s_Devices[iD] . iWND ;

    if( s_Devices[iD] . iOpened == 0 ) {
	csDeviceStatus[iW] . SetTextColor( RGB(0,0,0) ) ;
	csDeviceStatus[iW] . SetWindowText( " Not Opened" )  ;
    } else if( s_Devices[iD] . iOpened == 1 ) {
	csDeviceStatus[iW] . SetTextColor( RGB(0,200,0) ) ;
	csDeviceStatus[iW] . SetWindowText( " Device Available " )  ;
    } else if( s_Devices[iD] . iOpened == 2 ) {
	csDeviceStatus[iW] . SetTextColor( RGB(0,200,200) ) ;
	csDeviceStatus[iW] . SetWindowText( " Device Offline?" )  ;
    } else {
	csDeviceStatus[iW] . SetTextColor( RGB(210,0,0) ) ;
	csDeviceStatus[iW] . SetWindowText( " Open Failed" )  ;
    }

    if( s_Devices[iD] . bConnected ) {
	csDeviceNetworked[iW] . SetTextColor( RGB(0,200,0) ) ;
	csDeviceNetworked[iW] . SetWindowText( " Networked" )  ;
    } else {
	csDeviceNetworked[iW] . SetTextColor( RGB(0,0,200) ) ;
	csDeviceNetworked[iW] . SetWindowText( " Not Networked" )  ;
    }

    sprintf_s( temp, sizeof(temp), "%10d", s_Devices[iD] . ulNetworkRx ) ;
    csDeviceNet[iW][0]->SetWindowText( temp ) ;
    sprintf_s( temp, sizeof(temp), "%10d", s_Devices[iD] . ulDeviceTx ) ;
    csDeviceNet[iW][1]->SetWindowText( temp ) ;
    sprintf_s( temp, sizeof(temp), "%10d", s_Devices[iD] . ulDeviceRx ) ;
    csDeviceNet[iW][2]->SetWindowText( temp ) ;
    sprintf_s( temp, sizeof(temp), "%10d", s_Devices[iD] . ulNetworkTx ) ;
    csDeviceNet[iW][3]->SetWindowText( temp ) ;

    csDeviceHostName[iW] . SetWindowText( s_Devices[iD] . cHostName ) ;
    csDeviceHostIP[iW] . SetWindowText( s_Devices[iD] . cHostIP ) ;

    sprintf_s( temp, sizeof(temp), "%6d", s_Devices[iD] . iHostPort ) ;
    csDeviceHostPort[iW] . SetWindowText( temp ) ;

    csDeviceCommand[iW] . SetWindowText( s_Devices[iD].cCmdDisplay ) ;
    csDeviceReply[iW] . SetWindowText( s_Devices[iD].cReplyDisplay ) ;

    ReleaseMutex(  s_Devices[iD] . hMutex ) ;
}


static POINT cpStatus[] = {
{ 0, 0 },
} ;

static int iId = 9000 ;

int C_OCFDlg::SetupStatus( int iD, int iW )
{
    CHAR tmp[1024] ;
    int ht = 20 ;
    int x, x0 = 10 ;
    int y, y0 = iTopDevices + 30 ;
    int txtwd = 250 ;
    int x_0, y_0 ;
    int iBaseID ;

    x_0 = x0 ;
    y_0 = y0 + iW * ht + iW * 5 ;

    x = x_0 ;
    y = y_0 ;

    iBaseID = iId ;

    DWORD dwStyle = WS_BORDER | WS_CHILD ;
    RECT rect ;

    rect.left = x_0 ;
    rect.top = y_0 ;

    rect.right = x_0 + 1004 + 60 ;
    rect.bottom = y_0 + ht ;

    cwndParent[iW] . Create( NULL, "Window", dwStyle, rect, this, iId++, NULL ) ;
    cwndParent[iW] . ShowWindow( SW_SHOW ) ;
    CWnd *pMain = &cwndParent[iW] ;

    x_0 = 0 ;
    y_0 = 0 ;

    x = 0 ;
    y = 0 ;
    txtwd = 200 ;
    csDeviceName[iW].Create( _T(" Device Name"), WS_CHILD | SS_SUNKEN | SS_LEFTNOWORDWRAP | SS_CENTERIMAGE | SS_NOTIFY , CRect(x, y, x+txtwd, y+ht ), pMain, iId++ ) ;
    csDeviceName[iW].SetFont( &f_text ) ;
    csDeviceName[iW].ShowWindow( SW_SHOW ) ;
    csDeviceName[iW].SetToolTip( "Device Description" ) ;

    x = x + txtwd ;
    csDeviceVISA[iW].Create( _T(" VISA Address"), WS_CHILD | SS_SUNKEN | SS_CENTERIMAGE | SS_LEFT| SS_NOTIFY , CRect(x, y, x+120, y+ht ), pMain, iId++ ) ;
    csDeviceVISA[iW].SetFont( &f_text ) ;
    csDeviceVISA[iW].ShowWindow( SW_SHOW ) ;
    csDeviceVISA[iW].SetToolTip( "VISA Device Name" ) ;

    x = x + 120 ;
    csDeviceStatus[iW].Create( _T(" VISA Status"), WS_CHILD | SS_SUNKEN | SS_LEFT | SS_CENTERIMAGE | SS_NOTIFY , CRect(x, y, x+130, y+ht ), pMain, iId++ ) ;
    csDeviceStatus[iW].SetFont( &f_text ) ;
    csDeviceStatus[iW].ShowWindow( SW_SHOW ) ;
    csDeviceStatus[iW].SetToolTip( "VISA Open Status" ) ;

    x = x + 130 ;
    csDeviceNetworked[iW].Create( _T(" Network Status"), WS_CHILD | SS_SUNKEN | SS_LEFT | SS_CENTERIMAGE | SS_NOTIFY , CRect(x, y, x+130, y+ht ), pMain, iId++ ) ;
    csDeviceNetworked[iW].SetFont( &f_text ) ;
    csDeviceNetworked[iW].ShowWindow( SW_SHOW ) ;
    csDeviceNetworked[iW].SetToolTip( "Network Connection Status" ) ;

    int x1 = x + 130 ;
    int y1 = y ;
    int selectwidth = 480 ;

    rect.left = x1 ;
    rect.top = y1 ;
    rect.right = x1 + selectwidth ;
    rect.bottom = y1 + ht;
    dwStyle = WS_CHILD ;

    cwndTraffic[iW] . Create( NULL, "Window", dwStyle, rect, pMain, iId++, NULL ) ;
    cwndTraffic[iW] . ShowWindow( SW_HIDE ) ;
    CWnd *pThis = &cwndTraffic[iW] ;

    x = 0 ; y = 0 ;
    int i ;
    int hx = x ;
    int hy = y ;
    for( i = 0 ; i < 4 ; i++ ) {
	csDeviceNet[iW][i] = new CColour() ;
	x = hx + i * 120 ;
	y = hy ;
	csDeviceNet[iW][i]->Create( NULL, WS_CHILD | SS_SUNKEN | SS_CENTERIMAGE | SS_RIGHT| SS_NOTIFY , CRect(x, y, x+120, y+ht ), pThis, iId++ ) ;
	csDeviceNet[iW][i]->SetFont( &f_status ) ;
	csDeviceNet[iW][i]->ShowWindow( SW_SHOW ) ;
    }
    csDeviceNet[iW][0]->SetWindowText( "Network RX B " ) ;
    csDeviceNet[iW][1]->SetWindowText( "Device TX B " ) ;
    csDeviceNet[iW][2]->SetWindowText( "Device RX B " ) ;
    csDeviceNet[iW][3]->SetWindowText( "Network TX B " ) ;

    csDeviceNet[iW][0]->SetToolTip( "Bytes RX from Network" ) ;
    csDeviceNet[iW][1]->SetToolTip( "Bytes TX to Device" ) ;
    csDeviceNet[iW][2]->SetToolTip( "Bytes RX from Device" ) ;
    csDeviceNet[iW][3]->SetToolTip( "Bytes TX to Network" ) ;


    rect.left = x1 ;
    rect.top = y1 ;
    rect.right = x1 + selectwidth ;
    rect.bottom = y1 + ht;
    dwStyle = WS_CHILD ;

    cwndNetwork[iW] . Create( NULL, "Window", dwStyle, rect, pMain, iId++, NULL ) ;
    cwndNetwork[iW] . ShowWindow( SW_HIDE ) ;
    pThis = &cwndNetwork[iW] ;
    x = 0 ; y = 0 ;

    csDeviceLocalPort[iW].Create( _T("Local P"), WS_CHILD | SS_SUNKEN | SS_CENTERIMAGE | SS_RIGHT| SS_NOTIFY , CRect(x, y, x+60, y+ht ), pThis, iId++ ) ;
    csDeviceLocalPort[iW].SetFont( &f_status ) ;
    csDeviceLocalPort[iW].ShowWindow( SW_SHOW ) ;
    csDeviceLocalPort[iW].SetToolTip( "Server's TCP Port Number for Device" ) ;

    x = x + 60 ;
    csDeviceHostName[iW].Create( _T(" Remote Host"), WS_CHILD | SS_SUNKEN | SS_LEFTNOWORDWRAP | SS_CENTERIMAGE| SS_NOTIFY , CRect(x, y, x+180, y+ht ), pThis, iId++ ) ;
    csDeviceHostName[iW].SetFont( &f_status ) ;
    csDeviceHostName[iW].ShowWindow( SW_SHOW ) ;
    csDeviceHostName[iW].SetToolTip( "Name of Connected Machine" ) ;

    x = x + 180 ;
    csDeviceHostIP[iW].Create( _T(" Remote IP"), WS_CHILD | SS_SUNKEN | SS_LEFTNOWORDWRAP | SS_CENTERIMAGE| SS_NOTIFY , CRect(x, y, x+180, y+ht ), pThis, iId++ ) ;
    csDeviceHostIP[iW].SetFont( &f_status ) ;
    csDeviceHostIP[iW].ShowWindow( SW_SHOW ) ;
    csDeviceHostIP[iW].SetToolTip( "IP of Connected Machine" ) ;

    x = x + 180 ;
    csDeviceHostPort[iW].Create( _T(" Remote P"), WS_CHILD | SS_SUNKEN | SS_CENTERIMAGE | SS_RIGHT| SS_NOTIFY , CRect(x, y, x+60, y+ht ), pThis, iId++ ) ;
    csDeviceHostPort[iW].SetFont( &f_status ) ;
    csDeviceHostPort[iW].ShowWindow( SW_SHOW ) ;
    csDeviceHostPort[iW].SetToolTip( "TCP Port on Connected Machine" ) ;

    rect.left = x1 ;
    rect.top = y1 ;
    rect.right = x1 + selectwidth ;
    rect.bottom = y1 + ht;
    dwStyle = WS_CHILD ;

    cwndVISA[iW] . Create( NULL, "Window", dwStyle, rect, pMain, iId++, NULL ) ;
    cwndVISA[iW] . ShowWindow( SW_HIDE ) ;
    pThis = &cwndVISA[iW] ;
    x = 0 ; y = 0 ;

    csDeviceCommand[iW].Create( _T(" Command Sent"), WS_CHILD | SS_SUNKEN | SS_LEFTNOWORDWRAP | SS_CENTERIMAGE| SS_NOTIFY , CRect(x, y, x+200, y+ht ), pThis, iId++ ) ;
    csDeviceCommand[iW].SetFont( &f_status ) ;
    csDeviceCommand[iW].ShowWindow( SW_SHOW ) ;
    csDeviceCommand[iW].SetToolTip( "Latest Command Sent" ) ;

    x = x + 200 ;
    csDeviceReply[iW].Create( _T(" Response"), WS_CHILD | SS_SUNKEN | SS_LEFTNOWORDWRAP | SS_CENTERIMAGE| SS_NOTIFY , CRect(x, y, x+280, y+ht ), pThis, iId++ ) ;
    csDeviceReply[iW].SetFont( &f_status ) ;
    csDeviceReply[iW].ShowWindow( SW_SHOW ) ;
    csDeviceReply[iW].SetToolTip( "Latest Response Received" ) ;


    if( iD >= 0 ) {
	s_Devices[iD] . iWND = iW ;
	s_Devices[iD] . iBaseID = iBaseID ;
	csDeviceName[iW].SetWindowText( s_Devices[iD] . cName ) ;
	csDeviceName[iW].SetToolTip( s_Devices[iD] . cDescription ) ;
	csDeviceVISA[iW].SetWindowText( s_Devices[iD] . cVISA ) ;
	csDeviceNetworked[iW].SetFont( &f_bold ) ;
	csDeviceStatus[iW].SetFont( &f_bold ) ;
	sprintf_s( tmp, sizeof(tmp), "%6d", s_Devices[iD] . iPort ) ;
	csDeviceLocalPort[iW].SetWindowText( tmp ) ;
    }

    return( iTopDevices + 30 + iW * ht + iW * 5 + ht ) ;
}

#if QUERY_IDN_MAYBE_NOT

void C_OCFDlg::OnBnClickedButton1()
{
    /*idn.c
    This example program queries a GPIB device for an identification string 
    and prints the results. Note that you must change the address. */

    ViSession defaultRM, vi;
    char buf [256] = {0};

    viOpenDefaultRM (&defaultRM);

    int i ;
    for( i = 0 ; i < iNDevs ; i++ ) {
		        
	if( s_Devices[i] . bEnabled == FALSE ) continue ;
	if( strstr( s_Devices[i] . cVISA, "GPIB" ) == 0 ) continue ;

	viOpen (defaultRM, s_Devices[i] . cVISA, VI_NULL, VI_NULL, &vi ) ;

	/* Send an *IDN? string to the device */
	viPrintf (vi, "*IDN?\n");

	/* Read results */
	viScanf (vi, "%t", &buf);

	strcpy_s( s_Devices[i] . cIDN, sizeof(s_Devices[i].cIDN), buf ) ;

	/* Close session */
	viClose (vi);

    }
    viClose (defaultRM);

    /*
    LPSTR p_str = m_str_IDN.GetBuffer( 128 ) ;
    sprintf_s( p_str, 128, "Instrument=%s", buf ) ;
    m_str_IDN.ReleaseBuffer() ;
    */
    UpdateData( FALSE ) ;
}

#endif // QUERY_IDN_MAYBE_NOT

void SetupTime( char *pStr, int iMaxStr )
{
    struct tm tm_utc, tm_local ;
    static time_t now, last = 0, first = 0 ;
    int m ;

    time(&now) ;

    if( first == 0 ) 
	first = now ;

    if( last == now )
	return ;
    last = now ;

    gmtime_s( &tm_utc, &now ) ;
    localtime_s( &tm_local, &now ) ;

    m = 0 ;
    m += sprintf_s( pStr+m, iMaxStr-m,
        "UTC: %4d-%02d-%02d %02d:%02d:%02d\n",
        1900+tm_utc.tm_year, 
        tm_utc.tm_mon+1,
        tm_utc.tm_mday,
        tm_utc.tm_hour,
        tm_utc.tm_min,
        tm_utc.tm_sec ) ;

    m += sprintf_s( pStr+m, iMaxStr-m,
        "%s: %4d-%02d-%02d %02d:%02d:%02d\n",
	"Lcl",
        1900+tm_local.tm_year, 
        tm_local.tm_mon+1,
        tm_local.tm_mday,
        tm_local.tm_hour,
        tm_local.tm_min,
        tm_local.tm_sec ) ;

    int iUptime = (int)(now - first) ;
    int iUd, iUh, iUm, iUs ;
    iUd = iUptime / 86400 ;
    iUptime = iUptime - iUd * 86400 ;
    iUh = iUptime / 3600 ;
    iUptime = iUptime - iUh * 3600 ;
    iUm = iUptime / 60 ;
    iUs = iUptime - iUm * 60 ;

    m += sprintf_s( pStr+m, iMaxStr-m,
        "%s:    %4d:%02d:%02d:%02d",
	"Uptime",
	iUd, iUh, iUm, iUs ) ;
}

void C_OCFDlg::OnTimer(UINT_PTR nIDEvent)
{
    // CDialog::OnTimer(nIDEvent);
    UpdateStatusAll() ;

    LPSTR p_str = m_str_Time.GetBuffer( 256 ) ;
    SetupTime( p_str, 256 ) ;
    m_str_Time.ReleaseBuffer() ;

    UpdateData( FALSE ) ;
}

void C_OCFDlg::OnBnClickedSelectTraffic()
{
	m_SelectTraffic.SetCheck( 1 ) ;
	m_SelectNetwork.SetCheck( 0 ) ;
	m_SelectVISA.SetCheck( 0 ) ;
}

void C_OCFDlg::OnBnClickedSelectNetwork()
{
	m_SelectTraffic.SetCheck( 0 ) ;
	m_SelectNetwork.SetCheck( 1 ) ;
	m_SelectVISA.SetCheck( 0 ) ;
}

void C_OCFDlg::OnBnClickedSelectVISA()
{
	m_SelectTraffic.SetCheck( 0 ) ;
	m_SelectNetwork.SetCheck( 0 ) ;
	m_SelectVISA.SetCheck( 1 ) ;
}

void C_OCFDlg::OnBnClickedList()
{
    BeginWaitCursor() ;
    ListResources() ;
    EndWaitCursor() ;
    MessageBox( m_str_VISA, "Visa Device List", MB_OK ) ;
}

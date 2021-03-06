#include "StdAfx.h"
#include <afxsock.h>            // MFC socket extensions
#include <direct.h>            // MFC socket extensions
#include <stdio.h>
#include <winsock2.h>
#include "Globals.h"

#define FLOG(a) (s_Devices[a].fLog)

static void CreateSocketAndBind( int iDEVICE )
{
    SOCKET fdbind = 0 ;

    fdbind = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
    if( fdbind < 0 ) {
	s_Devices[iDEVICE].fdbind = -1 ;
	return ;
    }

    BOOL bTCPNODELAY = TRUE ;
    // Could decrease delays. Worth a try.
    setsockopt(
	fdbind,
	IPPROTO_TCP,
	TCP_NODELAY,
	(char*)&bTCPNODELAY,
	sizeof(bTCPNODELAY)
    ) ;

    BOOL bCONDITIONAL_ACCEPT = TRUE ;
    setsockopt(
	fdbind,
	SOL_SOCKET,
	SO_CONDITIONAL_ACCEPT,
	(char*)&bCONDITIONAL_ACCEPT,
	sizeof(bCONDITIONAL_ACCEPT)
    ) ;

    BOOL bKEEP_ALIVE = TRUE ;
    setsockopt(
	fdbind,
	SOL_SOCKET,
	SO_KEEPALIVE,
	(char*)&bKEEP_ALIVE,
	sizeof(bKEEP_ALIVE)
    ) ;

    BOOL bREUSE = TRUE ;
    setsockopt(
	fdbind,
	SOL_SOCKET,
	SO_REUSEADDR,
	(char*)&bREUSE,
	sizeof(bREUSE)
    ) ;

    int iArgument = 16384, iArgumentLength = sizeof(iArgument) ;
    setsockopt(
	fdbind,
	SOL_SOCKET,
	SO_SNDBUF,
	(char*)&iArgument,
	iArgumentLength
    ) ;

    iArgumentLength = sizeof(iArgument) ;
    getsockopt(
	fdbind,
	SOL_SOCKET,
	SO_SNDBUF,
	(char*)&iArgument,
	&iArgumentLength
    ) ;

    struct sockaddr_in sin ;
    memset( (void*)&sin, 0, sizeof(sin) ) ;
    sin.sin_family = AF_INET ;
    sin.sin_port = htons( s_Devices[iDEVICE].iPort ) ;

    int status = bind( fdbind, (struct sockaddr*)&sin, sizeof(sin) ) ;
    if( status ) {
	s_Devices[iDEVICE].fdbind = -1 ;
	return ;
    }

    status = listen( fdbind, 1 ) ;
    if( status ) {
	s_Devices[iDEVICE].fdbind = -1 ;
	return ;
    }

    s_Devices[iDEVICE].fdbind = fdbind ;
    WSAEventSelect( fdbind, s_Devices[iDEVICE].hBindEvent, FD_ACCEPT ) ;
}

int CALLBACK fAcceptTest( LPWSABUF, LPWSABUF, LPQOS, LPQOS, LPWSABUF, LPWSABUF, GROUP FAR *, DWORD);

static int CALLBACK fAcceptTest( 
    LPWSABUF lpCallerId,
    LPWSABUF lpCallerData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    LPWSABUF lpCalleeId,
    LPWSABUF lpCalleeData,
    GROUP FAR * g,
    DWORD_PTR dwCallbackData
    )
{
    int iDEVICE = (int)dwCallbackData ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "in accept: %d %d\n", iDEVICE, s_Devices[iDEVICE].bConnected ) ;

    if( s_Devices[iDEVICE].bConnected == TRUE ) return( CF_REJECT ) ;
    else return( CF_ACCEPT ) ;
}

static void AcceptConnection( int iDEVICE )
{
    struct sockaddr_in addr ;
    int lenaddr = sizeof(struct sockaddr_in) ;
    DWORD dwData = iDEVICE ;
    int fd ;

    fd = WSAAccept( s_Devices[iDEVICE].fdbind, (struct sockaddr*)&addr, &lenaddr, (LPCONDITIONPROC)fAcceptTest, (DWORD)iDEVICE ) ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Accept return: %d\n", fd ) ;
    if( fd >= 0 && s_Devices[iDEVICE].bConnected == FALSE ) {
	s_Devices[iDEVICE].fd = fd ;
	s_Devices[iDEVICE].sin = addr ;
	WSAEventSelect( s_Devices[iDEVICE].fd, s_Devices[iDEVICE].hConnectEvent, FD_READ | FD_WRITE | FD_CLOSE ) ;
	s_Devices[iDEVICE].bConnected = TRUE ;
	if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Accepted: %d\n", fd ) ;
	s_Devices[iDEVICE].bCanWrite = TRUE ;

	strcpy_s( s_Devices[iDEVICE].cHostIP, sizeof(s_Devices[iDEVICE].cHostIP), inet_ntoa( addr.sin_addr ) ) ;
	s_Devices[iDEVICE].iHostPort = ntohs( addr.sin_port ) ;

	struct hostent *cname ;
	cname = gethostbyaddr( (char*)&addr.sin_addr, sizeof(struct in_addr), addr.sin_family );

	if( cname ) strcpy_s( s_Devices[iDEVICE].cHostName, sizeof(s_Devices[iDEVICE].cHostName), cname->h_name ) ;
	else strcpy_s( s_Devices[iDEVICE].cHostName, sizeof(s_Devices[iDEVICE].cHostName), "Unknown" ) ;

	s_Devices[iDEVICE].ulNetworkRx = 0 ;
	s_Devices[iDEVICE].ulNetworkTx = 0 ;

	if( TRUE ) {
	    BOOL bKEEP_ALIVE = TRUE ;
	    setsockopt(
		fd,
		SOL_SOCKET,
		SO_KEEPALIVE,
		(char*)&bKEEP_ALIVE,
		sizeof(bKEEP_ALIVE)
	    ) ;

	    int iArgument = 16384, iArgumentLength = sizeof(iArgument) ;
	    setsockopt(
		fd,
		SOL_SOCKET,
		SO_SNDBUF,
		(char*)&iArgument,
		iArgumentLength
	    ) ;
	}

    }
}

static void CloseConnection( int iDEVICE )
{
    s_Devices[iDEVICE].bConnected = FALSE ;
    closesocket( s_Devices[iDEVICE].fd ) ;
    s_Devices[iDEVICE].fd = -1 ;
}

static BOOL RemoveLFCR( char *p, int iLen, int *newLen, int *newOffset )
{
    /* We always remove a trailing \r\n */
    int i ;

    BOOL bSawLF = FALSE ;
    *newLen = iLen ;
    *newOffset = iLen ;

    for( i = iLen-1 ; i >= 0 ; i-- ) {
	if( p[i] == '\n' ) { p[i] = '\0' ; *newLen = i ; bSawLF = TRUE ; continue ; }

	if( bSawLF ) {
	    if( p[i] == '\r' ) { p[i] = '\0' ; *newLen = i ; }
	    break ;
	} else {
	    *newOffset = i ;
	}
    }

    return( bSawLF ) ;
}


static void ReadConnection( int iDEVICE )
{
    int tOffset = s_Devices[iDEVICE].tOffset ;
    int tLength ;
    char *tBuffer = s_Devices[iDEVICE].tBuffer ;

    int iStatus = recv( s_Devices[iDEVICE].fd, tBuffer+tOffset, s_Devices[iDEVICE].tSize-tOffset, 0 ) ;

    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "recv bytes: %d\n", iStatus ) ;
    if( iStatus == 0 || iStatus == SOCKET_ERROR ) {
        CloseConnection( iDEVICE ) ;
        return ;
    }
    s_Devices[iDEVICE].ulNetworkRx += iStatus ;
    iStatus += tOffset ;
    tBuffer[ iStatus ] = '\0' ;

    BOOL bStatus = RemoveLFCR( tBuffer, iStatus, &tLength, &tOffset ) ;

    if( bStatus ) {
        memcpy( s_Devices[iDEVICE].cmdBuffer, tBuffer, tLength ) ;
	s_Devices[iDEVICE].cmdBuffer[tLength] = '\0' ;

	strncpy( s_Devices[iDEVICE].cmdBuffer+tLength, s_Devices[iDEVICE].cTxAppendChars, s_Devices[iDEVICE].lenTxAppendChars ) ;
	tLength += s_Devices[iDEVICE].lenTxAppendChars ;

	s_Devices[iDEVICE].cmdLength = tLength ;
	s_Devices[iDEVICE].cmdBuffer[tLength] = '\0' ;
	s_Devices[iDEVICE].bCmdReady = TRUE ;

	memmove( tBuffer, tBuffer+tOffset, (iStatus-tOffset) ) ;
	s_Devices[iDEVICE].tOffset = iStatus - tOffset ;
    } else {
        s_Devices[iDEVICE].cmdBuffer[0] = '\0' ;
	s_Devices[iDEVICE].tOffset = iStatus ;
    }

    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "crlf status:<%d>\n", bStatus ) ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "recv:<%s>\n", tBuffer ) ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "cmd: Ready %d Len=%d <%s>\n", s_Devices[iDEVICE].bCmdReady, s_Devices[iDEVICE].cmdLength, s_Devices[iDEVICE].cmdBuffer ) ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "append:<%s>\n", s_Devices[iDEVICE].cTxAppendChars ) ;
}

static void WriteToConnection( int iDEVICE )
{
    int iVecs = 0 ;
    WSABUF ioVec[32] ;
    int rLength = s_Devices[iDEVICE].rLength ;
    char *rBuffer = s_Devices[iDEVICE].rBuffer ;

    ioVec[iVecs].len = rLength ;
    ioVec[iVecs].buf = rBuffer ;
    iVecs++ ;

    ioVec[iVecs].len = 2 ;
    ioVec[iVecs].buf = "\r\n" ;
    iVecs++ ;

    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Here: %3d\n", 17 ) ;
    DWORD dwSent = 0 ;
    int iRetVal = WSASendTo( s_Devices[iDEVICE].fd, ioVec, iVecs, &dwSent, 0, (sockaddr*)&s_Devices[iDEVICE].sin, sizeof(struct sockaddr_in), NULL, NULL ) ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Net = %d %ld\n", iRetVal, dwSent ) ;

    s_Devices[iDEVICE].bCanWrite = TRUE ;
    if( iRetVal == 0 ) {
	s_Devices[iDEVICE].ulNetworkTx += dwSent ;
    } else { 							// Had this: if( iRetVal == SOCKET_ERROR ) {
	int iLastError = WSAGetLastError() ;
	if( iLastError == WSAEWOULDBLOCK ) {
	    s_Devices[iDEVICE].bCanWrite = FALSE ;
	    WSAEventSelect( s_Devices[iDEVICE].fd, s_Devices[iDEVICE].hConnectEvent, FD_READ | FD_WRITE | FD_CLOSE ) ;
	}
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Here: %3d\n", 18 ) ;
    }
}

int OpenDevice( int iDEVICE )
{
    /*idn.c
    This example program queries a GPIB device for an identification string
    and prints the results. Note that you must change the address. */

    ViSession vi;
    ViStatus viStatus ;

    viStatus = viOpen( DefaultRM, s_Devices[iDEVICE].cVISA, VI_NULL, VI_NULL, &vi ) ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "viOpen: %08lx\n", viStatus ) ;
    if( viStatus != VI_SUCCESS ) {
	s_Devices[iDEVICE].iOpened = -1 ;
	return( 1 ) ;
    }

    s_Devices[iDEVICE].vi = vi ;
    s_Devices[iDEVICE].iOpened = 1 ;
    s_Devices[iDEVICE].iErrors = 0 ;

    /* Initialize device */
    // viPrintf (vi, "*RST\n");

    if( s_Devices[iDEVICE] . iTimeOut > 0 )
	viStatus = viSetAttribute( vi, VI_ATTR_TMO_VALUE, s_Devices[iDEVICE] . iTimeOut ) ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "viSetAttribute: %08lx\n", viStatus ) ;

    if( s_Devices[iDEVICE] . cRxTermChars[0] != 0 ) {
	viStatus = viSetAttribute( vi, VI_ATTR_TERMCHAR, s_Devices[iDEVICE] . cRxTermChars[0] ) ;
	viStatus = viSetAttribute( vi, VI_ATTR_TERMCHAR_EN, VI_TRUE ) ;
	if( s_Devices[iDEVICE] . eType == eSerial ) viStatus = viSetAttribute( vi, VI_ATTR_ASRL_END_IN, VI_ASRL_END_TERMCHAR ) ;
    } else {
	viStatus = viSetAttribute( vi, VI_ATTR_TERMCHAR_EN, VI_FALSE ) ;
	if( s_Devices[iDEVICE] . eType == eSerial ) viStatus = viSetAttribute( vi, VI_ATTR_ASRL_END_IN, VI_ASRL_END_NONE ) ;
    }
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "viSetAttribute: %08lx\n", viStatus ) ;

    if( s_Devices[iDEVICE] . eType == eSerial && s_Devices[iDEVICE] . iBaudRate > 0 )
	viStatus = viSetAttribute( vi, VI_ATTR_ASRL_BAUD, s_Devices[iDEVICE] . iBaudRate ) ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "viSetAttribute: %08lx\n", viStatus ) ;

    s_Devices[iDEVICE].ulDeviceRx = 0 ;
    s_Devices[iDEVICE].ulDeviceTx = 0 ;

    WaitForSingleObject( s_Devices[iDEVICE].hMutex, INFINITE ) ;
    _snprintf( s_Devices[iDEVICE].cCmdDisplay, DISPLAY_SIZE, "" ) ;
    _snprintf( s_Devices[iDEVICE].cReplyDisplay, DISPLAY_SIZE, "" ) ;
    ReleaseMutex( s_Devices[iDEVICE].hMutex ) ;

    return( 0 ) ;
}

void DisplayOnly( int iDEVICE, char *pText, int iTextLength, char *pDisplay )
{
    int i, j = 0 ;

    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "displayonly: %d <%s> %d %08lx\n", iDEVICE, pText, iTextLength, pDisplay ) ;

    WaitForSingleObject( s_Devices[iDEVICE].hMutex, INFINITE ) ;

    for( i = 0 ; i < iTextLength ; i++ ) {
	u_char X = pText[i] ;
	int M, im ;
	char T[8] ;

	if( X > 0x7f || iscntrl(X) ) {
	    M = _snprintf( T, sizeof(T), "<%02x>", (UINT)X ) ;
	} else {
	    T[0] = (char)X ; M = 1 ;
	}
	for( im = 0 ; im < M ; im++ ) {
	    pDisplay[j] = T[im] ; j++ ;
	    if( j >= DISPLAY_SIZE ) break ;
	}
	if( j >= DISPLAY_SIZE ) break ;
    }
    pDisplay[j] = '\0' ;
    ReleaseMutex( s_Devices[iDEVICE].hMutex ) ;

    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "out displayonly: %d <%s> %d %s %d\n", iDEVICE, pText, iTextLength, pDisplay, j ) ;
}


void TransactDevice( int iDEVICE )
{
    BOOL bJustWrite = FALSE ;

    s_Devices[iDEVICE].bCmdReady = FALSE ;
    ViStatus viStatus = VI_SUCCESS ;

    if( s_Devices[iDEVICE] . bRxFlush ) viStatus = viFlush( s_Devices[iDEVICE] . vi, VI_READ_BUF_DISCARD | VI_WRITE_BUF_DISCARD ) ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Flush (%d) visa status: %d\n", s_Devices[iDEVICE].bRxFlush, viStatus ) ;

    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Write to visa:\n" ) ;
    int cmdLength = s_Devices[iDEVICE].cmdLength ;
    char *cmdBuffer = s_Devices[iDEVICE].cmdBuffer ;
    ViUInt32 iWritten = 0 ;

    if( (s_Devices[iDEVICE].ucTxWriteOnlyChar != 0) && (cmdBuffer[0] == s_Devices[iDEVICE].ucTxWriteOnlyChar) ) {
	bJustWrite = TRUE ;
	cmdBuffer++ ;
	cmdLength-- ;
    }
    DisplayOnly( iDEVICE, cmdBuffer, cmdLength, s_Devices[iDEVICE].cCmdDisplay ) ;

    WaitForSingleObject( s_Devices[iDEVICE].hMutex, INFINITE ) ;
    _snprintf( s_Devices[iDEVICE].cReplyDisplay, DISPLAY_SIZE, "" ) ;
    ReleaseMutex( s_Devices[iDEVICE].hMutex ) ;

    if( cmdLength > 0 ) {
	viStatus = viWrite( s_Devices[iDEVICE] . vi, (ViBuf)cmdBuffer, cmdLength, &iWritten ) ;
	if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Write status 0x%lx [%d]\n", viStatus, iWritten ) ;
    } else {
	viStatus = VI_SUCCESS ;
	iWritten = 0 ;
    }
    if( viStatus == VI_SUCCESS ) {
	s_Devices[iDEVICE].ulDeviceTx += iWritten ;
	s_Devices[iDEVICE].iErrors = 0 ;
    } else {
	WaitForSingleObject( s_Devices[iDEVICE].hMutex, INFINITE ) ;
	_snprintf( s_Devices[iDEVICE].cReplyDisplay, DISPLAY_SIZE, "ERR 400 VISA Write Error <%08lx>", viStatus ) ;
	ReleaseMutex( s_Devices[iDEVICE].hMutex ) ;
	_snprintf( s_Devices[iDEVICE].rBuffer, DISPLAY_SIZE, "ERR 400 VISA Write Error <%08lx>", viStatus ) ;
	s_Devices[iDEVICE].rLength = strlen( s_Devices[iDEVICE].rBuffer ) ;
	if( s_Devices[iDEVICE].bCanWrite ) {
	    WriteToConnection( iDEVICE ) ;
	}
	viClear( s_Devices[iDEVICE] . vi ) ;
	s_Devices[iDEVICE].iErrors++ ;
	return ;
    }

    if( bJustWrite ) {
	WaitForSingleObject( s_Devices[iDEVICE].hMutex, INFINITE ) ;
	_snprintf( s_Devices[iDEVICE].cReplyDisplay, DISPLAY_SIZE, "ERR 200 Nothing to read.", viStatus ) ;
	ReleaseMutex( s_Devices[iDEVICE].hMutex ) ;
	_snprintf( s_Devices[iDEVICE].rBuffer, DISPLAY_SIZE, "ERR 200 Nothing to read.", viStatus ) ;
	s_Devices[iDEVICE].rLength = strlen( s_Devices[iDEVICE].rBuffer ) ;
	if( s_Devices[iDEVICE].bCanWrite ) {
	    WriteToConnection( iDEVICE ) ;
	}
	return ;
    }

    s_Devices[iDEVICE].tOffset = 0 ;

    if( s_Devices[iDEVICE].iDelayAfterWrite > 0 )
	Sleep( s_Devices[iDEVICE].iDelayAfterWrite ) ;

    int rSize = s_Devices[iDEVICE].rSize ;
    char *rBuffer = s_Devices[iDEVICE].rBuffer ;
    ViUInt32 rLength = 0 ;

    if( s_Devices[iDEVICE].iRxReadChars > 0 )
	rSize = s_Devices[iDEVICE].iRxReadChars ;

    viStatus = viRead( s_Devices[iDEVICE].vi, (ViBuf)rBuffer, rSize, &rLength ) ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Read rSize = %d Status = 0x%lx Length=%d Term=%x Trim=%d\n", rSize, viStatus, rLength, s_Devices[iDEVICE].cRxTermChars[0], s_Devices[iDEVICE].iRxTrimChars ) ;

    if( (viStatus == VI_SUCCESS) || (viStatus == VI_SUCCESS_MAX_CNT) ) {
	s_Devices[iDEVICE].iErrors = 0 ;
    } else {
	WaitForSingleObject( s_Devices[iDEVICE].hMutex, INFINITE ) ;
	_snprintf( s_Devices[iDEVICE].cReplyDisplay, DISPLAY_SIZE, "ERR 400 VISA Read Error <%08lx>", viStatus ) ;
	ReleaseMutex( s_Devices[iDEVICE].hMutex ) ;
	_snprintf( s_Devices[iDEVICE].rBuffer, DISPLAY_SIZE, "ERR 400 VISA Read Error <%08lx>", viStatus ) ;
	s_Devices[iDEVICE].rLength = strlen( s_Devices[iDEVICE].rBuffer ) ;
	if( s_Devices[iDEVICE].bCanWrite ) {
	    WriteToConnection( iDEVICE ) ;
	}
	// Removed this. Not obviously doing any good and definitely pushed out a null byte to a serial device!
	// viClear( s_Devices[iDEVICE] . vi ) ;
	s_Devices[iDEVICE].iErrors++ ;
	return ;
    }

    rBuffer[rLength] = '\0' ;


    if( (int)rLength >= s_Devices[iDEVICE].iRxTrimChars ) {
	rLength -= s_Devices[iDEVICE].iRxTrimChars ;
	rBuffer[rLength] = '\0' ;
	s_Devices[iDEVICE].ulDeviceRx += rLength ;
    }
    if( s_Devices[iDEVICE].bBinary ) {
	char temp[256] ;
	u_char *ucR = (u_char*)rBuffer ;
	int i, iL = 0 ;

	for( i = 0 ; i < rLength ; i++ ) {
	    iL += _snprintf( temp+iL, sizeof(temp)-iL, "%02x", ucR[i] ) ;
	}
	temp[iL] = '\0' ;
	strncpy( s_Devices[iDEVICE].rBuffer, temp, s_Devices[iDEVICE].rSize ) ;
	rLength = strlen( temp ) ;
    }
    DisplayOnly( iDEVICE, rBuffer, rLength, s_Devices[iDEVICE].cReplyDisplay ) ;

    s_Devices[iDEVICE].rLength = rLength ;
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Read 0x%lx %d Rtn=%d <%s>\n", viStatus, rLength, s_Devices[iDEVICE].bCanWrite, s_Devices[iDEVICE] . rBuffer ) ;

    if( s_Devices[iDEVICE].bCanWrite ) {
	WriteToConnection( iDEVICE ) ;
    }
}

void CloseDevice( int iDEVICE )
{
    ViStatus viStatus = viClose( s_Devices[iDEVICE] . vi );
    s_Devices[iDEVICE].iOpened = -1 ;

    WaitForSingleObject( s_Devices[iDEVICE].hMutex, INFINITE ) ;
    _snprintf( s_Devices[iDEVICE].cCmdDisplay, DISPLAY_SIZE, "Closed %08lx %d", viStatus, s_Devices[iDEVICE].iErrors ) ;
    ReleaseMutex( s_Devices[iDEVICE].hMutex ) ;
    Sleep(5000) ;
}

int SetupAndRun( int iDEVICE )
{
    HANDLE hEventArray[3] ;

    HANDLE hVisaEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ;
    if( hVisaEvent == INVALID_HANDLE_VALUE ) {
        return( 2 ) ;
    }

    s_Devices[iDEVICE].hBindEvent = WSACreateEvent() ;
    CreateSocketAndBind( iDEVICE ) ;
    if( (int)(s_Devices[iDEVICE].fdbind) < 0 ) {
	return( -1 ) ;
    }
    s_Devices[iDEVICE].iOpened = 0 ;
    s_Devices[iDEVICE].iErrors = 0 ;

    s_Devices[iDEVICE].hConnectEvent = WSACreateEvent() ;
    s_Devices[iDEVICE].bConnected = FALSE ;

    s_Devices[iDEVICE].tSize = BUFFER_SZ ;
    s_Devices[iDEVICE].tBuffer = (char*)malloc( BUFFER_SZ ) ;
    s_Devices[iDEVICE].tLength = 0 ;
    s_Devices[iDEVICE].tOffset = 0 ;

    s_Devices[iDEVICE].cmdSize = BUFFER_SZ ;
    s_Devices[iDEVICE].cmdBuffer = (char*)malloc( BUFFER_SZ ) ;
    s_Devices[iDEVICE].cmdLength = 0 ;
    s_Devices[iDEVICE].bCmdReady = FALSE ;

    s_Devices[iDEVICE].rSize = BUFFER_SZ ;
    s_Devices[iDEVICE].rBuffer = (char*)malloc( BUFFER_SZ ) ;
    s_Devices[iDEVICE].rLength = 0 ;

    s_Devices[iDEVICE].bCanWrite = FALSE ;
    s_Devices[iDEVICE].ulNetworkRx = 0 ;
    s_Devices[iDEVICE].ulNetworkTx = 0 ;
    s_Devices[iDEVICE].ulDeviceRx = 0 ;
    s_Devices[iDEVICE].ulDeviceTx = 0 ;

    hEventArray[0] = hVisaEvent ;
    hEventArray[1] = s_Devices[iDEVICE].hBindEvent ;
    int iBaseNEvents = 2 ;
    int iNEvents = 2 ;

    while( bRunning ) {


	int iOpenTries = 0 ;
	while( OpenDevice( iDEVICE ) > 0 ) {
	    Sleep( 2000 ) ;
	    iOpenTries++ ;
	    if( bRunning == FALSE ) return( 0 ) ;
	}

	if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Device opened.\n" ) ;

	while( bRunning ) {
	    iNEvents = iBaseNEvents ;
	    if( s_Devices[iDEVICE].bConnected ) {
		hEventArray[iNEvents] = s_Devices[iDEVICE].hConnectEvent ;
		iNEvents++ ;
	    }

	    DWORD  dwWait = WSAWaitForMultipleEvents( iNEvents, hEventArray, FALSE, 2000, FALSE ) ;

	    if( dwWait == WSA_WAIT_TIMEOUT ) {
		continue ;
	    }
	    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Wait %ld.\n", dwWait ) ;
	    if( dwWait == WSA_WAIT_FAILED ) {
		int iLastError = WSAGetLastError() ;
		if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Wait %ld %08lx.\n", dwWait, iLastError ) ;
		continue ;
	    }

	    int iEvent = dwWait - WSA_WAIT_EVENT_0 ;
	    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "iEvent %d.\n", iEvent ) ;

	    if( iEvent == 0 ) {
		if( s_Devices[iDEVICE].bCmdReady ) {
		    TransactDevice( iDEVICE ) ;
		    if( s_Devices[iDEVICE].iErrors >= s_Devices[iDEVICE].iMaxErrors ) {
			s_Devices[iDEVICE].iOpened = 2 ;
		    } else {
			s_Devices[iDEVICE].iOpened = 1 ;
		    }
		}

	    } else if( iEvent == 1 ) {
		WSANETWORKEVENTS wsaEvent ;

		int retval = WSAEnumNetworkEvents( s_Devices[iDEVICE].fdbind, s_Devices[iDEVICE].hBindEvent, &wsaEvent ) ;
		if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "iEvent %d: %08lx %08lx\n", iEvent, wsaEvent.lNetworkEvents, FD_ACCEPT ) ;

		if( wsaEvent.lNetworkEvents & FD_ACCEPT ) {
		    AcceptConnection( iDEVICE ) ;
		}

	    } else if( iEvent == 2 ) {
		WSANETWORKEVENTS wsaEvent ;
		int retval = WSAEnumNetworkEvents( s_Devices[iDEVICE].fd, s_Devices[iDEVICE].hConnectEvent, &wsaEvent ) ;
		int iLastErr = GetLastError() ;

		if( s_Devices[iDEVICE].bConnected ) {
		    if( wsaEvent.lNetworkEvents & FD_CLOSE ) {
			CloseConnection( iDEVICE ) ;
		    } else if( wsaEvent.lNetworkEvents & FD_READ ) {
			ReadConnection( iDEVICE ) ;
			if( s_Devices[iDEVICE].bCmdReady ) SetEvent( hVisaEvent ) ;

		    } else if( wsaEvent.lNetworkEvents & FD_WRITE ) {
			if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "write\n" ) ;
			s_Devices[iDEVICE].bCanWrite = TRUE ;
			WSAEventSelect( s_Devices[iDEVICE].fd, s_Devices[iDEVICE].hConnectEvent, FD_READ | FD_CLOSE ) ;
		    }
		}
	    }
	}
	CloseDevice( iDEVICE ) ;
	if( bRunning == FALSE ) break ;
	Sleep(2000) ;
    }

    return( 0 ) ;
}
DWORD WINAPI fThread_N( LPVOID lpParameter )
{
    WSADATA wsadata ;

    WORD wVersionRequested = MAKEWORD( 2, 0 ) ;
    int err = WSAStartup( wVersionRequested, &wsadata ) ;
    err ;
    int iIDN = 0 ;

    BOOL bStatus = SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS ) ;
    bStatus = SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );   // 

    int iDEVICE = *(int*) lpParameter ;

#if 0
    char temp[256] ;
    sprintf_s( temp, sizeof(temp), "visa.%03d", iDEVICE ) ;

    FLOG(iDEVICE) = fopen( temp, "w" ) ; setvbuf( FLOG(iDEVICE), NULL, _IONBF, 0 ) ;
#else
    FLOG(iDEVICE) = NULL ;
#endif
    if( FLOG(iDEVICE) ) fprintf( FLOG(iDEVICE), "Starting ...\n" ) ;

    s_Devices[iDEVICE].tBuffer = NULL ;
    s_Devices[iDEVICE].tSize = 0 ;
    s_Devices[iDEVICE].cmdSize = 0 ;
    s_Devices[iDEVICE].cmdBuffer = NULL ;
    s_Devices[iDEVICE].rSize = 0 ;
    s_Devices[iDEVICE].rBuffer = NULL ;

    int iResult = SetupAndRun( iDEVICE ) ;

    if( s_Devices[iDEVICE].tBuffer != NULL ) {
	free( s_Devices[iDEVICE].tBuffer ) ;
	s_Devices[iDEVICE].tBuffer = NULL ;
	s_Devices[iDEVICE].tSize = 0 ;
    }
	    
    if( s_Devices[iDEVICE].cmdBuffer != NULL ) {
	free( s_Devices[iDEVICE].cmdBuffer ) ;
	s_Devices[iDEVICE].cmdSize = 0 ;
	s_Devices[iDEVICE].cmdBuffer = NULL ;
    }

    if( s_Devices[iDEVICE].rBuffer != NULL ) {
	free( s_Devices[iDEVICE].rBuffer ) ;
	s_Devices[iDEVICE].rSize = 0 ;
	s_Devices[iDEVICE].rBuffer = NULL ;
    }
    return( iResult ) ;
}


// Globals.h: interface for the Globals class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBALS_H__A1941F79_1AC8_47CF_AD96_192356D537B9__INCLUDED_)
#define AFX_GLOBALS_H__A1941F79_1AC8_47CF_AD96_192356D537B9__INCLUDED_

#include <visa.h>

#ifdef _GLOBALS_

#define	_EXTERN_ 
#define INITZERO = 0 
#define INITVALUE(a) = (a)

#else

#define _EXTERN_ extern
#define INITZERO
#define INITVALUE(a)

#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define N_DEVS  (64)
#define BUFFER_SZ	(1024)
#define DISPLAY_SIZE	(64)
#define N_WNDS  (N_DEVS+1)

enum eDeviceType { eUnknown, eSerial, eGPIB } ;

_EXTERN_ int iNDevs INITZERO ;
_EXTERN_ int iNWnds INITZERO ;
_EXTERN_ ViSession DefaultRM ;
_EXTERN_ BOOL bRMInitialized INITVALUE(FALSE) ;

_EXTERN_ BOOL bRunning INITVALUE(FALSE) ;

typedef struct {
    int iDevice ;
    char *cDevice ;
    char *cName ;
    char *cVISA ;
    int iPort ;
    BOOL bEnabled ;
    int iOpened ;
    int iErrors ;
    int iMaxErrors ;
    enum eDeviceType eType ;
    char *cDescription ;

    int iTimeOut ; 
    BOOL bRxFlush ; 
    char cRxTermChars[4] ;
    int iRxTrimChars ;
    int iRxReadChars ;
    char cTxAppendChars[4] ;
    int lenTxAppendChars ;
    u_char ucTxWriteOnlyChar ;
    int iBaudRate ;
    int iDelayAfterWrite ;
    BOOL bBinary ;

    ULONG ulNetworkRx ;
    ULONG ulNetworkTx ;
    ULONG ulDeviceRx ;
    ULONG ulDeviceTx ;

    HANDLE hThreadN ; 
    DWORD dwThreadID ;
    SOCKET fdbind ;
    HANDLE hBindEvent ;
    BOOL bConnected ;
    SOCKET fd ;
    HANDLE hConnectEvent ;
    struct sockaddr_in sin ;
    char cHostName[MAX_PATH] ;
    char cHostIP[MAX_PATH] ;
    int iHostPort ;

    int iBaseID ;
    ViSession vi ;
    FILE *fLog ;
    int tSize ;
    char *tBuffer ;
    int tLength ;
    int tOffset ;
    int iWND ;

    int cmdSize ;
    char *cmdBuffer ;
    int cmdLength ;
    BOOL bCmdReady ;

    int rSize ;
    char *rBuffer ;
    int rLength ;
    BOOL bCanWrite ;

    char cCmdDisplay[DISPLAY_SIZE+8] ;
    char cReplyDisplay[DISPLAY_SIZE+8] ;
    
    HANDLE hMutex ;
} s_Device ;

_EXTERN_ s_Device s_Devices[N_DEVS] ;

_EXTERN_ int iTiming INITVALUE(50) ;

DWORD WINAPI fThread_N( LPVOID lpParameter ) ;
void Alert( char * ) ;
void InitializeDeviceStruct( int ) ;


#define DEBUG_OUT
#undef DEBUG_OUT

#ifdef DEBUG_OUT

#define OPEN_LOG        fopen
#define DO_LOG          if( bDebug ) fprintf
#define DO_LOG_F        if( bDebug ) fflush
#define CLOSE_LOG       fclose

#else

#define OPEN_LOG        NULL,
#define DO_LOG          if( 0 )
#define DO_LOG_F        if( 0 )
#define CLOSE_LOG       if( 0 )

#endif

#endif // !defined(AFX_GLOBALS_H__A1941F79_1AC8_47CF_AD96_192356D537B9__INCLUDED_)


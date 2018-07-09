// Globals.cpp: implementation of the Globals class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#define _GLOBALS_
#include <sys/types.h>
#include "Globals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

void Alert( char *temp )
{
    MessageBox( GetDesktopWindow(), temp, "Problem", MB_OK ) ;
}

void InitializeDeviceStruct( int iD ) 
{
    s_Devices[iD] . iDevice = 0 ;
    s_Devices[iD] . cDevice = NULL ;
    s_Devices[iD] . cName = NULL ;
    s_Devices[iD] . cVISA = NULL ;
    s_Devices[iD] . iPort = 0 ;
    s_Devices[iD] . bEnabled = FALSE ;
    s_Devices[iD] . iOpened = 0 ;
    s_Devices[iD] . iErrors = 0 ;
    s_Devices[iD] . iMaxErrors = 0 ;
    s_Devices[iD] . eType = eUnknown ;
    s_Devices[iD] . cDescription = NULL ;

    s_Devices[iD] . iTimeOut = 0 ;
    s_Devices[iD] . bRxFlush = FALSE ;
    memset( s_Devices[iD] . cRxTermChars, 0, sizeof(s_Devices[iD] . cRxTermChars) ) ;
    s_Devices[iD] . iRxTrimChars = 0 ;
    s_Devices[iD] . iRxReadChars = 0 ;
    memset( s_Devices[iD] . cTxAppendChars, 0, sizeof(s_Devices[iD] . cTxAppendChars) ) ;
    s_Devices[iD] . lenTxAppendChars = 0 ;
    s_Devices[iD] . ucTxWriteOnlyChar = 0 ;
    s_Devices[iD] . iBaudRate = 0 ;
    s_Devices[iD] . iDelayAfterWrite = 0 ;
    s_Devices[iD] . bBinary = FALSE ;

    s_Devices[iD] . ulNetworkRx = 0 ;
    s_Devices[iD] . ulNetworkTx = 0 ;
    s_Devices[iD] . ulDeviceRx = 0 ;
    s_Devices[iD] . ulDeviceTx = 0 ;

    s_Devices[iD] . hThreadN = NULL ;
    s_Devices[iD] . dwThreadID = 0 ;
    s_Devices[iD] . fdbind = -1  ;
    s_Devices[iD] . hBindEvent = NULL ;
    s_Devices[iD] . bConnected = FALSE ;
    s_Devices[iD] . fd = -1  ;
    s_Devices[iD] . hConnectEvent = NULL ;
    memset( (void*)&s_Devices[iD] . sin, 0, sizeof(struct sockaddr_in) ) ; ;
    memset( s_Devices[iD] . cHostName, 0, sizeof(s_Devices[iD] . cHostName) ) ;
    memset( s_Devices[iD] . cHostIP, 0, sizeof(s_Devices[iD] . cHostIP) ) ;
    s_Devices[iD] . iHostPort = 0 ;

    s_Devices[iD] . iBaseID = 0 ;
    memset( (void*)&(s_Devices[iD] . vi), 0, sizeof(s_Devices[iD] . vi) ) ;
    s_Devices[iD] . fLog = NULL ;
    s_Devices[iD] . tSize = 0 ;
    s_Devices[iD] . tBuffer = NULL ;
    s_Devices[iD] . tLength = 0 ;
    s_Devices[iD] . tOffset = 0 ;
    s_Devices[iD] . iWND = -1  ;

    s_Devices[iD] . cmdSize = 0 ;
    s_Devices[iD] . cmdBuffer = NULL ;
    s_Devices[iD] . cmdLength = 0 ;
    s_Devices[iD] . bCmdReady = FALSE ;

    s_Devices[iD] . rSize = 0 ;
    s_Devices[iD] . rBuffer = NULL ;
    s_Devices[iD] . rLength = 0 ;
    s_Devices[iD] . bCanWrite = FALSE ;

    memset( s_Devices[iD] . cCmdDisplay, 0, sizeof(s_Devices[iD] . cCmdDisplay) ) ;
    memset( s_Devices[iD] . cReplyDisplay, 0, sizeof(s_Devices[iD] . cReplyDisplay) ) ;

    s_Devices[iD] . hMutex = NULL ;
}

// Colour.cpp : implementation file
//

#include "stdafx.h"
#include "OCF.h"
#include "Colour.h"


// CColour

IMPLEMENT_DYNAMIC(CColour, CStatic)
#define TOOLTIP_ID 0
static FILE *f = NULL ;

CColour::CColour()
{
    m_TextColor = RGB(0,0,0) ;
    m_BkColor = GetSysColor( COLOR_BTNFACE ) ;
    m_BkBrush.CreateSolidBrush( m_BkColor ) ;
}

CColour::~CColour()
{
}

BEGIN_MESSAGE_MAP(CColour, CStatic)
    ON_WM_CTLCOLOR_REFLECT()
    ON_WM_CREATE()
END_MESSAGE_MAP()

HBRUSH CColour::CtlColor(CDC* pDC, UINT nCtlColor)
{
    pDC->SetTextColor( m_TextColor ) ;
//  pDC->SetBkMode( TRANSPARENT ) ;
    pDC->SetBkColor( m_BkColor );
    return (HBRUSH)m_BkBrush.GetSafeHandle() ;
}

COLORREF CColour::SetTextColor( COLORREF crIn )
{
    COLORREF crOld = m_TextColor ;
    m_TextColor = crIn ;
    return( crOld ) ;
}

void CColour::SetToolTip( LPCTSTR lpszString )
{
    m_cttc.Activate(TRUE) ;
    m_cttc.UpdateTipText( lpszString, this ) ;
}

BOOL CColour::PreTranslateMessage(MSG* pMsg) 
{
    m_cttc.RelayEvent(pMsg);
    return CStatic::PreTranslateMessage(pMsg);
}

int CColour::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
            return -1;

    EnableToolTips(TRUE);
    m_cttc.Create(this, TTS_ALWAYSTIP ) ;
    m_cttc.AddTool(this ) ;
    m_cttc.Activate(FALSE) ;
    return( 0 ) ;
}

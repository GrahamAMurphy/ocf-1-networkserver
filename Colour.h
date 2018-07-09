#pragma once


// CColour

class CColour : public CStatic
{
	DECLARE_DYNAMIC(CColour)

public:
	CColour();
	virtual ~CColour();
	CBrush m_BkBrush ;
	COLORREF m_TextColor ;
	COLORREF m_BkColor ;
	CToolTipCtrl m_cttc ;

	//{{AFX_VIRTUAL(CColour)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CColour)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
	COLORREF SetTextColor( COLORREF ) ;
	void SetToolTip( LPCTSTR ) ;
};

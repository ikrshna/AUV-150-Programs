/*#if !defined(AFX_COLORBTN_H__FB30CD9E_517F_4B26_BC0A_9D9D4BF6BD3C__INCLUDED_)
#define AFX_COLORBTN_H__FB30CD9E_517F_4B26_BC0A_9D9D4BF6BD3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// colorBtn.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CcolorBtn window

class CcolorBtn : public CButton
{
// Construction
public:
	CcolorBtn();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CcolorBtn)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CcolorBtn();

	// Generated message map functions
protected:
	//{{AFX_MSG(CcolorBtn)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORBTN_H__FB30CD9E_517F_4B26_BC0A_9D9D4BF6BD3C__INCLUDED_)*/



#ifndef __COLORBTN_H__
#define __COLORBTN_H__

/////////////////////////////////////////////////////////////////////////////
// colorBtn.h : header file for the CColorButton class
//

const COLORREF CLOUDBLUE = RGB(128, 184, 223);
const COLORREF WHITE = RGB(255, 255, 255);
const COLORREF BLACK = RGB(1, 1, 1);
const COLORREF DKGRAY = RGB(128, 128, 128);
const COLORREF LTGRAY = RGB(192, 192, 192);
const COLORREF YELLOW = RGB(255, 255, 0);
const COLORREF DKYELLOW = RGB(128, 128, 0);
const COLORREF RED = RGB(255, 0, 0);
const COLORREF DKRED = RGB(128, 0, 0);
const COLORREF BLUE = RGB(0, 0, 255);
const COLORREF DKBLUE = RGB(0, 0, 128);
const COLORREF CYAN = RGB(0, 255, 255);
const COLORREF DKCYAN = RGB(0, 128, 128);
const COLORREF GREEN = RGB(0, 255, 0);
const COLORREF DKGREEN = RGB(0, 128, 0);
const COLORREF MAGENTA = RGB(255, 0, 255);
const COLORREF DKMAGENTA = RGB(128, 0, 128);
const COLORREF IVORY2 = RGB(238,238,224);


#define CB_BG_DEFAULT		 LTGRAY
#define CB_FG_DEFAULT		 BLACK				// black text 
#define CB_SID_DEFAULT	 DKGRAY 			// dark gray disabled text


class CColorButton : public CButton
{
DECLARE_DYNAMIC(CColorButton)
public:
	CColorButton(); 
	virtual ~CColorButton(); 

	BOOL Attach(const UINT nID, CWnd* pParent, 
		const COLORREF BGColor = CB_BG_DEFAULT, 	// gray button
		const COLORREF FGColor = CB_FG_DEFAULT, 			// black text 
		const COLORREF DisabledColor = CB_SID_DEFAULT,	// dark gray disabled text
		const UINT nBevel = 2
	);

	void SetFGColor( COLORREF color = CB_FG_DEFAULT, BOOL bRedraw=FALSE) { m_fg = color; if(bRedraw)
InvalidateRect(NULL);} 
	void SetBGColor( COLORREF color = CB_BG_DEFAULT, BOOL bRedraw=FALSE) {	m_bg = color;  if(bRedraw)
InvalidateRect(NULL);}
  void SetDisabledColor(COLORREF color = CB_SID_DEFAULT, BOOL bRedraw=FALSE) { m_disabled= color; 
if(bRedraw) InvalidateRect(NULL);}

	void SetColor( COLORREF colFG = CB_FG_DEFAULT,	COLORREF colBG= CB_BG_DEFAULT,	COLORREF colDIS =
CB_SID_DEFAULT, BOOL bRedraw=TRUE) 
			{ SetFGColor( colFG);
	      SetBGColor( colBG);
	    SetDisabledColor(colDIS);
				if(bRedraw) InvalidateRect(NULL);
			}

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	void DrawFrame(CDC *DC, CRect R, int Inset);
	void DrawFilledRect(CDC *DC, CRect R, COLORREF color);
	void DrawLine(CDC *DC, CRect EndPoints, COLORREF color);
	void DrawLine(CDC *DC, long left, long top, long right, long bottom, COLORREF color);
	void DrawButtonText(CDC *DC, CRect R, const char *Buf, COLORREF TextColor);

	COLORREF GetFGColor() { return m_fg; }	
	COLORREF GetBGColor() { return m_bg; }
	COLORREF GetDisabledColor() { return m_disabled; }
	UINT GetBevel() { return m_bevel; }

private:
	COLORREF m_fg, m_bg, m_disabled;
	UINT m_bevel;

};
#endif 


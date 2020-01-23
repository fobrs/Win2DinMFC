// MyScrollView.cpp : implementation file
//

#include "pch.h"
#include "resource.h"
#include "MyScrollView.h"
#include <Uxtheme.h>
// CMyScrollView




IMPLEMENT_DYNCREATE(CMyScrollView, CScrollView)

CMyScrollView::CMyScrollView()
{
	m_pfn_UpdatePanningFeedback = NULL;

	m_hThemeDLL = LoadLibrary(_T("uxtheme.dll"));
	if (m_hThemeDLL != NULL)
	{
		m_pfn_UpdatePanningFeedback = (__pfn_UpdatePanningFeedback)GetProcAddress(m_hThemeDLL, "UpdatePanningFeedback");
	}
}

CMyScrollView::~CMyScrollView()
{
	if (m_hThemeDLL != NULL)
		FreeLibrary(m_hThemeDLL);
}


BEGIN_MESSAGE_MAP(CMyScrollView, CScrollView)
END_MESSAGE_MAP()


// CMyScrollView drawing

void CMyScrollView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
}


void CMyScrollView::OnDraw(CDC* pDC)
{
}
// CMyScrollView diagnostics

#ifdef _DEBUG
void CMyScrollView::AssertValid() const
{
	CScrollView::AssertValid();
}

#ifndef _WIN32_WCE
void CMyScrollView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif
#endif //_DEBUG


void CMyScrollView::SetScaleToFitSize(SIZE sizeTotal)
{
	CScrollView::SetScaleToFitSize(sizeTotal);
	m_curScrollPos = CPoint(0, 0);
}



void CMyScrollView::ScrollToDevicePosition(POINT ptDev)
{
	ASSERT(ptDev.x >= 0);
	ASSERT(ptDev.y >= 0);

	// Note: ScrollToDevicePosition can and is used to scroll out-of-range
	//  areas as far as CScrollView is concerned -- specifically in
	//  the print-preview code.  Since OnScrollBy makes sure the range is
	//  valid, ScrollToDevicePosition does not vector through OnScrollBy.

	int xOrig = GetScrollPos(SB_HORZ);
	SetScrollPos(SB_HORZ, ptDev.x);
	int yOrig = GetScrollPos(SB_VERT);
	SetScrollPos(SB_VERT, ptDev.y);

	//ScrollWindow(xOrig - ptDev.x, yOrig - ptDev.y);

	DoNoScrollUpdate(xOrig - ptDev.x, yOrig - ptDev.y);


	m_curScrollPos = CPoint(ptDev);
}

void CMyScrollView::CenterOnPoint(CPoint ptCenter) // center in device coords
{
	CRect rect;
	GetClientRect(&rect);           // find size of client window

	int xDesired = ptCenter.x - rect.Width() / 2;
	int yDesired = ptCenter.y - rect.Height() / 2;

	DWORD dwStyle = GetStyle();

	if ((dwStyle & WS_HSCROLL) == 0 || xDesired < 0)
	{
		xDesired = 0;
	}
	else
	{
		int xMax = GetScrollLimit(SB_HORZ);
		if (xDesired > xMax)
			xDesired = xMax;
	}

	if ((dwStyle & WS_VSCROLL) == 0 || yDesired < 0)
	{
		yDesired = 0;
	}
	else
	{
		int yMax = GetScrollLimit(SB_VERT);
		if (yDesired > yMax)
			yDesired = yMax;
	}

	ASSERT(xDesired >= 0);
	ASSERT(yDesired >= 0);

	SetScrollPos(SB_HORZ, xDesired);
	SetScrollPos(SB_VERT, yDesired);
	m_curScrollPos = CPoint(xDesired, yDesired);
}


void CMyScrollView::ScrollToPosition(POINT pt)    // logical coordinates
{
	ASSERT(m_nMapMode > 0);     // not allowed for shrink to fit
	if (m_nMapMode != MM_TEXT)
	{
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);
		dc.LPtoDP((LPPOINT)&pt);
	}

	// now in device coordinates - limit if out of range
	int xMax = GetScrollLimit(SB_HORZ);
	int yMax = GetScrollLimit(SB_VERT);
	int xOverPan = 0;
	int yOverPan = 0;
	if (pt.x < 0) {
		xOverPan = pt.x;
		pt.x = 0;
	}
	else if (pt.x > xMax) {
		xOverPan = pt.x;
		pt.x = xMax;
	}
	if (pt.y < 0) {
		yOverPan = pt.y;
		pt.y = 0;
	}
	else if (pt.y > yMax) {
		yOverPan = pt.y;
		pt.y = yMax;
	}
	if (m_pfn_UpdatePanningFeedback != NULL)
		if (xOverPan != 0 || yOverPan != 0)
		{
			// we reached the bottom / top, pan
			m_pfn_UpdatePanningFeedback(GetSafeHwnd(), xOverPan, yOverPan, FALSE);
		}

	ScrollToDevicePosition(pt);
//	m_curScrollPos = pt;

}

BOOL CMyScrollView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	// calc new x position
	int x = GetScrollPos(SB_HORZ);
	int xOrig = m_curScrollPos.x;

	switch (LOBYTE(nScrollCode))
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = INT_MAX;
		break;
	case SB_LINEUP:
		x -= m_lineDev.cx;
		break;
	case SB_LINEDOWN:
		x += m_lineDev.cx;
		break;
	case SB_PAGEUP:
		x -= m_pageDev.cx;
		break;
	case SB_PAGEDOWN:
		x += m_pageDev.cx;
		break;
	case SB_THUMBTRACK:
		x = nPos;
		break;
	}

	// calc new y position
	int y = GetScrollPos(SB_VERT);
	int yOrig = m_curScrollPos.y;

	switch (HIBYTE(nScrollCode))
	{
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = INT_MAX;
		break;
	case SB_LINEUP:
		y -= m_lineDev.cy;
		break;
	case SB_LINEDOWN:
		y += m_lineDev.cy;
		break;
	case SB_PAGEUP:
		y -= m_pageDev.cy;
		break;
	case SB_PAGEDOWN:
		y += m_pageDev.cy;
		break;
	case SB_THUMBTRACK:
		y = nPos;
		break;
	}

	BOOL bResult = OnScrollBy(CSize(x - xOrig, y - yOrig), bDoScroll);
	if (bResult && bDoScroll)
		UpdateWindow();

	return bResult;
}

BOOL CMyScrollView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	int xOrig, x;
	int yOrig, y;

	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	CScrollBar* pBar;
	DWORD dwStyle = GetStyle();
	pBar = GetScrollBarCtrl(SB_VERT);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_VSCROLL)))
	{
		// vertical scroll bar not enabled
		sizeScroll.cy = 0;
	}
	pBar = GetScrollBarCtrl(SB_HORZ);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_HSCROLL)))
	{
		// horizontal scroll bar not enabled
		sizeScroll.cx = 0;
	}

	// adjust current x position
	xOrig = x = GetScrollPos(SB_HORZ);
	xOrig = x = m_curScrollPos.x;
	int xMax = GetScrollLimit(SB_HORZ);
	x += sizeScroll.cx;
	int xOverPan = 0;
	if (x < 0) {
		xOverPan = x;
		x = 0;
	}
	else if (x > xMax) {
		xOverPan = x;
		x = xMax;
	}
	// adjust current y position
	yOrig = y = GetScrollPos(SB_VERT);
	yOrig = y = m_curScrollPos.y;
	int yMax = GetScrollLimit(SB_VERT);
	y += sizeScroll.cy;
	int yOverPan = 0;
	if (y < 0) {
		yOverPan = y;
		y = 0;
	}
	else if (y > yMax) {
		yOverPan = y;
		y = yMax;
	}


	// did anything change?
	if (x == xOrig && y == yOrig)
	{
		m_curScrollPos = CPoint(x, y);
		return FALSE;
	}


	if (bDoScroll)
	{

		// do scroll and update scroll positions
		//ScrollWindow(-(x - xOrig), -(y - yOrig));

		DoNoScrollUpdate(-(x - xOrig), -(y - yOrig));


		if (x != xOrig)
			SetScrollPos(SB_HORZ, x);
		if (y != yOrig)
			SetScrollPos(SB_VERT, y);
		m_curScrollPos = CPoint(x, y);
	}
	return TRUE;
}


void CMyScrollView::DoNoScrollUpdate(int dx, int dy)
{
	//return;
	RECT r;
	r.bottom = r.left = r.bottom = r.top = 0;

	RECT rect;
	GetClientRect(&rect);


	r = rect;
	if (dx < 0)
	{
		r.left = r.right + dx;
	}
	else
	{
		r.left = 0;
		r.right = dx;
	}
	if (dx != 0)
	{
		//InvalidateRect(&r, 0);

		DrawClientRect(r);
		//TRACE(L"DoNoScrollUpdate: %d %d %d %d\n", r.left, r.top, r.right, r.bottom);
	}

	r = rect;
	if (dy < 0)
	{
		r.top = r.bottom + dy;
	}
	else
	{
		r.top = 0;
		r.bottom = dy;
	}
	if (dy != 0)
	{
		//InvalidateRect(&r, 0);
		DrawClientRect(r);

		//TRACE(L"DoNoScrollUpdate: %d %d %d %d\n", r.left, r.top, r.right, r.bottom);
	}
	//UpdateWindow();

}
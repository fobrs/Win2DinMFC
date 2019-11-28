#pragma once


typedef BOOL(_stdcall *__pfn_UpdatePanningFeedback)(HWND, LONG, LONG, BOOL);

// CMyScrollView view

class CMyScrollView : public CScrollView
{
	DECLARE_DYNCREATE(CMyScrollView)

protected:
	CMyScrollView();           // protected constructor used by dynamic creation
	virtual ~CMyScrollView();

	CPoint m_curScrollPos;
	HINSTANCE m_hThemeDLL;
	__pfn_UpdatePanningFeedback m_pfn_UpdatePanningFeedback;
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	void ScrollToPosition(POINT pt);    // set upper left position
	void SetScaleToFitSize(SIZE sizeTotal);

protected:
	virtual void OnInitialUpdate();     // first time after construct
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view

	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

	void ScrollToDevicePosition(POINT ptDev);
	void CenterOnPoint(CPoint ptCenter);

	DECLARE_MESSAGE_MAP()
};



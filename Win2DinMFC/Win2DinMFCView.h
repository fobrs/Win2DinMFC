// Win2DinMFCView.h : interface of the CWin2DinMFCView class
//

#pragma once
#include "timer.h"
#include "MyScrollView.h"

#include <winrt/Windows.UI.Composition.Desktop.h>
#include "winrt/Microsoft.Graphics.Canvas.h"
#include "winrt/Microsoft.Graphics.Canvas.Effects.h"
#include "winrt/Microsoft.Graphics.Canvas.Text.h"
#include "winrt/Microsoft.Graphics.Canvas.SVG.h"
#include "winrt/Microsoft.Graphics.Canvas.UI.Composition.h"
#include <winrt/Windows.Foundation.Numerics.h>
#include <d2d1svg.h>

using namespace winrt;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Composition;
using namespace winrt::Windows::UI::Composition::Desktop;

using namespace winrt::Microsoft::Graphics::Canvas;
using namespace winrt::Microsoft::Graphics::Canvas::Effects;
using namespace winrt::Microsoft::Graphics::Canvas::Svg;
using namespace winrt::Microsoft::Graphics::Canvas::UI::Composition;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Numerics;

#include "devicelost.h"

class CWin2DinMFCView : public CMyScrollView
{
protected: // create from serialization only
	CWin2DinMFCView() noexcept;
	DECLARE_DYNCREATE(CWin2DinMFCView)

// Attributes
public:
	CWin2DinMFCDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll);
	virtual ULONG GetGestureStatus(CPoint ptTouch);

// Implementation
public:
	virtual ~CWin2DinMFCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void OnDirect3DDeviceLost(DeviceLostHelper const* /* sender */, DeviceLostEventArgs const& /* args */);
	DesktopWindowTarget CWin2DinMFCView::CreateDesktopWindowTarget(Compositor const& compositor, HWND window);
	void CWin2DinMFCView::PrepareVisuals(Compositor const& compositor);

	void Zoom(int zDelta, CPoint pt);
	static VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	bool Redraw(float cx, float cy, float wx, float wy, float width, float height, UINT dpi);
	void Scenario_Wind2d(const Compositor & compositor, const ContainerVisual & root, UINT dpi, int cx, int cy);
	bool LoadSvg();

	HWND m_interopWindowHandle = nullptr;
	Compositor m_compositor;
	DesktopWindowTarget m_target{ nullptr };
	SpriteVisual m_root{ nullptr };

	UINT m_currentDpi;
	int m_width, m_height;

	float m_angle = 0.0f;

	CanvasDevice m_canvasDevice{ nullptr };
	DeviceLostHelper m_devicelost_helper;
	bool m_b_in_device_lost = false;
	CompositionGraphicsDevice m_graphicsDevice{ nullptr };
	winrt::Windows::UI::Composition::CompositionDrawingSurface m_drawingSurface{ nullptr };
	CanvasRenderTarget m_myBitmap{ nullptr };

	CallBackTimer m_cbt;

	CanvasSvgDocument m_svg{ nullptr };
	winrt::hstring m_w;


	// flame effect
	void CreateFlameEffect();
	void SetupText(ICanvasResourceCreator resourceCreator);
	void ConfigureEffect();

	CanvasCommandList m_textCommandList{ nullptr };
	MorphologyEffect m_morphology{ nullptr };
	CompositeEffect m_composite{ nullptr };
	Transform2DEffect m_flameAnimation{ nullptr };
	Transform2DEffect m_flamePosition{ nullptr };
	std::string m_text, m_newText;
	float m_fontSize = 10.0f;


	// zoom, pan
	bool m_bTranslateDragging = false;
	CPoint m_pt_cur_mouse;
	CSize m_scroll_diff;
	clock_t m_scroll_start_time;
	unsigned int m_scroll_time_diff;
	bool m_bIdleTranslateDragging;
	DWORD m_dwFrequency;

	int m_ppm_bitmap_resolution = 72;
	float3x2 m_transform = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

	float m_svg_width, m_svg_height;


// Generated message map functions
protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	LRESULT OnGesture(WPARAM w, LPARAM l);

};

#ifndef _DEBUG  // debug version in Win2DinMFCView.cpp
inline CWin2DinMFCDoc* CWin2DinMFCView::GetDocument() const
   { return reinterpret_cast<CWin2DinMFCDoc*>(m_pDocument); }
#endif


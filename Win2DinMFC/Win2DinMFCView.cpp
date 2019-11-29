// Win2DinMFCView.cpp : implementation of the CWin2DinMFCView class
//

#include "pch.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Win2DinMFC.h"
#endif

#include <memory>
#include <d3d11.h>
#include <D3d11_4.h>
#include "d2d1.h"
#include <d2d1_1.h>
#include <d2d1_3.h>
#include <d2d1helper.h>


#include <windows.ui.composition.interop.h>
#include <ShellScalingAPI.h>
#include <DispatcherQueue.h>
#include <Windows.Graphics.Interop.h>

#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.h>

#include "Win2DinMFCDoc.h"
#include "Win2DinMFCView.h"
#include "timer.h"

#include "ddraw_global.h"

using namespace winrt;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Microsoft::Graphics::Canvas::Effects;
using namespace winrt::Microsoft::Graphics::Canvas::Svg;
using namespace winrt::Microsoft::Graphics::Canvas::Text;



# define M_PI           3.14159265358979323846  /* pi */


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define SCALEPPI(X) ( X * m_currentDpi / 96)
#define PPI(X) (X)

#define ppm_bitmap_MAX_resolution 8000

ddraw_data_t g_ddraw_data;
bool g_m_layoutdiagram_damp_scrolling = true;



std::wstring UTF8_to_wchar(const char * in)
{
	std::wstring out;
	unsigned int codepoint;
	while (*in != 0)
	{
		unsigned char ch = static_cast<unsigned char>(*in);
		if (ch <= 0x7f)
			codepoint = ch;
		else if (ch <= 0xbf)
			codepoint = (codepoint << 6) | (ch & 0x3f);
		else if (ch <= 0xdf)
			codepoint = ch & 0x1f;
		else if (ch <= 0xef)
			codepoint = ch & 0x0f;
		else
			codepoint = ch & 0x07;
		++in;
		if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff))
		{
			if (sizeof(wchar_t) > 2)
				out.append(1, static_cast<wchar_t>(codepoint));
			else if (codepoint > 0xffff)
			{
				out.append(1, static_cast<wchar_t>(0xd800 + (codepoint >> 10)));
				out.append(1, static_cast<wchar_t>(0xdc00 + (codepoint & 0x03ff)));
			}
			else if (codepoint < 0xd800 || codepoint >= 0xe000)
				out.append(1, static_cast<wchar_t>(codepoint));
		}
	}
	return out;
}

std::wstring ReplaceString(std::wstring subject, const std::wstring& search,
	const std::wstring& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

static int round_to_int(double v)
{
	return int(v + ((v < 0.0) ? -0.5 : 0.5));
}




// CWin2DinMFCView

IMPLEMENT_DYNCREATE(CWin2DinMFCView, CMyScrollView)

BEGIN_MESSAGE_MAP(CWin2DinMFCView, CMyScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CWin2DinMFCView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_GESTURE, OnGesture)

END_MESSAGE_MAP()

// CWin2DinMFCView construction/destruction

CWin2DinMFCView::CWin2DinMFCView() noexcept
{
	m_ppm_bitmap_resolution = SCALEPPI(72);
	m_width = m_height = -1;
	g_ddraw_data.Initialize();
	m_dwFrequency = g_ddraw_data.GetFrequency();

}

CWin2DinMFCView::~CWin2DinMFCView()
{
}

BOOL CWin2DinMFCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CMyScrollView::PreCreateWindow(cs);
}

// CWin2DinMFCView drawing

void CWin2DinMFCView::OnDraw(CDC* /*pDC*/)
{
	CWin2DinMFCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here

	if (m_svg != nullptr)
	{
		Redraw(m_width / 4.0f, m_height / 4.0f, 300, 300, (float)m_width, (float)m_height, m_currentDpi);
	}

}


// CWin2DinMFCView printing


void CWin2DinMFCView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CWin2DinMFCView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CWin2DinMFCView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CWin2DinMFCView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CWin2DinMFCView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CWin2DinMFCView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CWin2DinMFCView diagnostics

#ifdef _DEBUG
void CWin2DinMFCView::AssertValid() const
{
	CView::AssertValid();
}

void CWin2DinMFCView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWin2DinMFCDoc* CWin2DinMFCView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWin2DinMFCDoc)));
	return (CWin2DinMFCDoc*)m_pDocument;
}
#endif //_DEBUG


// CWin2DinMFCView message handlers

BOOL CWin2DinMFCView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}


DesktopWindowTarget CWin2DinMFCView::CreateDesktopWindowTarget(Compositor const& compositor, HWND window)
{
	namespace abi = ABI::Windows::UI::Composition::Desktop;

	auto interop = compositor.as<abi::ICompositorDesktopInterop>();
	DesktopWindowTarget target{ nullptr };
	check_hresult(interop->CreateDesktopWindowTarget(window, true, reinterpret_cast<abi::IDesktopWindowTarget**>(put_abi(target))));
	return target;
}


void CWin2DinMFCView::PrepareVisuals(Compositor const& compositor)
{
	m_target = CreateDesktopWindowTarget(compositor, GetSafeHwnd());
	m_root = compositor.CreateSpriteVisual();
	m_root.RelativeSizeAdjustment({ 1.05f, 1.05f });
	m_root.Brush(compositor.CreateColorBrush({ 0x00, 0x00, 0x00 , 0x00 }));
	m_target.Root(m_root);
}


int CWin2DinMFCView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	PrepareVisuals(m_compositor);

	m_currentDpi = GetDpiForWindow(GetSafeHwnd());
	m_ppm_bitmap_resolution = SCALEPPI(72);

	CreateFlameEffect();

	m_text = "";
	m_newText = "Win2d in win32 desktop C++ with MFC";
	return 0;
}

static float GetFontSize(float width)
{
	const float maxFontSize = 72.0f;
	const float scaleFactor = 12.0f;
	return std::min<float>((float)width / scaleFactor, maxFontSize);
}


bool CWin2DinMFCView::Redraw(float cx, float cy, float wx, float wy, float width, float height, UINT dpi)
{
	auto pDoc = GetDocument();

	try {

		if (!pDoc || pDoc->m_svg_xml.size() == 0)
		{
			auto drawingSession0 = CanvasComposition::CreateDrawingSession(m_drawingSurface);
			auto rc = drawingSession0.as< ICanvasResourceCreator>();

			bool new_bitmap = false;
			if (m_myBitmap == nullptr)
			{
				CanvasRenderTarget my_bitmap(rc, width, height, (float)dpi);
				m_myBitmap = my_bitmap;
				new_bitmap = true;
			}
			auto drawingSession = m_myBitmap.CreateDrawingSession();

			auto w = Colors::Black();
			w.A = new_bitmap ? 255 : 20;
			Rect r{ 0, 0, width, height };
			drawingSession.FillRectangle(r, w);

			float2 c(cx + wx, cy + wy);
			auto m = make_float3x2_rotation((float)(m_angle * M_PI / 180.0), c);
			auto md = m * drawingSession.Transform();
			drawingSession.Transform(md);

			Rect r1{ cx, cy, wx, wy };
			Rect r2{ cx + wx, cy + wy, wx, wy };
			winrt::Windows::UI::Color col1 = Colors::Red();

			drawingSession.FillRectangle(r1, col1);
			drawingSession.FillRectangle(r2, Colors::Green());

			winrt::Microsoft::Graphics::Canvas::Text::CanvasTextFormat tf;
			tf.FontSize(m_angle / 2 + 1);
			winrt::hstring t{ L"Hello Win2D in MFC!" };

			Rect rt{ cx, cy, wx * 4, wy * 4 };
			CanvasRenderTarget my_text(rc, (float)wx * 4, (float)wy * 4, (float)dpi);
			auto ds = my_text.CreateDrawingSession();
			auto b = Colors::Black();
			b.A = 0;
			ds.Clear(b);
			ds.DrawText(t, rt, Colors::Blue(), tf);
			ds.Close();

			GaussianBlurEffect gbe;
			gbe.BlurAmount(5);
			gbe.Source(my_text);
			drawingSession.DrawImage(gbe);

			// If the text or font size has changed then recreate the text command list.
			auto newFontSize = GetFontSize(width);
			if (m_newText != m_text || newFontSize != m_fontSize)
			{
				m_text = m_newText;
				m_fontSize = newFontSize;
				SetupText(rc);
			};
			ConfigureEffect();
			float2 center(width / 2.0f, height / 2.0f);
			drawingSession.DrawImage(m_composite, center);
			drawingSession.Close();

			drawingSession0.DrawImage(m_myBitmap);
			drawingSession0.Close();

			if (m_angle == 360.0f)
				m_angle = 0.0f;
		}
		else if (pDoc)
		{
			if (m_svg)
			{
				auto drawingSession0 = CanvasComposition::CreateDrawingSession(m_drawingSurface);
				auto rc = drawingSession0.as< ICanvasResourceCreator>();

				drawingSession0.Antialiasing(CanvasAntialiasing::Antialiased);
				drawingSession0.Transform(m_transform);

				winrt::Windows::Foundation::Size size((float)m_width, (float)m_height);
				drawingSession0.Clear(Colors::White());
				drawingSession0.DrawSvg(m_svg, size);
				drawingSession0.Close();
			}
		}
	}
	catch (winrt::hresult_error const& /*ex*/)
	{
		return false;
	}
	return true;
}


bool CWin2DinMFCView::LoadSvg()
{
	auto pDoc = GetDocument();

	//we except UTF-8
	// first convert to wstring
	std::wstring w;
	if (pDoc->m_svg_xml[0] == -1 && pDoc->m_svg_xml[1] == -2)
	{
		//vector data is unicode
		w = std::wstring(((wchar_t*)&pDoc->m_svg_xml[0]) + 1, pDoc->m_svg_xml.size() / 2 - 2);
	}
	else
	{
		w = UTF8_to_wchar(&pDoc->m_svg_xml[0]);
		w = ReplaceString(w, L"encoding=\"utf-8", L"encoding=\"utf-16");
		w = ReplaceString(w, L"encoding=\"UTF-8", L"encoding=\"UTF-16");
	}
	m_w = winrt::hstring(w);

	try {
		if (m_svg != nullptr)
			m_svg.Close();
		m_svg = nullptr;
		auto canvasDevice = CanvasDevice::GetSharedDevice();
		m_svg = CanvasSvgDocument::LoadFromXml(canvasDevice, m_w);
	}
	catch (winrt::hresult_error & ex)
	{
		CString err;
		auto m = ex.message();
		std::wstring c = m.c_str();
		err.Format(L"Error loading SVG: %ls", c.c_str());
		AfxMessageBox(err);

		pDoc->m_svg_xml.clear();
	}

	if (m_svg)
	{

		auto root_element = m_svg.Root();

		if (root_element)
		{
			try {
				auto width = root_element.GetFloatAttribute(L"width");
				auto height = root_element.GetFloatAttribute(L"height");

				m_svg_width = width;
				m_svg_height = height;

			}
			catch (winrt::hresult_error&)
			{
				Rect viewBox;
				try {
					viewBox = root_element.GetRectangleAttribute(L"viewBox");

					m_svg_width = viewBox.Width;
					m_svg_height = viewBox.Height;

				}
				catch (winrt::hresult_error&)
				{
				}
			}
		}
#if 0

		com_ptr<ABI::Microsoft::Graphics::Canvas::ICanvasResourceWrapperNative> nativeDeviceWrapper = m_svg.as<ABI::Microsoft::Graphics::Canvas::ICanvasResourceWrapperNative>();
		com_ptr<ID2D1SvgDocument> pSvgDoc{ nullptr };
		check_hresult(nativeDeviceWrapper->GetNativeResource(nullptr, 0.0f, guid_of<ID2D1SvgDocument>(), pSvgDoc.put_void()));
		auto size = pSvgDoc->GetViewportSize();
#endif
		CSize total((int)m_svg_width, (int)m_svg_height);
		SetScrollSizes(MM_TEXT, total);
		
		CPoint scroll_pos(0, 0);
		ScrollToPosition(scroll_pos);

		m_transform.m31 = 0;
		m_transform.m32 = 0;

		m_transform.identity();
	}



	return true;
}


winrt::com_ptr<::IDXGIDevice> GetDXGIDevice(CanvasDevice & device)
{
	//First we need to get an ID2D1Device1 pointer from the shared CanvasDevice
	com_ptr<ABI::Microsoft::Graphics::Canvas::ICanvasResourceWrapperNative> nativeDeviceWrapper = device.as<ABI::Microsoft::Graphics::Canvas::ICanvasResourceWrapperNative>();
	com_ptr<ID2D1Device2> pDevice{ nullptr };
	check_hresult(nativeDeviceWrapper->GetNativeResource(nullptr, 0.0f, guid_of<ID2D1Device2>(), pDevice.put_void()));

	IDXGIDevice * pDXGIDevice;
	pDevice->GetDxgiDevice(&pDXGIDevice);

	winrt::com_ptr<::IDXGIDevice> dxgiDevice;
	dxgiDevice.attach(pDXGIDevice);
	return dxgiDevice;
}

void CWin2DinMFCView::OnDirect3DDeviceLost(DeviceLostHelper const* /* sender */, DeviceLostEventArgs const& /* args */)
{
	m_b_in_device_lost = true;
	m_cbt.stop();
	auto canvasDevice = CanvasDevice::GetSharedDevice();
	winrt::com_ptr<abi::ICompositionGraphicsDeviceInterop> compositionGraphicsDeviceInterop{ m_graphicsDevice.as<abi::ICompositionGraphicsDeviceInterop>() };

	com_ptr<ABI::Microsoft::Graphics::Canvas::ICanvasResourceWrapperNative> nativeDeviceWrapper = canvasDevice.as<ABI::Microsoft::Graphics::Canvas::ICanvasResourceWrapperNative>();
	com_ptr<ID2D1Device2> pDevice{ nullptr };
	check_hresult(nativeDeviceWrapper->GetNativeResource(nullptr, 0.0f, guid_of<ID2D1Device2>(), pDevice.put_void()));

	winrt::check_hresult(compositionGraphicsDeviceInterop->SetRenderingDevice(pDevice.get()));

	m_drawingSurface.Close();
	m_drawingSurface = nullptr ;
	m_myBitmap.Close();
	m_myBitmap = nullptr ;
	if (m_svg != nullptr)
	{
		m_svg.Close();
		m_svg = nullptr;
	}
	m_b_in_device_lost = false;
	Scenario_Wind2d(m_compositor, m_root, m_currentDpi, m_width, m_height);

}




void CWin2DinMFCView::Scenario_Wind2d(const Compositor & compositor, const ContainerVisual & root, UINT dpi, int cx, int cy)
{
	if (m_b_in_device_lost)
		return;
	// Configure a container visual.
	if (m_width != cx || m_height != cy || m_drawingSurface ==nullptr)
	{
		m_cbt.stop();
		m_width = cx, m_height = cy;
		try {

			SpriteVisual container{ nullptr };
			auto c = root.Children();
			if (c.Count() > 0)
			{
				//c.
				auto obj = c.First().Current();
				container = obj.as< SpriteVisual>();
				container.Size({ (float)m_width, (float)m_height });
			}
			else
			{
				container = compositor.CreateSpriteVisual();
				container.Size({ (float)m_width, (float)m_height });
				c.InsertAtTop(container);
			}

			//container.Offset({ 0.0f, 900.0f, 1.0f });

			if (m_drawingSurface != nullptr)
			{
				winrt::Windows::Graphics::SizeInt32 s{ m_width, m_height };
				m_drawingSurface.Resize(s);
				// Create a drawing surface brush.
				CompositionSurfaceBrush brush = compositor.CreateSurfaceBrush(m_drawingSurface);
				container.Brush(brush);

			}

			if (m_drawingSurface == nullptr)
			{
				// Get a canvas device.
				m_canvasDevice = CanvasDevice::GetSharedDevice();

				// Create the Direct2D device object.
						// Obtain the underlying DXGI device of the Direct3D device.

				auto dxgi_device = ::GetDXGIDevice(m_canvasDevice);
				m_devicelost_helper.WatchDevice(dxgi_device);
				m_devicelost_helper.DeviceLost({ this, &CWin2DinMFCView::OnDirect3DDeviceLost });

				if (m_graphicsDevice == nullptr)
					m_graphicsDevice = CanvasComposition::CreateCompositionGraphicsDevice(compositor, m_canvasDevice);

				// Create a drawing surface.
				m_drawingSurface = m_graphicsDevice.CreateDrawingSurface(
					Size((float)m_width, (float)m_height),
					winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
					winrt::Windows::Graphics::DirectX::DirectXAlphaMode::Premultiplied);

				// Create a drawing surface brush.
				CompositionSurfaceBrush brush = compositor.CreateSurfaceBrush(m_drawingSurface);
				container.Brush(brush);
			}

			if (m_myBitmap != nullptr)
				m_myBitmap.Close();
			m_myBitmap = nullptr;

			CreateFlameEffect();
			m_text = "";

			Redraw(m_width / 4.0f, m_height / 4.0f, 300, 300, (float)m_width, (float)m_height, dpi);

		}
		catch (winrt::hresult_error const& ex)
		{
			m_cbt.stop();
		}
	}

	//Redraw(m_width / 4.0f, m_height / 4.0f, 300, 300, (float)m_width, (float)m_height, dpi);

	if (m_svg == nullptr)
	{
		m_cbt.start(1000.0 / 60.0, [this, dpi]
			{
				if (Redraw(m_width / 4.0f, m_height / 4.0f, 300, 300, (float)m_width, (float)m_height, dpi) == false)
					return false;
				m_angle += 1.0f;
				return true;
			});
	}
}




void CWin2DinMFCView::OnInitialUpdate()
{
	CMyScrollView::OnInitialUpdate();

	auto pDoc = GetDocument();
	if (!pDoc || pDoc->m_svg_xml.size() == 0)
	{
		if (m_svg != nullptr)
		{
			m_svg.Close();
			m_svg = nullptr;
		}
		Scenario_Wind2d(m_compositor, m_root, m_currentDpi, 1400, 1000);

		CSize total(m_width, m_height);
		SetScrollSizes(MM_TEXT, total);
		CPoint scroll_pos(0, 0);
		ScrollToPosition(scroll_pos);
		m_transform.m31 = 0;
		m_transform.m32 = 0;

	}
	else
	{
		m_cbt.stop();

		if (m_svg != nullptr) {
			m_svg.Close();
			m_svg = nullptr;
		}

		if (m_svg == nullptr && pDoc->m_svg_xml.size() > 0)
		{
			if (!LoadSvg())
			{
			}
		}

		Redraw(m_width / 4.0f, m_height / 4.0f, 300, 300, (float)m_width, (float)m_height, m_currentDpi);
	}

	

}


void CWin2DinMFCView::OnSize(UINT nType, int cx, int cy)
{
	CMyScrollView::OnSize(nType, cx, cy);
	if (cx > 0 && cy > 0)
		Scenario_Wind2d(m_compositor, m_root, m_currentDpi, cx, cy);
}


void CWin2DinMFCView::CreateFlameEffect()
{
	// Thicken the text.
	m_morphology = MorphologyEffect();
	// The Source property is set by SetupText().
	m_morphology.Mode(MorphologyEffectMode::Dilate);
	m_morphology.Width(7);
	m_morphology.Height(1);

	// Blur, then colorize the text from black to red to orange as the alpha increases.
	auto gbe = GaussianBlurEffect();
	gbe.Source(m_morphology);
	gbe.BlurAmount(3.0f);
	Matrix5x4 m;
	m.M11 = 0.0f, m.M12 = 0.0f, m.M13 = 0.0f, m.M14 = 0.0f,
		m.M21 = 0.0f, m.M22 = 0.0f, m.M23 = 0.0f, m.M24 = 0.0f,
		m.M31 = 0.0f, m.M32 = 0.0f, m.M33 = 0.0f, m.M34 = 0.0f,
		m.M41 = 0.0f, m.M42 = 1.0f, m.M43 = 0.0f, m.M44 = 1.0f,
		m.M51 = 1.0f, m.M52 = -0.5f, m.M53 = 0.0f, m.M54 = 0.0f;

	auto colorize = ColorMatrixEffect();
	colorize.Source(gbe);
	colorize.ColorMatrix(m);

	// Generate a Perlin noise field (see flamePosition).
	// Animate the noise by modifying flameAnimation's transform matrix at render time.

	TurbulenceEffect te;
	te.Frequency(winrt::Windows::Foundation::Numerics::float2(0.109f, 0.109f));
	te.Size(winrt::Windows::Foundation::Numerics::float2(500.0f, 80.0f));

	BorderEffect be;
	be.Source(te);
	// Use Mirror extend mode to allow us to spatially translate the noise
	// without any visible seams.
	be.ExtendX(CanvasEdgeBehavior::Mirror);
	be.ExtendY(CanvasEdgeBehavior::Mirror);

	m_flameAnimation = Transform2DEffect();
	m_flameAnimation.Source(be);

	// Give the flame its wavy appearance by generating a displacement map from the noise
	// (see flameAnimation) and applying this to the text.
	// Stretch and position this flame behind the original text.
	DisplacementMapEffect de;
	de.Source(colorize);
	de.Displacement(m_flameAnimation);
	de.Amount(40.0f);

	m_flamePosition = Transform2DEffect();
	m_flamePosition.Source(de);
	// Set the transform matrix at render time as it depends on window size.
	
	// Composite the text over the flames.
	m_composite = CompositeEffect();
	m_composite.Sources().Append(m_flamePosition);
	m_composite.Sources().Append(nullptr);
}

void CWin2DinMFCView::SetupText(ICanvasResourceCreator resourceCreator)
{
	CanvasCommandList textCommandList(resourceCreator);
	auto ds = textCommandList.CreateDrawingSession();
	ds.Clear(Color{ 0, 0, 0, 0 });

	CanvasTextFormat tf;
	tf.FontFamily(L"Segoe UI");
	tf.FontSize(m_fontSize);
	tf.HorizontalAlignment(CanvasHorizontalAlignment::Center);
	tf.VerticalAlignment(CanvasVerticalAlignment::Top);
	

	winrt::hstring t;
	t = winrt::to_hstring(m_text);

	ds.DrawText(t, 0, 0, Colors::White(), tf);
	ds.Close();

	// Hook up the command list to the inputs of the flame effect graph.
	m_morphology.Source(textCommandList);
	m_composite.Sources().SetAt(1,textCommandList);
}

void CWin2DinMFCView::ConfigureEffect()
{
	// Animate the flame by shifting the Perlin noise upwards (-Y) over time.
	m_flameAnimation.TransformMatrix(make_float3x2_translation(0, -((float)60.0f * ::clock() / CLOCKS_PER_SEC)));

	// Scale the flame effect 2x vertically, aligned so it starts above the text.
	float verticalOffset = m_fontSize * 1.4f;

	auto centerPoint = float2(0, verticalOffset);
	m_flamePosition.TransformMatrix(make_float3x2_scale(1, 2, centerPoint));
}


BOOL CWin2DinMFCView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	Zoom((zDelta < 0 ? 1 : -1) * m_ppm_bitmap_resolution * 25 / 4, pt);
	//Zoom(zDelta*4, pt);
	return 0;
}


void CWin2DinMFCView::OnLButtonDown(UINT nFlags, CPoint point)
{
	//KillTimer(456);
	m_scroll_diff = 0;
	SetFocus();
	BOOL bH, bV;
	CheckScrollBars(bH, bV);
	if (GetCapture() != this)
	{
		SetCapture();
		m_bTranslateDragging = true;
		m_pt_cur_mouse = point;
		m_scroll_start_time = ::clock();
//		::SetCursor(GetGitterApp()->m_cursorMove);
	}
	CMyScrollView::OnLButtonDown(nFlags, point);
}



void CWin2DinMFCView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bTranslateDragging)
	{
		ReleaseCapture();
		m_bTranslateDragging = false;

		if (m_scroll_time_diff > 6 && m_scroll_diff != CSize(0, 0))
		{
			m_bIdleTranslateDragging = true;
			int milli_seconds = m_scroll_time_diff * 1000 / CLOCKS_PER_SEC;
			int dt = 1000 / (m_dwFrequency);

			int dx = m_scroll_diff.cx;
			int dy = m_scroll_diff.cy;
			int f = m_dwFrequency * milli_seconds;

			dx = (dx * 1000) / f;
			dy = (dy * 1000) / f;
			//				LogMessage(0, "%d, %d, %d %d %d (%d %d) %d", dt, dx, dy, m_dwFrequency, milli_seconds, m_scroll_diff.cx, m_scroll_diff.cy, m_scroll_time_diff);

			m_scroll_diff.cx = dx;
			m_scroll_diff.cy = dy;
			g_ddraw_data.WaitForVSync();
			SetTimer(456, dt, (void(__stdcall*)(HWND, UINT, UINT_PTR, DWORD)) & CWin2DinMFCView::TimerProc);

		}
	}


	CMyScrollView::OnLButtonUp(nFlags, point);
}

void CALLBACK CWin2DinMFCView::TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	CWin2DinMFCView* pThis = (CWin2DinMFCView*)CWnd::FromHandle(hwnd);
	if (pThis && idEvent == 456)
	{
		CPoint p, q;
		q = p = pThis->GetScrollPosition();
		p = p + pThis->m_scroll_diff;
		if (g_m_layoutdiagram_damp_scrolling)
		{
			float cx = (float)pThis->m_scroll_diff.cx;
			float cy = (float)pThis->m_scroll_diff.cy;
			float sign_cx = cx >= 0.0f ? 1.0f : -1.0f;
			float sign_cy = cy >= 0.0f ? 1.0f : -1.0f;
			float _cx = cx;
			float _cy = cy;
#if 0
			const float pi = 3.141592653589793238462643383279f;
			if (cx != 0.0f)
				_cx -= sign_cx * sin(fabs(cx) * pi / 180.0f) / fabs(cx);
			if (cy != 0.0f)
				_cy -= sign_cy * sin(fabs(cy) * pi / 180.0f) / fabs(cy);
#endif
			if (_cx != 0.0f)
				_cx -= sign_cx;
			if (_cy != 0.0f)
				_cy -= sign_cy;
			pThis->m_scroll_diff = CSize((int)_cx, (int)_cy);
			//			LogMessage(0,"cx: %.2f, cy: %.2f; _cx: %.2f, _cy: %.2f", cx, cy, _cx, _cy);
		}

		g_ddraw_data.WaitForVSync();

		pThis->ScrollToPosition(p);

		pThis->m_transform.m31 = (float)-p.x;
		pThis->m_transform.m32 = (float)-p.y;

		pThis->UpdateWindow();
		p = pThis->GetScrollPosition();


		if (p == q)
		{
			pThis->m_scroll_diff = CSize(0, 0);
			pThis->KillTimer(456);
		}
	}
}





void CWin2DinMFCView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_bTranslateDragging)
	{
		clock_t c = ::clock();
		m_scroll_time_diff = c - m_scroll_start_time;
		m_scroll_start_time = c;

		CPoint p0, p;
		p0 = p = GetScrollPosition();
		CSize s = point - m_pt_cur_mouse;
		//		s.cy = -s.cy;
		p -= s;
		if (p.x < 0) p.x = 0;
		if (p.y < 0) p.y = 0;
		m_pt_cur_mouse = point;
		BOOL bH, bV;
		CheckScrollBars(bH, bV);
		if (!bH)
			p.x = 0;
		if (!bV)
			p.y = 0;

		m_scroll_diff = p - p0;

		if (m_scroll_diff != CSize(0, 0))
		{
			ScrollToPosition(p);

			m_transform.m31 = (float)-p.x;
			m_transform.m32 = (float)-p.y;

			Invalidate();
		}
	}
	//CMyScrollView::OnMouseMove(nFlags, point);
}

void CWin2DinMFCView::Zoom(int zDelta, CPoint pt)
{
	KillTimer(456);

	// TODO: Add your message handler code here and/or call default
	int _dpi25 = SCALEPPI(25);;
	m_ppm_bitmap_resolution -= zDelta / _dpi25;
	TRACE("zDelta: %d, res: %d\n", zDelta, m_ppm_bitmap_resolution);
	if (m_ppm_bitmap_resolution < SCALEPPI(36))
		m_ppm_bitmap_resolution = SCALEPPI(36);
	if (m_ppm_bitmap_resolution > SCALEPPI(ppm_bitmap_MAX_resolution))
		m_ppm_bitmap_resolution = SCALEPPI(ppm_bitmap_MAX_resolution);


	float scale = m_ppm_bitmap_resolution / PPI(72.0f);
	float2 centerPoint((float)pt.x, (float)pt.y);
	m_transform.m11 = scale;
	m_transform.m22 = scale;

	scale = m_transform.m11;

	m_scroll_diff = 0;

	CPoint mp = pt;
	ScreenToClient(&mp);
	RECT r;
	GetClientRect(&r);
	CPoint p;
	p = GetScrollPosition();
	mp += p;
	CSize total = GetTotalSize();

	float frac_x = (float)mp.x / (total.cx);
	float frac_y = (float)mp.y / (total.cy);

	total = CSize((int)(m_svg_width* scale), (int)(m_svg_height * scale));

	if (total.cx > 100000000)
		total.cx = 100000000;
	if (total.cy > 100000000)
		total.cy = 100000000;

	if (total.cx < 0)
		total.cx = 0;
	if (total.cy < 0)
		total.cy = 0;


	CPoint mp2;
	mp2.x = round_to_int(frac_x * (total.cx));
	mp2.y = round_to_int(frac_y * (total.cy));
	p += (mp2 - mp);
	BOOL bH, bV;
//	CheckScrollBars(bH, bV);
	if (total.cx <= (r.right - r.left))
		p.x = 0;
	if (total.cy <= (r.bottom - r.top))
		p.y = 0;
	//	p.x = p.y = 0;
	m_transform.m31 = (float)-p.x;
	m_transform.m32 = (float)-p.y;
	SetScrollSizes(MM_TEXT, total);
	ScrollToPosition(p);
	Invalidate();
}

BOOL CWin2DinMFCView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	auto r = CMyScrollView::OnScrollBy(sizeScroll, bDoScroll);

	m_transform.m31 = (float)-m_curScrollPos.x;
	m_transform.m32 = (float)-m_curScrollPos.y;

	return r;
}

ULONG CWin2DinMFCView::GetGestureStatus(CPoint /*ptTouch*/)
{
	return 0;
}


static int cur_dist = 0;
static bool gesture_start = false;

LRESULT CWin2DinMFCView::OnGesture(WPARAM w, LPARAM lParam)
{
	// Create a structure to populate and retrieve the extra message info.
	GESTUREINFO gi;

	ZeroMemory(&gi, sizeof(GESTUREINFO));

	gi.cbSize = sizeof(GESTUREINFO);

	BOOL bResult = GetGestureInfo((HGESTUREINFO)lParam, &gi);
	BOOL bHandled = FALSE;
	if (bResult) {
		// now interpret the gesture
		switch (gi.dwID) {

		case GID_BEGIN:
			gesture_start = true;
			break;
		case GID_ZOOM:
			// Code for panning goes here
		{
			if (!gesture_start)
			{
				int d = (int)gi.ullArguments - cur_dist;
				if (d != 0)
				{
					int delta = d;

					double _d = (delta < 0 ? 1 : -1) * (double)m_ppm_bitmap_resolution * 25 * (fabs(d) / 120.0) / 4;
					Zoom((int)_d, CPoint(gi.ptsLocation.x, gi.ptsLocation.y));
				}
			}
			gesture_start = false;
			cur_dist = (int)gi.ullArguments;

		}
		bHandled = TRUE;
		break;
		default:
			// A gesture was not recognized
			break;
		}
	}
	else {
		DWORD dwErr = GetLastError();
		if (dwErr > 0) {
			//MessageBoxW(hWnd, L"Error!", L"Could not retrieve a GESTUREINFO structure.", MB_OK);
		}
	}
	if (bHandled) {
		return 1;
	}
	else {
		return DefWindowProc(WM_GESTURE, w, lParam);
	}
}

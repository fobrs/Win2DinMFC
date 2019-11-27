#include "stdafx.h"


#include <D2d1_1.h>
#include <D3d11_4.h>
#include <Dwrite.h>
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>
#include <Windows.ui.composition.interop.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.UI.Composition.h>



using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Composition;
using namespace winrt::Windows::UI::Core;


#include "devicelost.h"




DeviceLostHelper::~DeviceLostHelper()
{
	StopWatchingCurrentDevice();
	m_onDeviceLostHandler = nullptr;
}


void DeviceLostHelper::WatchDevice(winrt::com_ptr<::IDXGIDevice> const& dxgiDevice)
{
	// If we're currently listening to a device, then stop.
	StopWatchingCurrentDevice();

	// Set the current device to the new device.
	m_device = nullptr;
	winrt::check_hresult(::CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), reinterpret_cast<::IInspectable**>(winrt::put_abi(m_device))));

	// Get the DXGI Device.
	m_dxgiDevice = dxgiDevice;

	// QI For the ID3D11Device4 interface.
	winrt::com_ptr<::ID3D11Device4> d3dDevice{ m_dxgiDevice.as<::ID3D11Device4>() };

	// Create a wait struct.
	m_onDeviceLostHandler = nullptr;
	m_onDeviceLostHandler = ::CreateThreadpoolWait(DeviceLostHelper::OnDeviceLost, (PVOID)this, nullptr);

	// Create a handle and a cookie.
	m_eventHandle.attach(::CreateEvent(nullptr, false, false, nullptr));
	winrt::check_bool(bool{ m_eventHandle });
	m_cookie = 0;

	// Register for device lost.
	::SetThreadpoolWait(m_onDeviceLostHandler, m_eventHandle.get(), nullptr);
	winrt::check_hresult(d3dDevice->RegisterDeviceRemovedEvent(m_eventHandle.get(), &m_cookie));
}

void DeviceLostHelper::StopWatchingCurrentDevice()
{
	if (m_dxgiDevice && m_onDeviceLostHandler != nullptr)
	{
		// QI For the ID3D11Device4 interface.
		auto d3dDevice{ m_dxgiDevice.as<::ID3D11Device4>() };

		// Unregister from the device lost event.
		::CloseThreadpoolWait(m_onDeviceLostHandler);
		d3dDevice->UnregisterDeviceRemoved(m_cookie);

		// Clear member variables.
		m_onDeviceLostHandler = nullptr;
		m_eventHandle.close();
		m_cookie = 0;
		m_device = nullptr;
	}
}

void DeviceLostHelper::DeviceLost(winrt::delegate<DeviceLostHelper const*, DeviceLostEventArgs const&> const& handler)
{
	m_deviceLost = handler;
}


void DeviceLostHelper::RaiseDeviceLostEvent(IDirect3DDevice const& oldDevice)
{
	m_deviceLost(this, DeviceLostEventArgs::Create(oldDevice));
}

void CALLBACK DeviceLostHelper::OnDeviceLost(PTP_CALLBACK_INSTANCE /* instance */, PVOID context, PTP_WAIT /* wait */, TP_WAIT_RESULT /* waitResult */)
{
	auto deviceLostHelper = reinterpret_cast<DeviceLostHelper*>(context);
	auto oldDevice = deviceLostHelper->m_device;
	deviceLostHelper->StopWatchingCurrentDevice();
	deviceLostHelper->RaiseDeviceLostEvent(oldDevice);
}



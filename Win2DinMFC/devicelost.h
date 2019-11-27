#pragma once

#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;

namespace abi
{
	using namespace ABI::Windows::UI::Composition;
}

struct DeviceLostEventArgs
{
	DeviceLostEventArgs(IDirect3DDevice const& device) : m_device(device) {}
	IDirect3DDevice Device() { return m_device; }
	static DeviceLostEventArgs Create(IDirect3DDevice const& device) { return DeviceLostEventArgs{ device }; }

private:
	IDirect3DDevice m_device;
};

struct DeviceLostHelper
{
	DeviceLostHelper() = default;
	~DeviceLostHelper();

	IDirect3DDevice CurrentlyWatchedDevice() { return m_device; }

	void WatchDevice(winrt::com_ptr<::IDXGIDevice> const& dxgiDevice);
	void StopWatchingCurrentDevice();
	void DeviceLost(winrt::delegate<DeviceLostHelper const*, DeviceLostEventArgs const&> const& handler);
	winrt::delegate<DeviceLostHelper const*, DeviceLostEventArgs const&> m_deviceLost;

private:
	void RaiseDeviceLostEvent(IDirect3DDevice const& oldDevice);
	static void CALLBACK OnDeviceLost(PTP_CALLBACK_INSTANCE /* instance */, PVOID context, PTP_WAIT /* wait */, TP_WAIT_RESULT /* waitResult */);

private:
	IDirect3DDevice m_device;
	winrt::com_ptr<::IDXGIDevice> m_dxgiDevice;
	PTP_WAIT m_onDeviceLostHandler{ nullptr };
	winrt::handle m_eventHandle;
	DWORD m_cookie{ 0 };
};

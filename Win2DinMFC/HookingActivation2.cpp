
#include <windows.h>
#include <winstring.h>
#include <winrt/Windows.Foundation.h>

#ifdef _M_IX86
#pragma comment(linker, "/alternatename:_OS_RoGetActivationFactory@12=_RoGetActivationFactory@12")
#else
#pragma comment(linker, "/alternatename:OS_RoGetActivationFactory=RoGetActivationFactory")
#endif


extern "C"
{
	HRESULT __stdcall OS_RoGetActivationFactory(HSTRING classId, GUID const& iid, void** factory) noexcept;
}

bool starts_with(std::wstring_view value, std::wstring_view match) noexcept
{
	return 0 == value.compare(0, match.size(), match);
}
#ifndef _M_IX86
extern "C" HRESULT __stdcall WINRT_RoGetActivationFactory2(HSTRING classId, GUID const& iid, void** factory) noexcept
{
#else
extern "C"  HRESULT WINRT_RoGetActivationFactory2(void * _classId, GUID const & iid, void ** factory) noexcept
{
	HSTRING classId = (HSTRING)_classId;
#endif
	*factory = nullptr;
	std::wstring_view name( WindowsGetStringRawBuffer(classId, nullptr), WindowsGetStringLen(classId) );
	HMODULE library{ nullptr };

#if 1
	if (starts_with(name, L"Microsoft.Graphics.Canvas."))
	{
		library = LoadLibraryW(L"Microsoft.Graphics.Canvas.dll");
	}
	else if (starts_with(name, L"ComponentB."))
	{
		library = LoadLibraryW(L"ComponentB.dll");
	}
	else
	{
		return OS_RoGetActivationFactory(classId, iid, factory);
	}
#endif
	if (!library)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	using DllGetActivationFactory = HRESULT __stdcall(HSTRING classId, void** factory);
	auto call = reinterpret_cast<DllGetActivationFactory*>(GetProcAddress(library, "DllGetActivationFactory"));

	if (!call)
	{
		HRESULT const hr = HRESULT_FROM_WIN32(GetLastError());
		WINRT_VERIFY(FreeLibrary(library));
		return hr;
	}

	winrt::com_ptr<winrt::Windows::Foundation::IActivationFactory> activation_factory;
	HRESULT const hr = call(classId, activation_factory.put_void());

	if (FAILED(hr))
	{
		WINRT_VERIFY(FreeLibrary(library));
		return hr;
	}
	GUID iid2 = winrt::guid_of<winrt::Windows::Foundation::IActivationFactory>();
	if (iid != iid2)
	{
		return activation_factory->QueryInterface(iid, factory);
	}

	*factory = activation_factory.detach();
	return S_OK;
}

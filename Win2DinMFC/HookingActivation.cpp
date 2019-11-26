
#include <windows.h>

extern "C"
{
	HRESULT WINRT_RoGetActivationFactory2(void * classId, GUID const& iid, void** factory) noexcept;

}

#ifdef _M_IX86

extern "C" HRESULT __stdcall WINRT_RoGetActivationFactory(void * classId, GUID const& iid, void** factory) noexcept
{
	return WINRT_RoGetActivationFactory2(classId, iid, factory);
}

#else

extern "C" HRESULT  __stdcall WINRT_RoGetActivationFactory(void * classId, GUID const& iid, void** factory) noexcept
{
	return WINRT_RoGetActivationFactory2(classId, iid, factory);
}

#endif



// DirectDraw

#define DIRECTDRAW_VERSION 0x0700 // specify version 7.0 of DirectDraw
#define INITGUID 
#include <ddraw.h>

#include "Ddraw_global.h"


#if 0
	dd_last_error = DD_OK;
	dd_was_lost = false;

	HRESULT status = DirectDrawCreateEx(NULL, (void **) &dd_main, IID_IDirectDraw7, NULL);

	if(FAILED(status)) {
		DirectDrawError(status);
		win_status = GAMEX_DDSTART_FAILED;
		return false;
	}
#endif

static bool m_init;
static LPDIRECTDRAW7 m_dd;
static HINSTANCE m_hdd;
static DWORD m_dwFrequency;


ddraw_data_t::ddraw_data_t() 
{
	m_init = false;
	m_dd = 0;
	m_hdd = 0;
	m_dwFrequency = 60;
};

void ddraw_data_t::Initialize()
{
	if (!m_init)
	{
		m_hdd = LoadLibrary(L"ddraw.dll"); 
		if (m_hdd != 0) 
		{
			typedef HRESULT (WINAPI *DIRECTDRAWCREATEEX)(GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter );
			DIRECTDRAWCREATEEX ddc = (DIRECTDRAWCREATEEX)GetProcAddress(m_hdd,"DirectDrawCreateEx"); 
			if (ddc!= 0) {
				HRESULT r = ddc(0, (LPVOID  *)&m_dd, IID_IDirectDraw7, 0); 
				if (!FAILED(r))
				{
					r = m_dd->GetMonitorFrequency(&m_dwFrequency);
#if 1
					if (m_dd != 0)
					{
						m_dd->Release();
						m_dd = 0;
					}
#endif
				}
			}
		} 
		m_init = true;
	}
}

ddraw_data_t::~ddraw_data_t()
{
	if (m_dd != 0)
		m_dd->Release();
	m_dd = 0;
	if (m_hdd != 0)
		FreeLibrary(m_hdd);
	m_hdd = 0; 
};


void ddraw_data_t::WaitForVSync()
{
	if (m_dd != 0)
		m_dd->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
}

DWORD ddraw_data_t::GetFrequency()
{
	return m_dwFrequency;
}

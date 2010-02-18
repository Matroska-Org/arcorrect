// ARCorrect.cpp : Defines the initialization routines for the DLL.
//

#include <initguid.h>
#include "ARCorrect_GUID.h"

#include "ARCorrect.h"

const AMOVIESETUP_FILTER sudARCorrectFilter =
{
    &CLSID_ARCorrectFilter,   // clsID
    L"ARCorrect Filter",    // strName
    MERIT_UNLIKELY,         // dwMerit
    0,                        // nPins
    0                         // lpPin
};

// Global data
CFactoryTemplate g_Templates[]= {
   {
		L"ARCorrect Filter", 
		&CLSID_ARCorrectFilter, 
		CARCorrectFilter::CreateInstance, 
		NULL, 
		&sudARCorrectFilter
	 },   
	{ 
    L"ARCorrect Filter Property Page",
    &CLSID_ARCorrectFilter_Page,
    CARCorrectFilterPage::CreateInstance,
    NULL,
    NULL
  },	 
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	DbgSetModuleLevel(LOG_ERROR,5);
	DbgSetModuleLevel(LOG_TRACE,5); // comment this to remove trace
	//DbgSetModuleLevel(LOG_MEMORY,2);
	DbgSetModuleLevel(LOG_LOCKING,2);
	//DbgSetModuleLevel(LOG_TIMING,5);

	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

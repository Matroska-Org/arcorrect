
#ifndef _CAR_CORRECT_FILTER_PAGE_H_
#define _CAR_CORRECT_FILTER_PAGE_H_

#include <windows.h>
#include <streams.h>
#include <commctrl.h>

#include "ARCorrect_GUID.h"
#include "resource.h"

class CARCorrectFilterPage : public CBasePropertyPage {
public:
	CARCorrectFilterPage(LPUNKNOWN pUnk, HRESULT *pHr);
	~CARCorrectFilterPage();	

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr);

protected:
	BOOL OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	HRESULT OnConnect(IUnknown *pUnknown);
	HRESULT OnDisconnect();

	IUnknown *m_pFilter;
};

#endif // _CAR_CORRECT_FILTER_PAGE_H_

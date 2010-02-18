
#include "ARCorrectPage.h"

CARCorrectFilterPage::CARCorrectFilterPage(LPUNKNOWN pUnk, HRESULT *pHr)
	: CBasePropertyPage(TEXT("ARCorrect Page"), pUnk, IDD_ARCORRECT_PAGE, IDS_ARCORRECT_PAGE_TITLE)
	, m_pFilter(NULL)
{
	InitCommonControls();
}

CARCorrectFilterPage::~CARCorrectFilterPage() {
	
};

HRESULT CARCorrectFilterPage::OnConnect(IUnknown *pUnk) {
	if (pUnk == NULL) {
		return E_POINTER;
	}
	HRESULT hr = S_OK;
	/*ASSERT(m_pFilter == NULL);
	HRESULT hr = pUnk->QueryInterface(IID_IDSCryptDecryptorFilter, reinterpret_cast<void**>(&m_pFilter));
	if (hr == S_OK) {
		m_pFilter->GetEncrytionMethod((void **)&m_EncrytionMethod);		
	}*/
	return hr;
};

HRESULT CARCorrectFilterPage::OnDisconnect() {
	// Release of Interface after setting the appropriate contrast value
	if (!m_pFilter)
		return E_UNEXPECTED;

	m_pFilter->Release();
	m_pFilter = NULL;
	
	return NOERROR;
}

BOOL CARCorrectFilterPage::OnReceiveMessage(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			break;
		}
	}	
	return (BOOL)CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

CUnknown * WINAPI CARCorrectFilterPage::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr)
{
	return new CARCorrectFilterPage(pUnk, pHr);
}


#ifndef _ARCORRECT_FILTER_H_
#define _ARCORRECT_FILTER_H_

#include <windows.h>
#include <streams.h>
#include <Dvdmedia.h>
#include <string.h>
#include <Shlwapi.h>

#include "ARCorrectPage.h"
#include "ARCorrect_GUID.h"
#include "DShowUtils.h"
#include "ITrackInfo.h"
#include "PersistPropertyBag.h"

#if 0
class CARCorrectFilterOutputPin : public CTransInPlaceOutputPin
{
public:
	CARCorrectFilterOutputPin(TCHAR *pObjectName, CTransInPlaceFilter *pFilter, HRESULT * phr, LPCWSTR pPinName);

	virtual HRESULT CheckMediaType(const CMediaType *pmt);
};
#endif

class CARCorrectFilterOutputPin : public CTransformOutputPin
{
public:	
	HRESULT PassSample(IMediaSample *pSample) { return m_pInputPin->Receive(pSample); };
};

class CARCorrectFilter : public CTransformFilter, public ISpecifyPropertyPages, public CPersistPropertyBag, protected CRegistryStuff
{
public:
	CARCorrectFilter(LPUNKNOWN pUnk, HRESULT *pHr);
	virtual ~CARCorrectFilter();

	//virtual CBasePin *GetPin(int n);
	virtual HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *ppropInputRequest);
	virtual STDMETHODIMP JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName);
	virtual HRESULT CheckInputType(const CMediaType *mtIn);
	virtual HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
	virtual HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	virtual HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);
	//virtual HRESULT CompleteConnect(PIN_DIRECTION dir,IPin *pReceivePin);
	virtual HRESULT Receive(IMediaSample *pSample);
	virtual HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);	

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	STDMETHODIMP GetPages(CAUUID *pPages);

	// IPropertyBag Methods
	STDMETHODIMP Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog);
	STDMETHODIMP Write(LPCOLESTR pszPropName, VARIANT *pVar);

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr);	

protected:
	CMediaType m_InputType;
	CMediaType m_OutputType;
	LONG m_BufferSize;
	
	TrackExtendedInfoVideo m_VideoInfo;

	LONG m_AutoInsertFilterGraph;
	LONG m_ARCorrectionResizeMode;
};

#endif // _ARCORRECT_FILTER_H_
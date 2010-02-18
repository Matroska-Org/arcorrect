
#include "arcorrectfilter.h"

static const char idAutoInsertFilterGraph[] = "AutoInsertFilterGraph";
static const char idARCorrectionResizeMode[] = "ARCorrectionResizeMode";
static const char* arARCorrectFilterProps[] =
{
	idAutoInsertFilterGraph,
	idARCorrectionResizeMode
};

#if 0
CARCorrectFilterOutputPin::CARCorrectFilterOutputPin(TCHAR *pObjectName, CTransInPlaceFilter *pFilter, HRESULT * phr, LPCWSTR pPinName)
	: CTransInPlaceOutputPin(pObjectName, pFilter, phr, pPinName)
{

}

HRESULT CARCorrectFilterOutputPin::CheckMediaType(const CMediaType *pmt )
{
  if(((CARCorrectFilter *)m_pTIPFilter)->m_pInput->IsConnected() )
      return ((CARCorrectFilter *)m_pTIPFilter)->m_pInput->GetConnected()->QueryAccept( pmt );
  else
      return S_OK;
} // CheckMediaType
#endif

CARCorrectFilter::CARCorrectFilter(LPUNKNOWN pUnk, HRESULT *pHr)
	: CTransformFilter(TEXT("ARCorrect Filter"), pUnk, CLSID_ARCorrectFilter)
	, CPersistPropertyBag(arARCorrectFilterProps, SIZEOF_ARRAY(arARCorrectFilterProps), &CLSID_ARCorrectFilter)
	, CRegistryStuff(&CLSID_ARCorrectFilter)
{
	m_BufferSize = 0;		
	memset(&m_VideoInfo, 0, sizeof(TrackExtendedInfoVideo));

	LoadFromRegistry(idAutoInsertFilterGraph, &m_AutoInsertFilterGraph, 1);
	LoadFromRegistry(idARCorrectionResizeMode, &m_ARCorrectionResizeMode, 0);
}

CARCorrectFilter::~CARCorrectFilter()
{

}

/*CBasePin * CARCorrectFilter::GetPin(int n)
{
    HRESULT hr = S_OK;

    // Create an input pin if not already done

    if (m_pInput == NULL) {

        m_pInput = new CTransInPlaceInputPin( NAME("TransInPlace input pin")
                                            , this        // Owner filter
                                            , &hr         // Result code
                                            , L"Input"    // Pin name
                                            );

        // Constructor for CTransInPlaceInputPin can't fail
        ASSERT(SUCCEEDED(hr));
    }

    // Create an output pin if not already done

    if (m_pInput!=NULL && m_pOutput == NULL) {

        m_pOutput = new CARCorrectFilterOutputPin( NAME("ARCorrectFilter output pin")
                                              , this       // Owner filter
                                              , &hr        // Result code
                                              , L"Output"  // Pin name
                                              );

        // a failed return code should delete the object

        ASSERT(SUCCEEDED(hr));
        if (m_pOutput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

    // Return the appropriate pin

    ASSERT (n>=0 && n<=1);
    if (n == 0) {
        return m_pInput;
    } else if (n==1) {
        return m_pOutput;
    } else {
        return NULL;
    }
} // GetPin
*/

HRESULT CARCorrectFilter::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *ppropInputRequest) {
	HRESULT hr = NOERROR;

	// Is the input pin connected
	if(m_pInput->IsConnected() == FALSE) {
		return E_UNEXPECTED;
	}

	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(ppropInputRequest, E_POINTER);

	ppropInputRequest->cBuffers = 1;
	ppropInputRequest->cbAlign  = 1;

	// Get input pin's allocator size and use that
	ALLOCATOR_PROPERTIES InProps;
	IMemAllocator *pInAlloc = NULL;

	hr = m_pInput->GetAllocator(&pInAlloc);
	if(SUCCEEDED(hr)) {
		hr = pInAlloc->GetProperties(&InProps);
		if(SUCCEEDED(hr)) {
			ppropInputRequest->cbBuffer = InProps.cbBuffer * 2;
			m_BufferSize = ppropInputRequest->cbBuffer;
		}
		pInAlloc->Release();
	}

	if(FAILED(hr))
		return hr;

	ASSERT(ppropInputRequest->cbBuffer);

	// Ask the allocator to reserve us some sample memory, NOTE the function
	// can succeed (that is return NOERROR) but still not have allocated the
	// memory that we requested, so we must check we got whatever we wanted

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(ppropInputRequest, &Actual);
	if(FAILED(hr)) {
		return hr;
	}

	ASSERT(Actual.cBuffers == 1);

	if(ppropInputRequest->cBuffers > Actual.cBuffers ||
		ppropInputRequest->cbBuffer > Actual.cbBuffer)
	{
		return E_FAIL;
	}

	return NOERROR;

};

STDMETHODIMP CARCorrectFilter::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
	NOTE1("CARCorrectFilter::JoinFilterGraph(pName = %ws)", pName);
	HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
	IEnumFilters *pFilters = NULL;
	IBaseFilter *pFilter = NULL;
	FILTER_INFO filterInfo = { 0 };

	if (m_pGraph == NULL)
		return hr;

	hr = m_pGraph->EnumFilters(&pFilters);
	if (FAILED(hr)) {
		return hr;
	}

	ASSERT(pFilters != NULL);
	// Reset it just to be safe
	hr = pFilters->Reset();
	while (hr == S_OK) {
		hr = pFilters->Next(1, &pFilter, NULL);
		if (hr != S_OK) {
			hr = S_OK;
			break;		
		}
		
		ASSERT(pFilter != NULL);
		hr = pFilter->QueryFilterInfo(&filterInfo);
		if (FAILED(hr)) {
			break;		
		}
		
		_wcslwr(filterInfo.achName);

		if ((wcsstr(filterInfo.achName, L"video render") != NULL)
			|| (wcsstr(filterInfo.achName, L"video mixing render") != NULL)
			|| (wcsstr(filterInfo.achName, L"full screen render") != NULL))
		{
			IPin *pVideoIn = NULL;
			IPin *pVideoOut = NULL;
			PIN_INFO pinInfo = { 0 };

			hr = FindInputPin(pFilter, &pVideoIn);
			if (FAILED(hr))
				return hr;
			
			ASSERT(pVideoIn != NULL);
			hr = pVideoIn->ConnectedTo(&pVideoOut);
			if (FAILED(hr))
				return hr;				
			
			ASSERT(pVideoOut != NULL);
			hr = pVideoOut->QueryPinInfo(&pinInfo);
			if (FAILED(hr))
				return hr;

			ASSERT(pinInfo.pFilter != NULL);
			hr = pinInfo.pFilter->QueryFilterInfo(&filterInfo);
			if (FAILED(hr))
				return hr;

			if (!!lstrcmpW(filterInfo.achName, L"ARCorrect Filter")) {
				AM_MEDIA_TYPE videoFormat;
				hr = pVideoIn->ConnectionMediaType(&videoFormat);
				if (FAILED(hr))
					return hr;

				hr = pVideoIn->Disconnect();
				if (FAILED(hr))
					return hr;
				hr = pVideoOut->Disconnect();
				if (FAILED(hr))
					return hr;

				NOTE("Connecting decoder to ARCorrector...");
				hr = m_pGraph->ConnectDirect(pVideoOut, GetPin(0), &videoFormat);
				if (FAILED(hr))
					return hr;
				NOTE("Connecting ARCorrector to Video Renderer...");
				hr = m_pGraph->ConnectDirect(GetPin(1), pVideoIn, NULL);
				if (FAILED(hr))
					return hr;
#if 0
				IFilterGraph2 *pGraph2;				
				hr = m_pGraph->QueryInterface(IID_IFilterGraph2, (void **)&pGraph2);
				
				hr = pGraph2->ReconnectEx(GetPin(1), &m_InputType);
#endif
				FreeMediaType(videoFormat);
				
				hr = pFilters->Reset();
			}

		} else if ((wcsstr(filterInfo.achName, L"matroska") != NULL)
						|| (wcsstr(filterInfo.achName, L"mkv") != NULL)
						|| (wcsstr(filterInfo.achName, L"mks") != NULL))
		{
			BOOL bRet = FALSE;
			ITrackInfo *pTrackInfo;
			hr = pFilter->QueryInterface(__uuidof(ITrackInfo), (void **)&pTrackInfo);
			if (FAILED(hr)) {
				hr = S_OK;
				continue;
			}

			ASSERT(pTrackInfo != NULL);
			for (UINT t = 0; t < pTrackInfo->GetTrackCount(); t++) {				
				TrackElement trackEntry;
				TrackExtendedInfoVideo trackVideoEntry;
				
				trackEntry.Size = sizeof(TrackElement);
				trackVideoEntry.Size = sizeof(TrackExtendedInfoVideo);

				bRet = pTrackInfo->GetTrackInfo(t, &trackEntry);
				ASSERT(bRet);

				if (trackEntry.Type == TypeVideo) {
					bRet = pTrackInfo->GetTrackExtendedInfo(t, (void *)&trackVideoEntry);
					ASSERT(bRet);
					m_VideoInfo = trackVideoEntry;
					break;
				}
			}
		}

	}

	return hr;
}

HRESULT CARCorrectFilter::CheckInputType(const CMediaType *mtIn) {
	NOTE("CARCorrectFilter::CheckInputType()");
	
	if ((*mtIn->Type() == MEDIATYPE_Video) && IsUncompressedVideo(*mtIn->Subtype())) {
		DisplayType(TEXT("CARCorrectFilter::CheckInputType - mtIn"), mtIn);

		if (*mtIn->FormatType() == FORMAT_VideoInfo) {			
			m_InputType = *mtIn;

			VIDEOINFOHEADER *pvhr = (VIDEOINFOHEADER *)m_InputType.pbFormat;
			VIDEOINFOHEADER2 vhr2 = { 0 };
			vhr2.AvgTimePerFrame = pvhr->AvgTimePerFrame;
			vhr2.bmiHeader = pvhr->bmiHeader;
			vhr2.dwBitErrorRate = pvhr->dwBitErrorRate;
			vhr2.dwBitRate = pvhr->dwBitRate;
			vhr2.rcSource = pvhr->rcSource;
			vhr2.rcTarget = pvhr->rcTarget;
			vhr2.dwPictAspectRatioX = m_VideoInfo.DisplayWidth;
			vhr2.dwPictAspectRatioY = m_VideoInfo.DisplayHeight;
			
			m_OutputType = m_InputType;
			m_OutputType.SetFormatType(&FORMAT_VideoInfo2);
			m_OutputType.SetFormat((BYTE *)&vhr2, sizeof(VIDEOINFOHEADER2));

			return S_OK;
		}
	}
	return S_FALSE;	
};

HRESULT CARCorrectFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut) {
	NOTE("CARCorrectFilter::CheckTransform()");
	HRESULT hr = CheckInputType(mtIn);

	if(hr == S_FALSE) {
		return hr;
	}

	return NOERROR;
};

HRESULT CARCorrectFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	ASSERT(iPosition == 0 || iPosition == 1);
	
	if (iPosition == 0) {
		CheckPointer(pMediaType, E_POINTER);	

		*pMediaType = m_InputType;

		return S_OK;
	}

	return VFW_S_NO_MORE_ITEMS;
}

HRESULT CARCorrectFilter::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt) {
	NOTE("CARCorrectFilter::SetMediaType()");
	HRESULT hr = CTransformFilter::SetMediaType(direction, pmt);

	if (direction == PINDIR_INPUT) {

	} else if (direction == PINDIR_OUTPUT) {
		
	}

	return hr;
}

HRESULT CARCorrectFilter::Receive(IMediaSample *pSample)
{	
	return CTransformFilter::Receive(pSample);

	pSample->AddRef();
	return ((CARCorrectFilterOutputPin *)m_pOutput)->PassSample(pSample);
}

HRESULT CARCorrectFilter::Transform(IMediaSample *pIn, IMediaSample *pOut) {
	HRESULT hr = DShowIMediaSampleCopy(pIn, pOut, true, m_BufferSize);
	
	REFERENCE_TIME TimeStart, TimeEnd;
	LONGLONG MediaStart, MediaEnd;

	pIn->GetTime(&TimeStart, &TimeEnd);	
	pIn->GetMediaTime(&MediaStart,&MediaEnd);
	NOTE4("CARCorrectFilter::Transform(pIn, length = %i, size = %i, TimeStart = %i, TimeEnd = %i", pIn->GetActualDataLength(), pIn->GetSize(), (long)TimeStart / 1000, (long)TimeEnd / 1000);
	//hr = pOut->SetMediaType(&m_OutputType);

	return hr;
};

STDMETHODIMP CARCorrectFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	NOTE("CARCorrectFilter::NonDelegatingQueryInterface()");

	if (riid == IID_ISpecifyPropertyPages) {
		return GetInterface((ISpecifyPropertyPages *) this, ppv);			
	
	} else if (riid == IID_IPropertyBag) {
    CheckPointer(ppv, E_POINTER);
		*ppv = (IPropertyBag*)this;
		return GetInterface((IPropertyBag *) this, ppv);

	} else {
		return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}

STDMETHODIMP CARCorrectFilter::GetPages(CAUUID *pPages)
{
	NOTE("CARCorrectFilter::GetPages()");
	
	if (!pPages) 
		return E_POINTER;

	pPages->cElems = 1;
	pPages->pElems = reinterpret_cast<GUID*>(CoTaskMemAlloc(sizeof(GUID)));
	if (pPages->pElems == NULL) 
	{
		return E_OUTOFMEMORY;
	}
	*(pPages->pElems) = CLSID_ARCorrectFilter_Page;
	return NOERROR;
}

STDMETHODIMP CARCorrectFilter::Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog)
{
	char szName[256];

	wcstombs(szName, pszPropName, sizeof(szName));
	VariantClear(pVar);

	if (!strcmp(szName, idAutoInsertFilterGraph)) {
		pVar->vt = VT_I4;
		pVar->lVal = 0;//m_PassMode;
		return NOERROR;

	}	else if (!strcmp(szName, idARCorrectionResizeMode)) {
		pVar->vt = VT_I4;
		pVar->lVal = 0;//GetFactor();
		return NOERROR;
	}

	return E_FAIL;
}

STDMETHODIMP CARCorrectFilter::Write(LPCOLESTR pszPropName, VARIANT *pVar)
{
	char szName[256];

	wcstombs(szName, pszPropName, sizeof(szName));

	if (!strcmp(szName, idAutoInsertFilterGraph)) {
		VariantChangeType(pVar, pVar, 0, VT_I4);
		m_AutoInsertFilterGraph = (pVar->lVal);
		SaveToRegistry(idAutoInsertFilterGraph, m_AutoInsertFilterGraph);
		return NOERROR;

	}	else if (!strcmp(szName, idARCorrectionResizeMode)) {
		VariantChangeType(pVar, pVar, 0, VT_I4);
		m_ARCorrectionResizeMode = (pVar->lVal);
		SaveToRegistry(idARCorrectionResizeMode, m_ARCorrectionResizeMode);
		return NOERROR;
	}

	return E_FAIL;
}

CUnknown *WINAPI CARCorrectFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
	NOTE("CARCorrectFilter::CreateInstance()");
	return new CARCorrectFilter(pUnk, phr);
}

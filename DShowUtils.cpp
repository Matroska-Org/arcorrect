
#include "DShowUtils.h"

#include <initguid.h>
// 30323449-0000-0010-8000-00AA00389B71  'I420' == MEDIASUBTYPE_I420
static const GUID MEDIASUBTYPE_I420 =
{ 0x30323449, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

/// Make destination an identical copy of source
HRESULT DShowIMediaSampleCopy(IMediaSample *pSource, IMediaSample *pDest, bool bCopyData, LONG bufferSize)
{
	CheckPointer(pSource, E_POINTER);
	CheckPointer(pDest, E_POINTER);

	if (bCopyData) {
		// Copy the sample data
		BYTE *pSourceBuffer, *pDestBuffer;
		long lSourceSize = pSource->GetActualDataLength();

	#ifdef DEBUG  
		if (bufferSize == 0) {
			long lDestSize = pDest->GetSize();
			ASSERT(lDestSize >= lSourceSize);
		} else {
			ASSERT(bufferSize >= lSourceSize);
		}
	#endif

		pSource->GetPointer(&pSourceBuffer);
		pDest->GetPointer(&pDestBuffer);

		CopyMemory((PVOID) pDestBuffer,(PVOID) pSourceBuffer, lSourceSize);
	}

	// Copy the sample times
	REFERENCE_TIME TimeStart, TimeEnd;
	if(NOERROR == pSource->GetTime(&TimeStart, &TimeEnd))
	{
		pDest->SetTime(&TimeStart, &TimeEnd);
	}

	LONGLONG MediaStart, MediaEnd;
	if(pSource->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR)
	{
		pDest->SetMediaTime(&MediaStart,&MediaEnd);
	}

	// Copy the media type
	AM_MEDIA_TYPE *pMediaType;
	pSource->GetMediaType(&pMediaType);
	pDest->SetMediaType(pMediaType);
	DeleteMediaType(pMediaType);

	// Copy the actual data length
	long lDataLength = pSource->GetActualDataLength();
	pDest->SetActualDataLength(lDataLength);

	if (pSource->IsDiscontinuity() ==  S_OK)
		pDest->SetDiscontinuity(TRUE);
	else
		pDest->SetDiscontinuity(FALSE);

	if (pSource->IsPreroll() ==  S_OK)
		pDest->SetPreroll(TRUE);
	else
		pDest->SetPreroll(FALSE);

	if (pSource->IsSyncPoint() ==  S_OK)
		pDest->SetSyncPoint(TRUE);
	else
		pDest->SetSyncPoint(FALSE);

	return NOERROR;
}

HRESULT FindOutputPin(IBaseFilter *pFilter, IPin **ppPin)
{
	if (!pFilter || ! ppPin)
		return E_POINTER;

	*ppPin = 0;
	HRESULT hr;
	//Find the output pin of the Source Filter
	IEnumPins *pPinEnum;
	hr = pFilter->EnumPins(&pPinEnum);
	if (FAILED(hr))
	  return E_FAIL;
	IPin *pSearchPin;
	while (pPinEnum->Next(1, &pSearchPin, NULL) == S_OK)
	{
		PIN_DIRECTION pPinDir;
		hr = pSearchPin->QueryDirection(&pPinDir);
		if (FAILED(hr))
		  return E_FAIL;
		if (pPinDir == PINDIR_OUTPUT)
		{
			//Found out pin
			*ppPin = pSearchPin;
			break;
		}
	}
	pPinEnum->Release();
	return hr;
}

HRESULT FindInputPin(IBaseFilter *pFilter, IPin **ppPin)
{
	if (!pFilter || ! ppPin)
		return E_POINTER;

	*ppPin = 0;
	HRESULT hr;
	//Find the output pin of the Source Filter
	IEnumPins *pPinEnum;
	hr = pFilter->EnumPins(&pPinEnum);
	if (FAILED(hr))
	  return E_FAIL;

	IPin *pSearchPin;
	while (pPinEnum->Next(1, &pSearchPin, NULL) == S_OK)
	{
		PIN_DIRECTION pPinDir;
		hr = pSearchPin->QueryDirection(&pPinDir);
		if (FAILED(hr))
	    return E_FAIL;
		if (pPinDir == PINDIR_INPUT)
		{
			//Found out pin
			*ppPin = pSearchPin;
			break;
		}
	}
	pPinEnum->Release();
	return hr;
}

BOOL IsUncompressedVideo(const GUID &subtype)
{
	UINT i = 0;
	static const GUID *uncompressed_subtypes[] = {
		&MEDIASUBTYPE_I420,
		&MEDIASUBTYPE_YV12,
		&MEDIASUBTYPE_YVU9,
		&MEDIASUBTYPE_IF09,
		&MEDIASUBTYPE_IYUV,
		&MEDIASUBTYPE_AYUV,
		&MEDIASUBTYPE_UYVY,
		&MEDIASUBTYPE_Y411,
		&MEDIASUBTYPE_Y41P,
		&MEDIASUBTYPE_Y211,
		&MEDIASUBTYPE_YUY2,
		&MEDIASUBTYPE_YVYU,
		&MEDIASUBTYPE_YUYV,
		&MEDIASUBTYPE_RGB1,
		&MEDIASUBTYPE_RGB4,
		&MEDIASUBTYPE_RGB8,
		&MEDIASUBTYPE_RGB565,
		&MEDIASUBTYPE_RGB555,
		&MEDIASUBTYPE_RGB24,
		&MEDIASUBTYPE_RGB32,
		&MEDIASUBTYPE_ARGB32,
		NULL
	};

	while (uncompressed_subtypes[i] != NULL) {
		const GUID *entry = uncompressed_subtypes[i];
		if (subtype == *entry)
			return TRUE;
		i++;
	}

	return FALSE;
}

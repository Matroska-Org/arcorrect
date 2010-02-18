
#ifndef _DSHOW_UTILS_H_
#define _DSHOW_UTILS_H_

#include <streams.h>

HRESULT DShowIMediaSampleCopy(IMediaSample *pSource, IMediaSample *pDest, bool bCopyData, LONG bufferSize = 0);
HRESULT FindOutputPin(IBaseFilter *pFilter, IPin **ppPin);
HRESULT FindInputPin(IBaseFilter *pFilter, IPin **ppPin);
BOOL IsUncompressedVideo(const GUID &subtype);

#endif // _DSHOW_UTILS_H_

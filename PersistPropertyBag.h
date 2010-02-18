/*******************************************************************************
*                                                                              *
* This file is part of the Ogg Vorbis DirectShow filter collection             *
*                                                                              *
* Copyright (c) 2001, Tobias Waldvogel                                         *
* All rights reserved.                                                         *
*                                                                              *
* Redistribution and use in source and binary forms, with or without           *
* modification, are permitted provided that the following conditions are met:  *
*                                                                              *
*  - Redistributions of source code must retain the above copyright notice,    *
*    this list of conditions and the following disclaimer.                     *
*                                                                              *
*  - Redistributions in binary form must reproduce the above copyright notice, *
*    this list of conditions and the following disclaimer in the documentation *
*    and/or other materials provided with the distribution.                    *
*                                                                              *
*  - The names of the contributors may not be used to endorse or promote       *
*    products derived from this software without specific prior written        *
*    permission.                                                               *
*                                                                              *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"  *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE    *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   *
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE     *
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR          *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF         *
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS     *
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN      *
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)      *
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   *
* POSSIBILITY OF SUCH DAMAGE.                                                  *
*                                                                              *
*******************************************************************************/

#ifndef __CPERSIST_PROPERTY_BAG__
#define __CPERSIST_PROPERTY_BAG__

#include <streams.h>
#define SIZEOF_ARRAY(ar)            (sizeof(ar)/sizeof((ar)[0]))

class CRegistryStuff
{
private:
	const GUID*		m_pClsID;
public:
	CRegistryStuff(const GUID* pClsID);
	HKEY	OpenRegistry();
	HRESULT	SaveToRegistry(const char* szName, bool bVal);
	HRESULT	SaveToRegistry(const char* szName, long iVal);
	HRESULT	SaveToRegistry(const char* szName, unsigned long iVal);
	HRESULT	SaveToRegistry(const char* szName, float fVal);
	HRESULT	SaveToRegistry(const char* szName, char* szVal);
	HRESULT	SaveToRegistry(const char* szName, wchar_t* wzVal);
	HRESULT	LoadFromRegistry(const char* szName, bool* pbVal, bool bDefault);
	HRESULT	LoadFromRegistry(const char* szName, long* piVal, long iDefault);
	HRESULT	LoadFromRegistry(const char* szName, unsigned long* piVal, unsigned long iDefault);
	HRESULT	LoadFromRegistry(const char* szName, float* pfVal, float fDefault);
};


class CPersistPropertyBag	: public IPersistPropertyBag
{
private:
	int				m_iProps;
	const char**	m_arProps;
	const GUID*		m_pClsID;
public:
	CPersistPropertyBag(const char** arProps, int iProps, const GUID* pClsID);

	HRESULT STDMETHODCALLTYPE			InitNew(void);
	HRESULT	STDMETHODCALLTYPE			GetClassID(CLSID *pClsID);
	HRESULT	STDMETHODCALLTYPE			Load(IPropertyBag *pPropBag, IErrorLog *pErrorLog);
	HRESULT STDMETHODCALLTYPE			Save(IPropertyBag *pPropBag, BOOL fClearDirty,
										 BOOL fSaveAllProperties);

	virtual HRESULT STDMETHODCALLTYPE Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog) = 0;
	virtual HRESULT STDMETHODCALLTYPE Write(LPCOLESTR pszPropName, VARIANT *pVar) = 0;
};

#endif
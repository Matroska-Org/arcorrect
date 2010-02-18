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

#include "PersistPropertyBag.h"
//#include "common.h"
#include <stdio.h>

CPersistPropertyBag::CPersistPropertyBag(const char** arProps, int iProps, const GUID* pClsID)
{
	m_iProps  = iProps;
	m_arProps = arProps;
	m_pClsID = pClsID;
};

HRESULT STDMETHODCALLTYPE CPersistPropertyBag::GetClassID(CLSID* pClsID)
{
	*pClsID = * m_pClsID;
	return NOERROR;
}

HRESULT STDMETHODCALLTYPE CPersistPropertyBag::InitNew(void)
{
	return NOERROR;
}

// Walks through all properties in m_arProps and calls read
HRESULT STDMETHODCALLTYPE CPersistPropertyBag::Load(IPropertyBag *pPropBag, IErrorLog *pErrorLog)
{
	VARIANT	V;
	wchar_t	wzName[128];

	VariantInit(&V);

	for (int i=0; i<m_iProps; i++)
	{
		mbstowcs(wzName, m_arProps[i], 128);
		if (SUCCEEDED(pPropBag->Read(wzName, &V, NULL)))
			Write(wzName, &V);
	}

	VariantClear(&V);
	return NOERROR;
}

// Walks through all properties in m_arProps and calls write
HRESULT STDMETHODCALLTYPE CPersistPropertyBag::Save(IPropertyBag *pPropBag, BOOL fClearDirty,
												BOOL fSaveAllProperties)
{
	VARIANT	V;
	wchar_t	wzName[128];

	VariantInit(&V);

	for (int i=0; i<m_iProps; i++)
	{
		mbstowcs(wzName, m_arProps[i], 128);
		if (SUCCEEDED(Read(wzName, &V, NULL)))
			pPropBag->Write(wzName, &V);
	}

	VariantClear(&V);
	return NOERROR;
}

CRegistryStuff::CRegistryStuff(const GUID* pClsID)
{
	m_pClsID = pClsID;
};

// Returns a handle to the registry key HKEY_CLASSES_ROOT, CLSID\<idCLSID>\defaults
HKEY CRegistryStuff::OpenRegistry()
{
	OLECHAR	szCLSID[CHARS_IN_GUID];
	CHAR	chCLSIDEntry[260];
	LPDWORD lpdwDisposition = 0;
	HKEY	hKey;
	LONG	result;

	StringFromGUID2(*m_pClsID, szCLSID, CHARS_IN_GUID);
	wsprintf(chCLSIDEntry, "CLSID\\%ls\\Defaults", szCLSID);
	result = RegCreateKeyEx(HKEY_CLASSES_ROOT, chCLSIDEntry, 0, "",
					REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL,
					&hKey, lpdwDisposition);

	return result == ERROR_SUCCESS ? hKey : NULL;
}

HRESULT	CRegistryStuff::SaveToRegistry(const char* szName, bool bVal)
{
	if (HKEY hKey = OpenRegistry())
	{
		DWORD	dwData = bVal ? 1 : 0;
		RegSetValueEx(hKey, szName, 0, REG_DWORD, (CONST BYTE*)&dwData, sizeof(dwData));
		RegCloseKey(hKey);
		return NOERROR;
	}
	return E_FAIL;
}

HRESULT	CRegistryStuff::SaveToRegistry(const char* szName, long iVal)
{
	return SaveToRegistry(szName, (unsigned long)iVal);
}

HRESULT	CRegistryStuff::SaveToRegistry(const char* szName, unsigned long iVal)
{
	if (HKEY hKey = OpenRegistry())
	{
		RegSetValueEx(hKey, szName, 0, REG_DWORD, (CONST BYTE*)&iVal, sizeof(iVal));
		RegCloseKey(hKey);
		return NOERROR;
	}
	return E_FAIL;
}

HRESULT	CRegistryStuff::SaveToRegistry(const char* szName, float fVal)
{
	if (HKEY hKey = OpenRegistry())
	{
		char	szVal[32];
		sprintf(szVal, "%f", fVal);
		RegSetValueEx(hKey, szName, 0, REG_SZ, (CONST BYTE*)szVal, (DWORD)strlen(szVal));
		RegCloseKey(hKey);
		return NOERROR;
	}
	return E_FAIL;
}


HRESULT	CRegistryStuff::SaveToRegistry(const char* szName, char* szVal)
{
	if (HKEY hKey = OpenRegistry())
	{
		RegSetValueEx(hKey, szName, 0, REG_SZ, (CONST BYTE*)szVal, (DWORD)strlen(szVal)+1);
		RegCloseKey(hKey);
		return NOERROR;
	}
	return E_FAIL;
}

HRESULT	CRegistryStuff::SaveToRegistry(const char* szName, wchar_t* wzVal)
{
	if (HKEY hKey = OpenRegistry())
	{
		char	szValue[128];

		wcstombs(szValue, wzVal, 128);
		RegSetValueEx(hKey, szName, 0, REG_SZ, (CONST BYTE*)szValue, (DWORD)strlen(szValue)+1);
		RegCloseKey(hKey);
		return NOERROR;
	}
	return E_FAIL;

}

HRESULT	CRegistryStuff::LoadFromRegistry(const char* szName,
											  bool* pbVal, bool bDefault)
{
	if (HKEY hKey = OpenRegistry())
	{
		DWORD	dwData;
		DWORD	dwSize = sizeof(dwData);

		if (RegQueryValueEx(hKey, szName, NULL, NULL, (BYTE*)&dwData, &dwSize) == ERROR_SUCCESS)
			*pbVal = dwData != 0;
		else	
			*pbVal = bDefault;

		RegCloseKey(hKey);
		return NOERROR;
	}
	return E_FAIL;
}

HRESULT	CRegistryStuff::LoadFromRegistry(const char* szName,
											  long* piVal, long iDefault)
{
	return LoadFromRegistry(szName, (unsigned long*) piVal,
									(unsigned long) iDefault);
}

HRESULT	CRegistryStuff::LoadFromRegistry(const char* szName,
							unsigned long* piVal, unsigned long iDefault)
{
	if (HKEY hKey = OpenRegistry())
	{
		DWORD	dwSize = sizeof(*piVal);

		if (RegQueryValueEx(hKey, szName, NULL, NULL, (BYTE*)piVal, &dwSize) != ERROR_SUCCESS)
			*piVal = iDefault;
	
		RegCloseKey(hKey);
		return NOERROR;
	}
	return E_FAIL;
}

HRESULT	CRegistryStuff::LoadFromRegistry(const char* szName,
											  float* pfVal, float fDefault)
{
	if (HKEY hKey = OpenRegistry())
	{
		char	szVal[32];
		DWORD	dwSize = sizeof(szVal);

		if (RegQueryValueEx(hKey, szName, NULL, NULL, (BYTE*)szVal, &dwSize) == ERROR_SUCCESS)
			*pfVal = (float)atof(szVal);
		else
			*pfVal = fDefault;
	
		RegCloseKey(hKey);
		return NOERROR;
	}
	return E_FAIL;
}

#include "stdafx.h"

#include "XPIConfig.h"

#include <xmllite.h>
#include <shlwapi.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <boost/scoped_array.hpp>

#include "CResourceString.hpp"
#include "XPIColors.h"
#include "XPIUtilities.hpp"
#include "extvars.hpp"

#pragma  comment(lib, "xmllite")
#pragma  comment(lib, "shlwapi")

BOOL ParseOpcode(__in IXmlReader* pReader)
{
	if (pReader == NULL || pOpcodeInfo == NULL)
		return FALSE;

	XmlNodeType NodeType;
	LPCWSTR     lpcwszElement, lpcwszValue;
	LPWSTR      lpwszEndPtr;
	DWORD       dwOpcode;

	if (FAILED(pReader->MoveToFirstAttribute()))
		return FALSE;
	if (FAILED(pReader->GetValue(&lpcwszValue, NULL)))
		return FALSE;

	dwOpcode = wcstoul(lpcwszValue, &lpwszEndPtr, 10);

	while (pReader->Read(&NodeType) == S_OK)
	{
		if (NodeType == XmlNodeType_Element)
		{
			if (FAILED(pReader->GetLocalName(&lpcwszElement, NULL)))
				return FALSE;
		}
		else if (NodeType == XmlNodeType_Text)
		{
			if (FAILED(pReader->GetValue(&lpcwszValue, NULL)))
				return FALSE;

			if (lstrcmpiW(lpcwszElement, L"ignore") == 0)
			{
				if (lstrcmpiW(lpcwszValue, L"true") == 0)
					(*pOpcodeInfo)[dwOpcode].bIgnore = TRUE;
				else
					(*pOpcodeInfo)[dwOpcode].bIgnore = FALSE;
			}
			else if (lstrcmpiW(lpcwszElement, L"block") == 0)
			{
				if (lstrcmpiW(lpcwszValue, L"true") == 0)
					(*pOpcodeInfo)[dwOpcode].bBlock = TRUE;
				else
					(*pOpcodeInfo)[dwOpcode].bBlock = FALSE;
			}
			else if (lstrcmpiW(lpcwszElement, L"alias") == 0)
			{
				StringCchCopyW((*pOpcodeInfo)[dwOpcode].wszAlias, _countof((*pOpcodeInfo)[dwOpcode].wszAlias), lpcwszValue);
			}
			else if (lstrcmpiW(lpcwszElement, L"comment") == 0)
			{
				StringCchCopyW((*pOpcodeInfo)[dwOpcode].wszComment, _countof((*pOpcodeInfo)[dwOpcode].wszComment), lpcwszValue);
			}
			else if (lstrcmpiW(lpcwszElement, L"color") == 0)
			{
				for (INT i = 0; i < COUNT_XPI_COLOR; i++)
				{
					if (lstrcmpiW(pStrings->Get(g_XPIColors[i].wName).c_str(), lpcwszValue) == 0)
					{
						(*pOpcodeInfo)[dwOpcode].crColor = g_XPIColors[i].crColor;
						break;
					}
				}
			}
		}
		else if (NodeType == XmlNodeType_EndElement)
		{
			pReader->GetLocalName(&lpcwszValue, NULL);
			if (lstrcmpiW(lpcwszValue, L"opcode") == 0)
				return TRUE;
		}
	}

	return TRUE;
}

BOOL LoadXPIConfig(__in_z LPCWSTR lpcwszFile)
{
	if (lpcwszFile == NULL || pOpcodeInfo == NULL)
		return FALSE;

	IStream* pFileStream;

	if (FAILED(SHCreateStreamOnFileEx(lpcwszFile, STGM_READ | STGM_FAILIFTHERE, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &pFileStream)))
		return FALSE;

	IXmlReader* pReader;
	XmlNodeType NodeType;
	LPCWSTR     lpcwszValue;

	if (FAILED(CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL)))
	{
		pFileStream->Release();
		return FALSE;
	}

	pReader->SetInput(pFileStream);

	while (pReader->Read(&NodeType) == S_OK)
	{
		switch (NodeType)
		{
		case XmlNodeType_Element:
			pReader->GetLocalName(&lpcwszValue, NULL);
			if (lstrcmpiW(lpcwszValue, L"opcode") == 0)
			{
				if (!ParseOpcode(pReader))
				{
					pReader->Release();
					pFileStream->Release();
					return FALSE;
				}
			}
			else if (lstrcmpiW(lpcwszValue, L"autoscroll") == 0)
			{
				while (NodeType != XmlNodeType_Text)
				{
					if (FAILED(pReader->Read(&NodeType)))
					{
						pReader->Release();
						pFileStream->Release();
						return FALSE;
					}
				}

				if (FAILED(pReader->GetValue(&lpcwszValue, NULL)))
				{
					pReader->Release();
					pFileStream->Release();
					return FALSE;
				}

				if (lstrcmpiW(lpcwszValue, L"true") == 0)
					bAutoscroll = TRUE;
				else
					bAutoscroll = FALSE;
			}
			break;
		case XmlNodeType_EndElement:
			pReader->GetLocalName(&lpcwszValue, NULL);
			break;
		}
	}

	pReader->Release();
	pFileStream->Release();
	return TRUE;
}

VOID WriteOpcodeEntry(__in IXmlWriter* pWriter, __in DWORD dwOpcode, __in BOOL bIgnore, __in BOOL bBlock, __in_z_opt LPCWSTR lpcwszAlias, __in_z_opt LPCWSTR lpcwszComment, __in_opt COLORREF crColor = 0)
{
	if (pWriter == NULL)
		return;

	WCHAR wszTemp[16];

	pWriter->WriteStartElement(NULL, L"opcode", NULL);
	StringCchPrintfW(wszTemp, _countof(wszTemp), L"%d", dwOpcode);
	pWriter->WriteAttributeString(NULL, L"value", NULL, wszTemp);
	pWriter->WriteStartElement(NULL, L"ignore", NULL);
	pWriter->WriteString(bIgnore ? L"true" : L"false");
	pWriter->WriteEndElement();
	pWriter->WriteStartElement(NULL, L"block", NULL);
	pWriter->WriteString(bBlock ? L"true" : L"false");
	pWriter->WriteEndElement();
	if (lpcwszAlias != NULL)
	{
		if (lpcwszAlias[0] != 0)
		{
			pWriter->WriteStartElement(NULL, L"alias", NULL);
			pWriter->WriteString(lpcwszAlias);
			pWriter->WriteEndElement();
		}
	}
	if (lpcwszComment != NULL)
	{
		if (lpcwszComment[0] != 0)
		{
			pWriter->WriteStartElement(NULL, L"comment", NULL);
			pWriter->WriteString(lpcwszComment);
			pWriter->WriteEndElement();
		}
	}
	if (crColor != 0)
	{
		pWriter->WriteStartElement(NULL, L"color", NULL);
		for (INT i = 0; i < COUNT_XPI_COLOR; i++)
		{
			if (g_XPIColors[i].crColor == crColor)
			{
				size_t cch;
				StringCchLength(pStrings->Get(g_XPIColors[i].wName).c_str(), 1024, &cch);
				boost::scoped_array<WCHAR> lpwszColor(new WCHAR[cch + 1]);
				StringCchCopyW(lpwszColor.get(), cch + 1, pStrings->Get(g_XPIColors[i].wName).c_str());
				CharLowerBuffW(lpwszColor.get(), cch + 1);
				pWriter->WriteString(lpwszColor.get());
				break;
			}
		}
		pWriter->WriteEndElement();
	}
	pWriter->WriteEndElement();
}

BOOL SaveXPIConfig(__in_z LPCWSTR lpcwszFile)
{
	if (lpcwszFile == NULL || pOpcodeInfo == NULL)
		return FALSE;

	if (pOpcodeInfo->empty())
		return TRUE;

	IStream* pFileStream;

	if (FAILED(SHCreateStreamOnFileEx(lpcwszFile, STGM_WRITE | STGM_CREATE, FILE_ATTRIBUTE_NORMAL, TRUE, NULL, &pFileStream)))
		return FALSE;

	IXmlWriter* pWriter;

	if (FAILED(CreateXmlWriter(__uuidof(IXmlWriter), (void**)&pWriter, NULL)))
	{
		pFileStream->Release();
		return FALSE;
	}

	pWriter->SetOutput(pFileStream);

	pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
	pWriter->WriteStartDocument(XmlStandalone_Omit);

	pWriter->WriteStartElement(NULL, L"xpi_config", NULL);

	pWriter->WriteStartElement(NULL, L"autoscroll", NULL);
	pWriter->WriteString(bAutoscroll ? L"true" : L"false");
	pWriter->WriteEndElement();

	foreach(OPCODE_MAP::iterator::value_type& i, *pOpcodeInfo)
		WriteOpcodeEntry(pWriter, i.first, i.second.bIgnore, i.second.bBlock, i.second.wszAlias, i.second.wszComment, i.second.crColor);

	pWriter->WriteEndElement();

	pWriter->WriteEndDocument();

	BOOL bRet = TRUE;

	if (FAILED(pWriter->Flush()))
		bRet = FALSE;

	pFileStream->Release();
	pWriter->Release();
	return bRet;
}

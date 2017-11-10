#include "stdafx.h"
#include "misc.h"

//--------------------------------------------------------------------------------------
// Implementation of CGrowableArray
//--------------------------------------------------------------------------------------

// This version doesn't call ctor or dtor.
//template<typename TYPE> HRESULT CGrowableArray <TYPE>::SetSizeInternal(int nNewMaxSize)



//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray <TYPE>::Insert(int nIndex, const TYPE& value)
{
	HRESULT hr;

	// Validate index
	if (nIndex < 0 ||
		nIndex > m_nSize)
	{
		assert(false);
		return E_INVALIDARG;
	}

	// Prepare the buffer
	if (FAILED(hr = SetSizeInternal(m_nSize + 1)))
		return hr;

	// Shift the array
	MoveMemory(&m_pData[nIndex + 1], &m_pData[nIndex], sizeof(TYPE) * (m_nSize - nIndex));

	// Construct the new element
	::new (&m_pData[nIndex]) TYPE;

	// Set the value and increase the size
	m_pData[nIndex] = value;
	++m_nSize;

	return S_OK;
}


//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray <TYPE>::SetAt(int nIndex, const TYPE& value)
{
	// Validate arguments
	if (nIndex < 0 ||
		nIndex >= m_nSize)
	{
		assert(false);
		return E_INVALIDARG;
	}

	m_pData[nIndex] = value;
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Searches for the specified value and returns the index of the first occurrence
// within the section of the samples array that extends from iStart and contains the 
// specified number of elements. Returns -1 if value is not found within the given 
// section.
//--------------------------------------------------------------------------------------
template<typename TYPE> int CGrowableArray <TYPE>::IndexOf(const TYPE& value, int iStart, int nNumElements)
{
	// Validate arguments
	if (iStart < 0 ||
		iStart >= m_nSize ||
		nNumElements < 0 ||
		iStart + nNumElements > m_nSize)
	{
		assert(false);
		return -1;
	}

	// Search
	for (int i = iStart; i < (iStart + nNumElements); i++)
	{
		if (value == m_pData[i])
			return i;
	}

	// Not found
	return -1;
}


//--------------------------------------------------------------------------------------
// Searches for the specified value and returns the index of the last occurrence
// within the section of the samples array that contains the specified number of elements
// and ends at iEnd. Returns -1 if value is not found within the given section.
//--------------------------------------------------------------------------------------
template<typename TYPE> int CGrowableArray <TYPE>::LastIndexOf(const TYPE& value, int iEnd, int nNumElements)
{
	// Validate arguments
	if (iEnd < 0 ||
		iEnd >= m_nSize ||
		nNumElements < 0 ||
		iEnd - nNumElements < 0)
	{
		assert(false);
		return -1;
	}

	// Search
	for (int i = iEnd; i >(iEnd - nNumElements); i--)
	{
		if (value == m_pData[i])
			return i;
	}

	// Not found
	return -1;
}

//
////--------------------------------------------------------------------------------------
//CUniBuffer::CUniBuffer(int nInitialSize)
//{
//	CUniBuffer::Initialize();  // ensure static vars are properly init'ed first
//
//	m_nBufferSize = 0;
//	m_pwszBuffer = NULL;
//	m_bAnalyseRequired = true;
//	m_Analysis = NULL;
//	//m_pFontNode = NULL;
//
//	if (nInitialSize > 0)
//		SetBufferSize(nInitialSize);
//}
//
//
////--------------------------------------------------------------------------------------
//CUniBuffer::~CUniBuffer()
//{
//	delete[] m_pwszBuffer;
//	if (m_Analysis)
//		_ScriptStringFree(&m_Analysis);
//}
//
////--------------------------------------------------------------------------------------
//void CUniBuffer::Initialize()
//{
//	if (s_hDll) // Only need to do once
//		return;
//
//	s_hDll = LoadLibrary(UNISCRIBE_DLLNAME);
//	if (s_hDll)
//	{
//		FARPROC Temp;
//		GETPROCADDRESS(s_hDll, ScriptApplyDigitSubstitution, Temp);
//		GETPROCADDRESS(s_hDll, ScriptStringAnalyse, Temp);
//		GETPROCADDRESS(s_hDll, ScriptStringCPtoX, Temp);
//		GETPROCADDRESS(s_hDll, ScriptStringXtoCP, Temp);
//		GETPROCADDRESS(s_hDll, ScriptStringFree, Temp);
//		GETPROCADDRESS(s_hDll, ScriptString_pLogAttr, Temp);
//		GETPROCADDRESS(s_hDll, ScriptString_pcOutChars, Temp);
//	}
//}
//
//
////--------------------------------------------------------------------------------------
//void CUniBuffer::Uninitialize()
//{
//	if (s_hDll)
//	{
//		PLACEHOLDERPROC(ScriptApplyDigitSubstitution);
//		PLACEHOLDERPROC(ScriptStringAnalyse);
//		PLACEHOLDERPROC(ScriptStringCPtoX);
//		PLACEHOLDERPROC(ScriptStringXtoCP);
//		PLACEHOLDERPROC(ScriptStringFree);
//		PLACEHOLDERPROC(ScriptString_pLogAttr);
//		PLACEHOLDERPROC(ScriptString_pcOutChars);
//
//		FreeLibrary(s_hDll);
//		s_hDll = NULL;
//	}
//}
//
////--------------------------------------------------------------------------------------
//bool CUniBuffer::SetBufferSize(int nNewSize)
//{
//	// If the current size is already the maximum allowed,
//	// we can't possibly allocate more.
//	if (m_nBufferSize == GUI_MAX_EDITBOXLENGTH)
//		return false;
//
//	int nAllocateSize = (nNewSize == -1 || nNewSize < m_nBufferSize * 2) ? (m_nBufferSize ? m_nBufferSize *
//		2 : 256) : nNewSize * 2;
//
//	// Cap the buffer size at the maximum allowed.
//	if (nAllocateSize > GUI_MAX_EDITBOXLENGTH)
//		nAllocateSize = GUI_MAX_EDITBOXLENGTH;
//
//	WCHAR* pTempBuffer = new WCHAR[nAllocateSize];
//	if (!pTempBuffer)
//		return false;
//
//	ZeroMemory(pTempBuffer, sizeof(WCHAR) * nAllocateSize);
//
//	if (m_pwszBuffer)
//	{
//		CopyMemory(pTempBuffer, m_pwszBuffer, m_nBufferSize * sizeof(WCHAR));
//		delete[] m_pwszBuffer;
//	}
//
//	m_pwszBuffer = pTempBuffer;
//	m_nBufferSize = nAllocateSize;
//	return true;
//}
//
//
////--------------------------------------------------------------------------------------
//// Uniscribe -- Analyse() analyses the string in the buffer
////--------------------------------------------------------------------------------------
//HRESULT CUniBuffer::Analyse()
//{
//	if (m_Analysis)
//		_ScriptStringFree(&m_Analysis);
//
//	SCRIPT_CONTROL ScriptControl; // For uniscribe
//	SCRIPT_STATE ScriptState;   // For uniscribe
//	ZeroMemory(&ScriptControl, sizeof(ScriptControl));
//	ZeroMemory(&ScriptState, sizeof(ScriptState));
//	_ScriptApplyDigitSubstitution(NULL, &ScriptControl, &ScriptState);
//
//	if (!m_pFontNode)
//		return E_FAIL;
//
//	HDC hDC = /*m_pFontNode->pFont10 ? m_pFontNode->pFont10->GetDC() :*/
//		(HDC)(m_pFontNode->pFont ? m_pFontNode->pFont->GetSafeHandle() : NULL);
//	HRESULT hr = _ScriptStringAnalyse(hDC,
//		m_pwszBuffer,
//		lstrlenW(m_pwszBuffer) + 1,  // NULL is also analyzed.
//		lstrlenW(m_pwszBuffer) * 3 / 2 + 16,
//		-1,
//		SSA_BREAK | SSA_GLYPHS | SSA_FALLBACK | SSA_LINK,
//		0,
//		&ScriptControl,
//		&ScriptState,
//		NULL,
//		NULL,
//		NULL,
//		&m_Analysis);
//	if (SUCCEEDED(hr))
//		m_bAnalyseRequired = false;  // Analysis is up-to-date
//	return hr;
//}
//
//
////--------------------------------------------------------------------------------------
//WCHAR& CUniBuffer::operator[](int n)  // No param checking
//{
//	// This version of operator[] is called only
//	// if we are asking for write access, so
//	// re-analysis is required.
//	m_bAnalyseRequired = true;
//	return m_pwszBuffer[n];
//}
//
//
////--------------------------------------------------------------------------------------
//void CUniBuffer::Clear()
//{
//	*m_pwszBuffer = L'\0';
//	m_bAnalyseRequired = true;
//}
//
//
////--------------------------------------------------------------------------------------
//// Inserts the char at specified index.
//// If nIndex == -1, insert to the end.
////--------------------------------------------------------------------------------------
//bool CUniBuffer::InsertChar(int nIndex, WCHAR wChar)
//{
//	assert(nIndex >= 0);
//
//	if (nIndex < 0 || nIndex > lstrlenW(m_pwszBuffer))
//		return false;  // invalid index
//
//					   // Check for maximum length allowed
//	if (GetTextSize() + 1 >= GUI_MAX_EDITBOXLENGTH)
//		return false;
//
//	if (lstrlenW(m_pwszBuffer) + 1 >= m_nBufferSize)
//	{
//		if (!SetBufferSize(-1))
//			return false;  // out of memory
//	}
//
//	assert(m_nBufferSize >= 2);
//
//	// Shift the characters after the index, start by copying the null terminator
//	WCHAR* dest = m_pwszBuffer + lstrlenW(m_pwszBuffer) + 1;
//	WCHAR* stop = m_pwszBuffer + nIndex;
//	WCHAR* src = dest - 1;
//
//	while (dest > stop)
//	{
//		*dest-- = *src--;
//	}
//
//	// Set new character
//	m_pwszBuffer[nIndex] = wChar;
//	m_bAnalyseRequired = true;
//
//	return true;
//}
//
//
////--------------------------------------------------------------------------------------
//// Removes the char at specified index.
//// If nIndex == -1, remove the last char.
////--------------------------------------------------------------------------------------
//bool CUniBuffer::RemoveChar(int nIndex)
//{
//	if (!lstrlenW(m_pwszBuffer) || nIndex < 0 || nIndex >= lstrlenW(m_pwszBuffer))
//		return false;  // Invalid index
//
//	MoveMemory(m_pwszBuffer + nIndex, m_pwszBuffer + nIndex + 1, sizeof(WCHAR) *
//		(lstrlenW(m_pwszBuffer) - nIndex));
//	m_bAnalyseRequired = true;
//	return true;
//}
//
//
////--------------------------------------------------------------------------------------
//// Inserts the first nCount characters of the string pStr at specified index.
//// If nCount == -1, the entire string is inserted.
//// If nIndex == -1, insert to the end.
////--------------------------------------------------------------------------------------
//bool CUniBuffer::InsertString(int nIndex, const WCHAR* pStr, int nCount)
//{
//	assert(nIndex >= 0);
//	if (nIndex < 0)
//		return false;
//
//	if (nIndex > lstrlenW(m_pwszBuffer))
//		return false;  // invalid index
//
//	if (-1 == nCount)
//		nCount = lstrlenW(pStr);
//
//	// Check for maximum length allowed
//	if (GetTextSize() + nCount >= GUI_MAX_EDITBOXLENGTH)
//		return false;
//
//	if (lstrlenW(m_pwszBuffer) + nCount >= m_nBufferSize)
//	{
//		if (!SetBufferSize(lstrlenW(m_pwszBuffer) + nCount + 1))
//			return false;  // out of memory
//	}
//
//	MoveMemory(m_pwszBuffer + nIndex + nCount, m_pwszBuffer + nIndex, sizeof(WCHAR) *
//		(lstrlenW(m_pwszBuffer) - nIndex + 1));
//	CopyMemory(m_pwszBuffer + nIndex, pStr, nCount * sizeof(WCHAR));
//	m_bAnalyseRequired = true;
//
//	return true;
//}
//
//
////--------------------------------------------------------------------------------------
//bool CUniBuffer::SetText(LPCWSTR wszText)
//{
//	assert(wszText != NULL);
//
//	int nRequired = int(wcslen(wszText) + 1);
//
//	// Check for maximum length allowed
//	if (nRequired >= GUI_MAX_EDITBOXLENGTH)
//		return false;
//
//	while (GetBufferSize() < nRequired)
//		if (!SetBufferSize(-1))
//			break;
//	// Check again in case out of memory occurred inside while loop.
//	if (GetBufferSize() >= nRequired)
//	{
//		wcscpy_s(m_pwszBuffer, GetBufferSize(), wszText);
//		m_bAnalyseRequired = true;
//		return true;
//	}
//	else
//		return false;
//}
//
//
////--------------------------------------------------------------------------------------
//HRESULT CUniBuffer::CPtoX(int nCP, BOOL bTrail, int* pX)
//{
//	assert(pX);
//	*pX = 0;  // Default
//
//	HRESULT hr = S_OK;
//	if (m_bAnalyseRequired)
//		hr = Analyse();
//
//	if (SUCCEEDED(hr))
//		hr = _ScriptStringCPtoX(m_Analysis, nCP, bTrail, pX);
//
//	return hr;
//}
//
//
////--------------------------------------------------------------------------------------
//HRESULT CUniBuffer::XtoCP(int nX, int* pCP, int* pnTrail)
//{
//	assert(pCP && pnTrail);
//	*pCP = 0; *pnTrail = FALSE;  // Default
//
//	HRESULT hr = S_OK;
//	if (m_bAnalyseRequired)
//		hr = Analyse();
//
//	if (SUCCEEDED(hr))
//		hr = _ScriptStringXtoCP(m_Analysis, nX, pCP, pnTrail);
//
//	// If the coordinate falls outside the strText region, we
//	// can get character positions that don't exist.  We must
//	// filter them here and convert them to those that do exist.
//	if (*pCP == -1 && *pnTrail == TRUE)
//	{
//		*pCP = 0; *pnTrail = FALSE;
//	}
//	else if (*pCP > lstrlenW(m_pwszBuffer) && *pnTrail == FALSE)
//	{
//		*pCP = lstrlenW(m_pwszBuffer); *pnTrail = TRUE;
//	}
//
//	return hr;
//}
//
//
////--------------------------------------------------------------------------------------
//void CUniBuffer::GetPriorItemPos(int nCP, int* pPrior)
//{
//	*pPrior = nCP;  // Default is the char itself
//
//	if (m_bAnalyseRequired)
//		if (FAILED(Analyse()))
//			return;
//
//	const SCRIPT_LOGATTR* pLogAttr = _ScriptString_pLogAttr(m_Analysis);
//	if (!pLogAttr)
//		return;
//
//	if (!_ScriptString_pcOutChars(m_Analysis))
//		return;
//	int nInitial = *_ScriptString_pcOutChars(m_Analysis);
//	if (nCP - 1 < nInitial)
//		nInitial = nCP - 1;
//	for (int i = nInitial; i > 0; --i)
//		if (pLogAttr[i].fWordStop ||       // Either the fWordStop flag is set
//			(!pLogAttr[i].fWhiteSpace &&  // Or the previous char is whitespace but this isn't.
//				pLogAttr[i - 1].fWhiteSpace))
//		{
//			*pPrior = i;
//			return;
//		}
//	// We have reached index 0.  0 is always a break point, so simply return it.
//	*pPrior = 0;
//}
//
//
////--------------------------------------------------------------------------------------
//void CUniBuffer::GetNextItemPos(int nCP, int* pPrior)
//{
//	*pPrior = nCP;  // Default is the char itself
//
//	HRESULT hr = S_OK;
//	if (m_bAnalyseRequired)
//		hr = Analyse();
//	if (FAILED(hr))
//		return;
//
//	const SCRIPT_LOGATTR* pLogAttr = _ScriptString_pLogAttr(m_Analysis);
//	if (!pLogAttr)
//		return;
//
//	if (!_ScriptString_pcOutChars(m_Analysis))
//		return;
//	int nInitial = *_ScriptString_pcOutChars(m_Analysis);
//	if (nCP + 1 < nInitial)
//		nInitial = nCP + 1;
//
//	int i = nInitial;
//	int limit = *_ScriptString_pcOutChars(m_Analysis);
//	while (limit > 0 && i < limit - 1)
//	{
//		if (pLogAttr[i].fWordStop)      // Either the fWordStop flag is set
//		{
//			*pPrior = i;
//			return;
//		}
//		else if (pLogAttr[i].fWhiteSpace &&  // Or this whitespace but the next char isn't.
//			!pLogAttr[i + 1].fWhiteSpace)
//		{
//			*pPrior = i + 1;  // The next char is a word stop
//			return;
//		}
//
//		++i;
//		limit = *_ScriptString_pcOutChars(m_Analysis);
//	}
//	// We have reached the end. It's always a word stop, so simply return it.
//	*pPrior = *_ScriptString_pcOutChars(m_Analysis) - 1;
//}

bool TimeString(WCHAR *dest, size_t size, double msec)
{
	if (dest == NULL || size <= 0)
		return false;


	int mins = (int)(msec / 60);
	int hours = (int)(msec / 3600);
	int days = (int)(msec / 86400);
	if (msec < 60)
		StringCchPrintfW(dest, size, L"%.3f", msec);
	else if (msec < 3600)
	{
		float secs = msec - 60 * mins;
		if (secs < 10)
			StringCchPrintfW(dest, size, L"%2d:0%2.3f", mins, secs);
		else
			StringCchPrintfW(dest, size, L"%2d:%2.3f", mins, secs);
	}
	else if (msec < 86400)
	{
		mins -= 60 * hours;
		float secs = msec - 3600 * hours - 60 * mins;
		if (secs < 10)
			StringCchPrintfW(dest, size, L"%2d:%02d:0%2.3f", hours, mins, secs);
		else
			StringCchPrintfW(dest, size, L"%2d:%02d:%2.3f", hours, mins, secs);
	}
	else
	{
		hours -= 24 * days;
		mins -= 1440 * days + 60 * hours;
		float secs = msec - (double)86400 * days - (double)3600 * hours - (double)60 * mins;
		if (secs < 10)
			StringCchPrintfW(dest, size, L"%dday %02d:%02d:0%2.3f", days, hours, mins, secs);
		else
			StringCchPrintfW(dest, size, L"%dday %02d:%02d:%2.3f", days, hours, mins, secs);
	}
	return true;
}

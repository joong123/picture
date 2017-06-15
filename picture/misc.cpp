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

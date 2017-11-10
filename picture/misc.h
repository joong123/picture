#pragma once

#include<windows.h>
#include<strsafe.h>

bool TimeString(WCHAR *dest, size_t size, double msec);

//--------------------------------------------------------------------------------------
// A growable array
//--------------------------------------------------------------------------------------
template<typename TYPE> class CGrowableArray
{
public:
	CGrowableArray()
	{
		m_pData = NULL; m_nSize = 0; m_nMaxSize = 0; m_nCursor = 0;
	}
	CGrowableArray(const CGrowableArray <TYPE>& a)
	{
		//m_pData = NULL; m_nSize = 0; m_nMaxSize = 0; m_nCursor = 0;//TODO：不需要变量初始化？
		for (int i = 0; i < a.m_nSize; i++) Add(a.m_pData[i]);
	}
	~CGrowableArray()
	{
		RemoveAll();
	}

	const TYPE& operator[](int nIndex) const
	{
		return GetAt(nIndex);
	}
	TYPE& operator[](int nIndex)
	{
		return GetAt(nIndex);
	}

	CGrowableArray& operator=(const CGrowableArray <TYPE>& a)
	{
		if (this == &a) return *this; RemoveAll(); for (int i = 0; i < a.m_nSize;
			i++) Add(a.m_pData[i]); return *this;
	}

	HRESULT SetSize(int nNewMaxSize)
	{
		int nOldSize = m_nSize;

		if (nOldSize > nNewMaxSize)
		{
			//assert(m_pData);
			if (m_pData)
			{
				// Removing elements. Call dtor.

				for (int i = nNewMaxSize; i < nOldSize; ++i)
					m_pData[i].~TYPE();
			}
		}

		// Adjust buffer.  Note that there's no need to check for error
		// since if it happens, nOldSize == nNewMaxSize will be true.)
		HRESULT hr = SetSizeInternal(nNewMaxSize);

		if (nOldSize < nNewMaxSize)
		{
			//assert(m_pData);
			if (m_pData)
			{
				// Adding elements. Call ctor.

				for (int i = nOldSize; i < nNewMaxSize; ++i)
					::new (&m_pData[i]) TYPE;
			}
		}

		return hr;
	}
	HRESULT Add(const TYPE& value)
	{
		HRESULT hr;
		if (FAILED(hr = SetSizeInternal(m_nSize + 1)))
			return hr;

		// Construct the new element
		::new (&m_pData[m_nSize]) TYPE;

		// Assign
		m_pData[m_nSize] = value;
		++m_nSize;

		return S_OK;
	}
	HRESULT Insert(int nIndex, const TYPE& value);
	HRESULT SetAt(int nIndex, const TYPE& value);
	TYPE& GetAt(int nIndex) const
	{
		/*assert(nIndex >= 0 && nIndex < m_nSize);*/ return m_pData[nIndex];
	}
	TYPE GetAtSafe(int nIndex) const
	{
		/*assert(nIndex >= 0 && nIndex < m_nSize);*/ return m_pData[nIndex];
	}
	TYPE* GetPAt(int nIndex) const//自己加的
	{
		if (nIndex < 0 || nIndex >= m_nSize)
			return NULL;
		else
			return (TYPE*)&(m_pData[nIndex]);
	}
	int     GetSize() const
	{
		return m_nSize;
	}
	int		GetCapacity() const//自己加的
	{
		return m_nMaxSize;
	}
	TYPE* GetData()
	{
		return m_pData;
	}
	bool    Contains(const TYPE& value)
	{
		return (-1 != IndexOf(value));
	}

	int     IndexOf(const TYPE& value)
	{
		return (m_nSize > 0) ? IndexOf(value, 0, m_nSize) : -1;
	}
	int     IndexOf(const TYPE& value, int iStart)
	{
		return IndexOf(value, iStart, m_nSize - iStart);
	}
	int     IndexOf(const TYPE& value, int nIndex, int nNumElements);

	int     LastIndexOf(const TYPE& value)
	{
		return (m_nSize > 0) ? LastIndexOf(value, m_nSize - 1, m_nSize) : -1;
	}
	int     LastIndexOf(const TYPE& value, int nIndex)
	{
		return LastIndexOf(value, nIndex, nIndex + 1);
	}
	int     LastIndexOf(const TYPE& value, int nIndex, int nNumElements);

	HRESULT Remove(int nIndex)
	{
		if (nIndex < 0 ||
			nIndex >= m_nSize)
		{
			//assert(false);
			return E_INVALIDARG;
		}

		// Destruct the element to be removed
		m_pData[nIndex].~TYPE();

		// Compact the array and decrease the size
		MoveMemory(&m_pData[nIndex], &m_pData[nIndex + 1], sizeof(TYPE) * (m_nSize - (nIndex + 1)));
		--m_nSize;

		if (m_nCursor >= m_nSize)//自己加的
			m_nCursor = m_nSize;

		return S_OK;
	}
	void    RemoveAll()
	{
		SetSize(0);
	}
	void    Reset()
	{
		m_nSize = 0;
	}

	int		SetCursor(int cursor)
	{
		if (cursor >= 0 && cursor < m_nSize)
			m_nCursor = cursor;
		return m_nCursor;
	}
	int		SetCursorNext()
	{
		m_nCursor++;
		if (m_nCursor >= m_nSize)
			m_nCursor = 0;
	}
	int		SetCursorPrev()
	{
		m_nCursor--;
		if (m_nCursor < 0)
			m_nCursor = max(m_nSize - 1, 0);
	}
	int		SetCursorTail()
	{
		m_nCursor = max(m_nSize - 1, 0);
	}

protected:
	TYPE* m_pData;      // the actual array of samples
	int m_nSize;        // # of elements (upperBound - 1)
	int m_nMaxSize;     // max allocated
	int m_nCursor;		// 游标[自己添加的]

	HRESULT SetSizeInternal(int nNewMaxSize)  // This version doesn't call ctor or dtor.
	{
		if (nNewMaxSize < 0 || (nNewMaxSize > INT_MAX / sizeof(TYPE)))
		{
			//assert(false);
			return E_INVALIDARG;
		}

		if (nNewMaxSize == 0)
		{
			// Shrink to 0 size & cleanup
			if (m_pData)
			{
				free(m_pData);
				m_pData = NULL;
			}

			m_nMaxSize = 0;
			m_nSize = 0;
			m_nCursor = 0; //自己加的
		}
		else if (m_pData == NULL || nNewMaxSize > m_nMaxSize)
		{
			// Grow array
			int nGrowBy = (m_nMaxSize == 0) ? 16 : m_nMaxSize;

			// Limit nGrowBy to keep m_nMaxSize less than INT_MAX
			if ((UINT)m_nMaxSize + (UINT)nGrowBy > (UINT)INT_MAX)
				nGrowBy = INT_MAX - m_nMaxSize;

			nNewMaxSize = __max(nNewMaxSize, m_nMaxSize + nGrowBy);

			// Verify that (nNewMaxSize * sizeof(TYPE)) is not greater than UINT_MAX or the realloc will overrun
			if (sizeof(TYPE) > UINT_MAX / (UINT)nNewMaxSize)
				return E_INVALIDARG;

			TYPE* pDataNew = (TYPE*)realloc(m_pData, nNewMaxSize * sizeof(TYPE));
			if (pDataNew == NULL)
				return E_OUTOFMEMORY;

			m_pData = pDataNew;
			m_nMaxSize = nNewMaxSize;
		}

		return S_OK;
	}
};
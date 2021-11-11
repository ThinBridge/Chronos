#pragma once
class CItemIDList
{
public:
	LPITEMIDLIST m_pidl;
	CItemIDList() : m_pidl(NULL)
	{
	}
	CItemIDList(LPCITEMIDLIST pidl)
	{
		m_pidl = _CopyItemIDList(pidl);
	}
	virtual ~CItemIDList()
	{
		::CoTaskMemFree(m_pidl);
	}
	CString GetPath()
	{
		TCHAR szPath[MAX_PATH] = {0};
		if (::SHGetPathFromIDList(m_pidl, szPath))
			return szPath;
		else
			return CString();
	}

protected:
	UINT _GetSize(LPCITEMIDLIST pidl)
	{
		UINT cbTotal = 0;
		if (pidl)
		{
			cbTotal += sizeof(pidl->mkid.cb);
			while (pidl->mkid.cb)
			{
				cbTotal += pidl->mkid.cb;
				pidl = _Next(pidl);
			}
		}
		return cbTotal;
	}
	LPITEMIDLIST _Next(LPCITEMIDLIST pidl)
	{
		ATLASSERT(pidl != NULL);
		LPSTR lpMem = (LPSTR)pidl;
		lpMem += pidl->mkid.cb;
		return (LPITEMIDLIST)lpMem;
	}
	LPITEMIDLIST _CopyItemIDList(LPCITEMIDLIST lpi)
	{
		if (lpi == NULL)
			return NULL;
		UINT cb = _GetSize(lpi);
		LPITEMIDLIST lpiTemp = (LPITEMIDLIST)::CoTaskMemAlloc(cb);
		if (lpiTemp == NULL)
			return NULL;
		::memcpy(lpiTemp, lpi, cb);
		return lpiTemp;
	}
};

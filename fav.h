#pragma once
#include "itemidlist.h"
#include <intshcut.h>
#include "afxtempl.h"
#ifdef _DEBUG
#include <locale.h>
#endif
enum TYPE
{
	IEFavERR,
	IEFavROOT,
	IEFavDIR,
	IEFavURL,
	IEFavFILE,
	IEFavALL,
};

typedef CMap<CString, LPCTSTR, DWORD, DWORD> CFavoritesOrder;
typedef struct _tagCFavoritesOrderData
{
	DWORD size;
	DWORD priority;
	ITEMIDLIST idl;
} _CFavoritesOrderData;
#define FAVORITESORDER_NOTFOUND -5
class CIEFavOrder
{
public:
	CIEFavOrder()
	{
	}
	virtual ~CIEFavOrder()
	{
	}

	void MtlMakeSureTrailingBackSlash(CString& strDirectoryPath)
	{
		MtlMakeSureTrailingChar(strDirectoryPath, _T('\\'));
	}
	void MtlMakeSureTrailingChar(CString& strDirectoryPath, TCHAR ch)
	{
		if (!__MtlIsLastChar(strDirectoryPath, ch))
			strDirectoryPath += ch;
	}
	bool __MtlIsLastChar(const CString& str, TCHAR ch)
	{
		if (str.GetLength() <= 0)
			return false;

		if (str[str.GetLength() - 1] == ch)
			return true;
		else
			return false;
	}
	void MtlRemoveTrailingBackSlash(CString& str)
	{
		if (__MtlIsLastChar(str, _T('\\')))
			str = str.Left(str.GetLength() - 1);
	}
	void MtlGetFileName(const CString& strPath, CString& str)
	{
		int nIndex = strPath.ReverseFind(_T('\\'));

		if (nIndex == strPath.GetLength() - 1)
		{
			MtlGetDisplayTextFromPath(strPath, str);
			return;
		}

		str = strPath.Right(strPath.GetLength() - nIndex - 1);
	}
	bool MtlIsDirectory(const CString& strPath)
	{
		DWORD dw = ::GetFileAttributes(strPath);
		if (dw == 0xFFFFFFFF)
			return false;

		if (dw & FILE_ATTRIBUTE_DIRECTORY)
			return true;
		else
			return false;
	}
	bool MtlIsDirectoryPath(const CString& strPath, bool bUseSystem = false)
	{
		if (!bUseSystem)
		{
			if (__MtlIsLastChar(strPath, _T('\\')))
				return true;
			else
				return false;
		}
		else
		{
			return MtlIsDirectory(strPath);
		}
	}
	void MtlGetDisplayTextFromPath(const CString& strPath, CString& str)
	{
		str = strPath;
		if (MtlIsDirectoryPath(strPath))
		{
			int nIndex = str.ReverseFind(_T('\\'));
			str = str.Left(nIndex);
			nIndex = str.ReverseFind(_T('\\'));
			str = str.Right(str.GetLength() - nIndex - 1);
		}
		else
		{
			int nIndex = str.ReverseFind(_T('\\'));
			str = str.Right(str.GetLength() - nIndex - 1);
			nIndex = str.ReverseFind(_T('.'));
			if (nIndex != -1)
				str = str.Left(nIndex);
		}
		return;
	}

	void MtlGetFavoritesOrder(CFavoritesOrder& order, const CString& strDirPath)
	{
		ATLASSERT(!strDirPath.IsEmpty());
		const DWORD s_unknownOffset = 12;

		CString strRoot;
		TCHAR szFolder[MAX_PATH] = {0};
		::SHGetSpecialFolderPath(NULL, szFolder, CSIDL_FAVORITES, FALSE);
		strRoot = szFolder;
		MtlMakeSureTrailingBackSlash(strRoot);

		CString strKeyName = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MenuOrder\\Favorites");
		CString strRelative = strDirPath.Right(strDirPath.GetLength() - strRoot.GetLength());
		if (!strRelative.IsEmpty())
			strKeyName += _T('\\') + strRelative;
		MtlRemoveTrailingBackSlash(strKeyName);

		CRegKey rkOrder;
		LONG lRet = rkOrder.Open(HKEY_CURRENT_USER, strKeyName);
		if (lRet != ERROR_SUCCESS)
			return;

		DWORD dwSize = 0;
		DWORD dwType = REG_BINARY;
		lRet = ::RegQueryValueEx(rkOrder, _T("Order"), NULL, &dwType, NULL, &dwSize);
		if (lRet != ERROR_SUCCESS || dwSize < s_unknownOffset)
			return;

		BYTE* pByte = (BYTE*)_alloca((dwSize + 10) * sizeof(BYTE));
		::memset(pByte, 0, dwSize + 10);

		lRet = ::RegQueryValueEx(rkOrder, _T("Order"), NULL, &dwType, pByte, &dwSize);
		if (lRet != ERROR_SUCCESS)
			return;

		BYTE* pBegin = pByte + ((_CFavoritesOrderData*)pByte)->size + s_unknownOffset;
		BYTE* pEnd = pByte + dwSize;
		CString strName;
		while (pBegin < pEnd)
		{
			_CFavoritesOrderData* pData = (_CFavoritesOrderData*)pBegin;
			CItemIDList idl(&pData->idl);
			MtlGetFileName(idl.GetPath(), strName);
			if (!strName.IsEmpty())
			{
				order.SetAt(strName, pData->priority);
			}

			pBegin += pData->size;
		}
	}
};
class CFavoriteItem
{
public:
	CFavoriteItem()
	{
		bType = IEFavURL;
		commandID = 0;
	}
	CFavoriteItem(int iType)
	{
		bType = iType;
	}
	virtual ~CFavoriteItem()
	{
		CleanUP();
	}
	CString strURL;
	CString strTitle;

	INT_PTR commandID;
	void CleanUP()
	{
		CFavoriteItem* tmp = NULL;
		INT_PTR imax = m_aFavItems.GetSize();
		for (INT_PTR i = 0; i < imax; i++)
		{
			tmp = (CFavoriteItem*)m_aFavItems.GetAt(i);
			if (tmp)
				delete tmp;
		}
		m_aFavItems.RemoveAll();
	}
	CFavoriteItem* AddChild(LPCTSTR strTitle, LPCTSTR strURL, int iType)
	{
		if (bType == IEFavDIR || bType == IEFavROOT)
		{
			CFavoriteItem* tmp = NULL;
			tmp = new CFavoriteItem;
			if (tmp)
			{
				tmp->strTitle = strTitle;
				tmp->strURL = strURL;
				tmp->bType = iType;
				m_aFavItems.Add(tmp);
			}
			return tmp;
		}
		return NULL;
	}
	int GetType()
	{
		return bType;
	}
	void SortItem(CFavoritesOrder& order)
	{
		if (bType != IEFavDIR && bType != IEFavROOT)
			return;
		INT_PTR iColSize = m_aFavItems.GetSize();
		INT_PTR iOrderSize = order.GetCount();
		if (iColSize <= 1 || iOrderSize <= 1)
			return;
		INT_PTR* SortedItemArray = 0;
		SortedItemArray = new INT_PTR[iColSize];
		memset(SortedItemArray, 0x00, sizeof(INT_PTR) * iColSize);
		CString strName;
		CFavoriteItem* pFavTmp = NULL;
		CPtrArray ptraDuplicateData;
		for (int i = 0; i < iColSize; i++)
		{
			pFavTmp = (CFavoriteItem*)m_aFavItems.GetAt(i);
			if (pFavTmp)
			{
				strName = pFavTmp->strTitle;
				//get
				DWORD iorderno = 0;
				if (order.Lookup(strName, iorderno))
				{
					if (0 <= iorderno && iorderno < (DWORD)iColSize)
					{
						if (SortedItemArray[iorderno] == 0)
						{
							SortedItemArray[iorderno] = (INT_PTR)pFavTmp;
						}
						else
						{
							ptraDuplicateData.Add(pFavTmp);
						}
					}
					else
					{
						ptraDuplicateData.Add(pFavTmp);
					}
				}
				else
				{
					ptraDuplicateData.Add(pFavTmp);
				}
			}
		}
		m_aFavItems.RemoveAll();
		pFavTmp = NULL;
		for (int i = 0; i < iColSize; i++)
		{
			pFavTmp = (CFavoriteItem*)SortedItemArray[i];
			if (pFavTmp)
			{
				m_aFavItems.Add(pFavTmp);
			}
		}
		INT_PTR iMax = ptraDuplicateData.GetSize();
		pFavTmp = NULL;
		for (INT_PTR i = 0; i < iMax; i++)
		{
			pFavTmp = (CFavoriteItem*)ptraDuplicateData.GetAt(i);
			if (pFavTmp)
			{
				m_aFavItems.Add(pFavTmp);
			}
		}
		if (SortedItemArray)
		{
			delete[] SortedItemArray;
			SortedItemArray = NULL;
		}
	}
	void SortItem()
	{
		if (bType != IEFavDIR && bType != IEFavROOT)
			return;
		INT_PTR iColSize = m_aFavItems.GetSize();
		if (iColSize <= 1)
			return;
		INT_PTR* SortedItemArray1 = 0;
		SortedItemArray1 = new INT_PTR[iColSize];
		memset(SortedItemArray1, 0x00, sizeof(INT_PTR) * iColSize);

		INT_PTR* SortedItemArray2 = 0;
		SortedItemArray2 = new INT_PTR[iColSize];
		memset(SortedItemArray2, 0x00, sizeof(INT_PTR) * iColSize);

		CFavoriteItem* pFavTmp = NULL;
		int iIndex1 = 0;
		int iIndex2 = 0;
		for (int i = 0; i < iColSize; i++)
		{
			pFavTmp = (CFavoriteItem*)m_aFavItems.GetAt(i);
			if (pFavTmp)
			{
				if (pFavTmp->bType == IEFavDIR)
				{
					SortedItemArray1[iIndex1] = (INT_PTR)pFavTmp;
					iIndex1++;
				}
				else if (pFavTmp->bType == IEFavURL || pFavTmp->bType == IEFavFILE)
				{
					SortedItemArray2[iIndex2] = (INT_PTR)pFavTmp;
					iIndex2++;
				}
			}
		}
		m_aFavItems.RemoveAll();
		pFavTmp = NULL;
		for (int i = 0; i < iColSize; i++)
		{
			pFavTmp = (CFavoriteItem*)SortedItemArray1[i];
			if (pFavTmp)
			{
				m_aFavItems.Add(pFavTmp);
			}
			else
				break;
		}
		pFavTmp = NULL;
		for (int i = 0; i < iColSize; i++)
		{
			pFavTmp = (CFavoriteItem*)SortedItemArray2[i];
			if (pFavTmp)
			{
				m_aFavItems.Add(pFavTmp);
			}
			else
				break;
		}
		if (SortedItemArray1)
		{
			delete[] SortedItemArray1;
			SortedItemArray1 = NULL;
		}
		if (SortedItemArray2)
		{
			delete[] SortedItemArray2;
			SortedItemArray2 = NULL;
		}
	}
	CFavoriteItem* GetItem(INT_PTR iindex)
	{
		if (0 <= iindex && iindex < m_aFavItems.GetSize())
		{
			CFavoriteItem* tmp = NULL;
			tmp = (CFavoriteItem*)m_aFavItems.GetAt(iindex);
			if (tmp)
			{
				return tmp;
			}
		}
		return NULL;
	}
	void SetItem(int iindex, CFavoriteItem* tmp)
	{
		if (0 <= iindex && iindex < m_aFavItems.GetSize())
		{
			m_aFavItems.SetAt(iindex, tmp);
		}
		return;
	}
	void AddItem(CFavoriteItem* tmp)
	{
		if (tmp)
		{
			m_aFavItems.Add(tmp);
		}
	}
	INT_PTR GetSize()
	{
		return m_aFavItems.GetSize();
	}
	void FavItemRemoveAll()
	{
		m_aFavItems.RemoveAll();
	}
	void AddPtrArray(CPtrArray* ptraFavItem)
	{
		this->commandID = ptraFavItem->GetSize();
		ptraFavItem->Add(this);
		if (bType == IEFavDIR || bType == IEFavROOT)
		{
			for (int i = 0; i < m_aFavItems.GetSize(); i++)
			{
				CFavoriteItem* tmp = NULL;
				tmp = (CFavoriteItem*)m_aFavItems.GetAt(i);
				if (tmp)
				{
					tmp->AddPtrArray(ptraFavItem);
				}
			}
		}
	}

protected:
	CPtrArray m_aFavItems;
	BOOL bType;
};
class CFavoriteItemManager
{
public:
	CFavoriteItemManager()
	{
		m_FavRootItem = NULL;
		m_iRet = 0;
	}
	virtual ~CFavoriteItemManager()
	{
		CleanUP();
	}
	void Init(const CString& strPath, BOOL IEOrder)
	{
		CleanUP();
		bIEOrder = IEOrder;
		m_FavRootItem = new CFavoriteItem(IEFavROOT);
		m_FavRootItem->strURL = strPath;
		m_FavRootItem->strTitle = _T("FAVROOT");
		m_ptraFavItemDir.RemoveAll();
		m_ptraFavItem.RemoveAll();
		m_ptraFavItemDir.Add(m_FavRootItem);
	}
	void CleanUP()
	{
		if (m_FavRootItem)
		{
			delete m_FavRootItem;
			m_FavRootItem = NULL;
		}
	}

	CFavoriteItem* AddChild(CFavoriteItem* parentItem, LPCTSTR strTitle, LPCTSTR strURL, int iType)
	{
		if (!parentItem) return NULL;

		CFavoriteItem* tmp = NULL;
		tmp = parentItem->AddChild(strTitle, strURL, iType);
		if (iType == IEFavDIR)
		{
			m_ptraFavItemDir.Add(tmp);
		}
		return tmp;
	}
	CFavoriteItem* GetRootItem()
	{
		return m_FavRootItem;
	}
	BOOL CreateFavoriteTree(CString& strRootPath, BOOL IEOrder)
	{
		CIEFavOrder ord;
		ord.MtlMakeSureTrailingBackSlash(strRootPath);
		this->Init(strRootPath, IEOrder);
		m_iRet = 0;
		this->Reflect(m_FavRootItem, strRootPath);
		if (m_iRet == 0)
		{
			CString strPath;
			INT_PTR iMax = m_ptraFavItemDir.GetSize();
			for (INT_PTR i = 0; i < iMax; i++)
			{
				CFavoriteItem* tmp = NULL;
				tmp = (CFavoriteItem*)m_ptraFavItemDir.GetAt(i);
				if (tmp)
				{
					if (IEOrder)
					{
						CFavoritesOrder order;
						strPath = tmp->strURL;
						ord.MtlMakeSureTrailingBackSlash(strPath);
						ord.MtlGetFavoritesOrder(order, strPath);
						tmp->SortItem(order);
						order.RemoveAll();
					}
					else
					{
						tmp->SortItem();
					}
				}
			}
			if (m_FavRootItem)
			{
				m_ptraFavItem.RemoveAll();
				m_FavRootItem->AddPtrArray(&m_ptraFavItem);
			}
		}
		return m_iRet == 0 ? TRUE : FALSE;
	}
	void Reflect(CFavoriteItem* parentItem, const CString& strPath)
	{
		CIEFavOrder ord;
		CString strFindPath = strPath + _T("*.*");
		WIN32_FIND_DATA wfd = {0};
		HANDLE h = ::FindFirstFile(strFindPath, &wfd);
		if (h == INVALID_HANDLE_VALUE)
		{
			m_iRet = (INT_PTR)INVALID_HANDLE_VALUE;
			return;
		}
		// Now scan the directory
		BOOL bIncludeHidden = FALSE;
		CString URLString;
		CString FileExt;
		CString URLPATH;
		CString strFileNameTemp;
		CString strDirectoryPath;
		do
		{
			if (m_iRet != 0)
				break;
			BOOL bDirectory = wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
			BOOL bVisible = bIncludeHidden ? TRUE : ((wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0);

			if (bVisible == FALSE)
				continue;

			if (bDirectory)
			{
				if (::lstrcmp(wfd.cFileName, _T(".")) == 0 || ::lstrcmp(wfd.cFileName, _T("..")) == 0)
					continue;

				strDirectoryPath = strPath + wfd.cFileName;
				ord.MtlMakeSureTrailingBackSlash(strDirectoryPath);
				CFavoriteItem* parentItemSub = NULL;
				parentItemSub = this->AddChild(parentItem, wfd.cFileName, strDirectoryPath, IEFavDIR);
				if (parentItemSub)
					this->Reflect(parentItemSub, strDirectoryPath);
			}
			else
			{
				strFileNameTemp = wfd.cFileName;
				FileExt = strFileNameTemp.Mid(strFileNameTemp.ReverseFind('.'));
				if (FileExt.CompareNoCase(_T(".url")) == 0
				|| FileExt.CompareNoCase(_T(".html")) == 0
				|| FileExt.CompareNoCase(_T(".htm")) ==0)
				{
					URLPATH = strPath + wfd.cFileName;
					GetInternetShortcutUrl(URLPATH, URLString);
					if (!URLString.IsEmpty())
					{
						this->AddChild(parentItem, strFileNameTemp, URLString, IEFavURL);
					}
				}
				else
				{
					URLPATH = strPath + wfd.cFileName;
					this->AddChild(parentItem, strFileNameTemp, URLPATH, IEFavFILE);
				}
			}

		} while (::FindNextFile(h, &wfd));
		::FindClose(h);
		return;
	}
	CFavoriteItem* GetItemData(int iindex)
	{
		if (0 <= iindex && iindex < m_ptraFavItem.GetSize())
		{
			CFavoriteItem* tmp = NULL;
			tmp = (CFavoriteItem*)m_ptraFavItem.GetAt(iindex);
			if (tmp)
			{
				return tmp;
			}
		}
		return NULL;
	}
	static inline BOOL IsURL_HTTP(LPCTSTR str)
	{
		if (str == NULL)
			return FALSE;
		CString strCheckStr = str;
		if (strCheckStr.Find(_T("http://")) == 0 || //http
		    strCheckStr.Find(_T("https://")) == 0   //https
		)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
		return FALSE;
	}
	void GetInternetShortcutUrl(LPCTSTR pszFolder, CString& strVal)
	{
		TCHAR szURL[4096] = {0};
		DWORD nSize = 4096;
		::GetPrivateProfileString(_T("InternetShortcut"), _T("URL"), NULL, szURL, nSize, pszFolder);
		strVal = szURL;
		if (!IsURL_HTTP(strVal))
		{
			strVal.Empty();
		}
		return;
	}

protected:
	CFavoriteItem* m_FavRootItem;
	CPtrArray m_ptraFavItemDir;
	CPtrArray m_ptraFavItem;

	int m_iRet;
	int iindex;
	int iLinkCnt;
	BOOL bIEOrder;
};

#pragma once

#include "stdafx.h"
#include "IResource.h"
#include "types.h"

using namespace std;
using namespace std::tr1;

class CWin32File;
class CPackResource : public IResource {
public:
  CPackResource(tstring name, shared_ptr<CWin32File> spFile, ITEM_INFO *pInfo,
                int level);

  virtual ~CPackResource(void);

  virtual LPCTSTR GetName();

  virtual size_t GetDecompressedContent(char *pBuffer, size_t size);

  virtual size_t GetCompressedContent(char *pBuffer, size_t size, int level);

  virtual size_t GetCompressedSize();

  virtual size_t GetDecompressedSize();

  virtual size_t GetVersion();

  virtual void Release();

protected:
  tstring m_name;
  shared_ptr<CWin32File> m_spFile;
  ITEM_INFO m_info;
  int m_level;
};

#pragma once
#include "IResourceSet.h"
#include "stdafx.h"

using namespace std;
using namespace std::tr1;

class CPackResourceSet : public IResourceSet {
public:
  friend class CResourceFileSystem;

  CPackResourceSet();

  virtual ~CPackResourceSet(void);

  bool Open(LPCTSTR lpszPackFile);

  virtual int FindResourceIndex(LPCTSTR lpszName);

  virtual IResource *GetResource(size_t index);

  virtual size_t GetResourceCount();

  virtual void Release();

protected:
  vector<shared_ptr<IResource>> m_Resources;
};

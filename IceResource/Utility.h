#pragma once

#include "IResource.h"
#include "clBlowFish.h"
#include <memory>
#include <vector>

//#pragma comment(lib, "zlib125-dll/lib/zdll.lib")

using namespace std;
using namespace std::tr1;

class ResourceGreater {
public:
  bool operator()(shared_ptr<IResource> spResource1,
                  shared_ptr<IResource> spResource2) {
    return spResource1->GetName() > spResource2->GetName();
  }
};

class CUtility {
public:
  CUtility(void);
  ~CUtility(void);

  static int BinaryFindResourceIndex(vector<shared_ptr<IResource>> &resList,
                                     LPCTSTR lpszName);

  static int FindResourceIndex(vector<shared_ptr<IResource>> &resList,
                               LPCTSTR lpszName);

  unsigned char *Compress(unsigned char *data, unsigned int originalSize,
	  unsigned int *dataSize);
  unsigned char *Uncompress(unsigned char *data, unsigned int dataSize,
	  unsigned int originalSize);
  void Encrypt(void *data, unsigned int size, unsigned int version,
	  unsigned int blowFishKey1, unsigned int blowFishKey2);
  void Decrypt(void *data, unsigned int size, unsigned int version,
	  unsigned int blowFishKey1, unsigned int blowFishKey2);
};

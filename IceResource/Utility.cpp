#include "Utility.h"
#include "StdAfx.h"
#include "clCompress.h"
#include "clBlowFish.h"

CUtility::CUtility(void) {}

CUtility::~CUtility(void) {}

int CUtility::FindResourceIndex(vector<shared_ptr<IResource>> &resList,
                                LPCTSTR lpszName) {
  for (size_t i = 0; i < resList.size(); i++) {
    if (lstrcmpi(resList.at(i)->GetName(), lpszName) == 0) {
      return i;
    }
  }

  return -1;
}

int CUtility::BinaryFindResourceIndex(vector<shared_ptr<IResource>> &resList,
                                      LPCTSTR lpszName) {
  size_t begin = 0, end = resList.size() - 1;
  while (begin <= end) {
    int middle = (end - begin) / 2 + begin;
    shared_ptr<IResource> spResrouce = resList.at(middle);
    int result = lstrcmpi(spResrouce->GetName(), lpszName);

    if (result == 0) {
      return middle;
    } else if (result > 0) {
      begin = middle + 1;
    } else {
      end = middle - 1;
    }
  }

  return -1;
}

void CUtility::Encrypt(void *data, unsigned int size, unsigned int version,
	unsigned int blowFishKey1, unsigned int blowFishKey2) {
	if (data && size) {
		if (version < 4) {
			clBlowFish bf(&blowFishKey1, sizeof(blowFishKey1), true);
			bf.Encrypt(data, size);
		}
		else {
			if ((version < 5 && size <= 0x19000) ||
				(version >= 5 && version <= 9 && size <= 0x25800)) {
				clBlowFish bf(&blowFishKey2, sizeof(blowFishKey2), true);
				bf.Encrypt(data, size);
			}
			{
				clBlowFish bf(&blowFishKey1, sizeof(blowFishKey1), true);
				bf.Encrypt(data, size);
			}
			unsigned char x =
				(unsigned char)(blowFishKey1 ^
				(blowFishKey1 >>
					(version < 5 ? 16
						: version <= 9 ? version + 5 : 0)));
			for (unsigned char *s = (unsigned char *)data, *e = s + size; s < e; s++)
				if (*s && *s != x)
					*s ^= x;
		}
	}
}

void CUtility::Decrypt(void *data, unsigned int size, unsigned int version,
	unsigned int blowFishKey1, unsigned int blowFishKey2) {
	if (data && size) {
		if (version < 4) {
			clBlowFish bf(&blowFishKey1, sizeof(blowFishKey1), true);
			bf.Decrypt(data, size);
		}
		else {
			unsigned char x =
				(unsigned char)(blowFishKey1 ^
				(blowFishKey1 >>
					(version < 5 ? 16
						: version <= 9 ? version + 5 : 0)));
			for (unsigned char *s = (unsigned char *)data, *e = s + size; s < e; s++)
				if (*s && *s != x)
					*s ^= x;
			{
				clBlowFish bf(&blowFishKey1, sizeof(blowFishKey1), true);
				bf.Decrypt(data, size);
			}
			if ((version < 5 && size <= 0x19000) ||
				(version >= 5 && version <= 9 && size <= 0x25800)) {
				clBlowFish bf(&blowFishKey2, sizeof(blowFishKey2), true);
				bf.Decrypt(data, size);
			}
		}
	}
}

unsigned char *CUtility::Uncompress(unsigned char *data, unsigned int dataSize,
	unsigned int originalSize) {
	// Check
	if (!(data && dataSize && originalSize))
		return nullptr;

	// Decompression
	unsigned char *d = new unsigned char[originalSize];
	if (!d)
		return nullptr;
	clUncompress cmp;
	if (!cmp.Uncompress(d, data, originalSize, dataSize)) {
		delete[] d;
		return nullptr;
	}

	return d;
}

unsigned char * CUtility::Compress(unsigned char *data, unsigned int originalSize,
	unsigned int *dataSize) {

	// Check
	if (!(data && originalSize && dataSize))
		return nullptr;

	// Compression
	unsigned int size = originalSize + originalSize / 8 + 3;
	unsigned char *d = new unsigned char[size];
	if (!d)
		return nullptr;
	clCompress cmp;
	*dataSize = cmp.Compress(d, data, size, originalSize);

	return d;
}
unsigned char *CUtility::Uncompress(unsigned char *data, unsigned int dataSize,
	unsigned int originalSize) {

	// Check
	if (!(data && dataSize && originalSize))
		return nullptr;

	// Decompression
	unsigned char *d = new unsigned char[originalSize];
	if (!d)
		return nullptr;
	clUncompress cmp;
	if (!cmp.Uncompress(d, data, originalSize, dataSize)) {
		delete[] d;
		return nullptr;
	}

	return d;
}

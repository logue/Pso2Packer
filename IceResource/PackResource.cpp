#include "PackResource.h"
#include "StdAfx.h"

#include "Utility.h"
#include "Win32File.h"

CPackResource::CPackResource(tstring name, shared_ptr<CWin32File> spFile,
                             ITEM_INFO *pInfo, int level) {
  m_name = name;
  m_spFile = spFile;
  m_info = *pInfo;
  m_level = level;
}

CPackResource::~CPackResource(void) {}

LPCTSTR CPackResource::GetName() { return m_name.c_str(); }

size_t CPackResource::GetCompressedSize() { return m_info.compress_size; }

size_t CPackResource::GetDecompressedSize() { return m_info.decompress_size; }

size_t CPackResource::GetVersion() { return m_info.seed; }

size_t CPackResource::GetDecompressedContent(char *pBuffer, size_t size) {
  vector<char> compressContent(m_info.compress_size);

  GetCompressedContent(&compressContent[0], m_info.compress_size, 0);

  unsigned long decodeLen = size;

  decodeLen = CUtility::Uncompress(pBuffer, &compressContent[0],
                           m_info.compress_size);

  return decodeLen;
}

size_t CPackResource::GetCompressedContent(char *pBuffer, size_t size,
                                           int level) {
  m_spFile->Seek(m_info.offset, FILE_BEGIN);
  int tmp = m_spFile->Read(pBuffer, m_info.compress_size);
  if (tmp != m_info.compress_size) {
    return 0;
  }
  CUtility::Decrypt(pBuffer, tmp, m_info.seed);

  return tmp;
}

void CPackResource::Release() {
}

#pragma once

#include "ExportMacro.h"
#include "IObject.h"

class ICERESOURCE_API IResource : public IObject {
public:
  // Constructor
  IResource(void) {}

  // Destructor
  virtual ~IResource(void) {}

  // The entity's full name, is a relative path, such as db / ss.xml.
  virtual LPCTSTR GetName() = 0;

  // Decompressed content. (uncrypted)
  virtual size_t GetDecompressedContent(char *pBuffer, size_t size) = 0;

  // Compressed content .(uncrypted)
  virtual size_t GetCompressedContent(char *pBuffer, size_t size,
                                      int level) = 0;

  // Compressed content size.
  virtual size_t GetCompressedSize() = 0;

  // Decompressed content size.
  virtual size_t GetDecompressedSize() = 0;
};

#ifndef ZAR_HANDLER_H
#define ZAR_HANDLER_H

#include "Common.h"

#include <memory>
#include <string>
#include <vector>

#include <zarchive/zarchivereader.h>
#include <zarchive/zarchivewriter.h>

namespace NArchive {
namespace NZar {

struct CZarItem
{
  std::wstring Path;
  std::string Utf8Path;
  ZArchiveNodeHandle Handle = ZARCHIVE_INVALID_NODE;
  UInt64 Size = 0;
  bool IsDir = false;
};

Z7_CLASS_IMP_CHandler_IInArchive_1(IOutArchive)

  std::unique_ptr<ZArchiveReader> _reader;
  std::vector<CZarItem> _items;
  UInt64 _phySize = 0;

  void BuildItemList(ZArchiveNodeHandle dirHandle, const std::string &prefix);
};

}}

#endif

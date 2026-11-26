#include "StdAfx.h"
#include "ZarHandler.h"
#include "InStreamBuf.h"

#include <algorithm>
#include <cstring>

namespace NArchive {
namespace NZar {

static const PROPID kProps[] =
{
  kpidPath,
  kpidIsDir,
  kpidSize
};

static const PROPID kArcProps[] =
{
  kpidPhySize,
  kpidMethod
};

IMP_IInArchive_Props
IMP_IInArchive_ArcProps

void CHandler::BuildItemList(ZArchiveNodeHandle dirHandle, const std::string &prefix)
{
  const UInt32 count = _reader->GetDirEntryCount(dirHandle);
  for (UInt32 i = 0; i < count; i++)
  {
    ZArchiveReader::DirEntry entry;
    if (!_reader->GetDirEntry(dirHandle, i, entry))
      continue;

    std::string utf8Path = prefix.empty()
      ? std::string(entry.name)
      : prefix + "/" + std::string(entry.name);

    ZArchiveNodeHandle handle = _reader->LookUp(utf8Path);
    if (handle == ZARCHIVE_INVALID_NODE)
      continue;

    CZarItem item;
    item.Utf8Path = utf8Path;
    item.Handle = handle;
    item.IsDir = entry.isDirectory;
    item.Size = entry.isFile ? entry.size : 0;
    item.Path = Utf8ToWide(utf8Path.c_str(), utf8Path.size());
    for (size_t n = 0; n < item.Path.size(); n++)
      if (item.Path[n] == L'/')
        item.Path[n] = L'\\';

    _items.push_back(item);

    if (entry.isDirectory)
      BuildItemList(handle, utf8Path);
  }
}

Z7_COM7F_IMF(CHandler::GetArchiveProperty(PROPID propID, PROPVARIANT *value))
{
  value->vt = VT_EMPTY;
  switch (propID)
  {
    case kpidPhySize:
      value->vt = VT_UI8;
      value->uhVal.QuadPart = _phySize;
      break;
    case kpidMethod:
    {
      value->bstrVal = ::SysAllocString(L"Zstd");
      if (value->bstrVal)
        value->vt = VT_BSTR;
      break;
    }
  }
  return S_OK;
}

Z7_COM7F_IMF(CHandler::GetNumberOfItems(UInt32 *numItems))
{
  *numItems = (UInt32)_items.size();
  return S_OK;
}

Z7_COM7F_IMF(CHandler::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value))
{
  value->vt = VT_EMPTY;
  if (index >= (UInt32)_items.size())
    return E_INVALIDARG;

  const CZarItem &item = _items[index];
  switch (propID)
  {
    case kpidPath:
      PropVarSet_WStr(value, item.Path.c_str());
      break;
    case kpidIsDir:
      value->vt = VT_BOOL;
      value->boolVal = item.IsDir ? VARIANT_TRUE : VARIANT_FALSE;
      break;
    case kpidSize:
      if (!item.IsDir)
      {
        value->vt = VT_UI8;
        value->uhVal.QuadPart = item.Size;
      }
      break;
  }
  return S_OK;
}

Z7_COM7F_IMF(CHandler::Open(IInStream *stream,
    const UInt64 * /* maxCheckStartPosition */,
    IArchiveOpenCallback * /* openCallback */))
{
  Close();

  auto wrapped = std::make_unique<InStreamIStream>(stream);
  _phySize = wrapped->Size();

  ZArchiveReader *reader = ZArchiveReader::OpenFromStream(std::move(wrapped));
  if (!reader)
  {
    Close();
    return S_FALSE;
  }

  _reader.reset(reader);
  BuildItemList(0, std::string());
  return S_OK;
}

Z7_COM7F_IMF(CHandler::Close())
{
  _items.clear();
  _reader.reset();
  _phySize = 0;
  return S_OK;
}

Z7_COM7F_IMF(CHandler::Extract(const UInt32 *indices, UInt32 numItems,
    Int32 testMode, IArchiveExtractCallback *extractCallback))
{
  const bool allItems = (numItems == (UInt32)(Int32)-1);
  if (allItems)
    numItems = (UInt32)_items.size();

  UInt64 totalSize = 0;
  for (UInt32 i = 0; i < numItems; i++)
  {
    const UInt32 index = allItems ? i : indices[i];
    if (index < (UInt32)_items.size() && !_items[index].IsDir)
      totalSize += _items[index].Size;
  }
  extractCallback->SetTotal(totalSize);

  UInt64 completed = 0;
  std::vector<Byte> buffer(1024 * 1024);

  for (UInt32 i = 0; i < numItems; i++)
  {
    const UInt32 index = allItems ? i : indices[i];
    if (index >= (UInt32)_items.size())
      continue;

    const CZarItem &item = _items[index];

    CMyComPtr<ISequentialOutStream> outStream;
    const Int32 askMode = testMode
      ? NExtract::NAskMode::kTest
      : NExtract::NAskMode::kExtract;
    HRESULT hr = extractCallback->GetStream(index, &outStream, askMode);
    if (hr != S_OK)
      return hr;

    extractCallback->PrepareOperation(askMode);

    if (item.IsDir || item.Size == 0 || (!outStream && !testMode))
    {
      extractCallback->SetOperationResult(NExtract::NOperationResult::kOK);
      if (!item.IsDir)
      {
        completed += item.Size;
        extractCallback->SetCompleted(&completed);
      }
      continue;
    }

    Int32 opRes = NExtract::NOperationResult::kOK;
    UInt64 remaining = item.Size;
    UInt64 offset = 0;

    while (remaining > 0)
    {
      const UInt64 chunk = std::min<UInt64>(remaining, buffer.size());
      const UInt64 got = _reader->ReadFromFile(item.Handle, offset, chunk, buffer.data());
      if (got != chunk)
      {
        opRes = NExtract::NOperationResult::kDataError;
        break;
      }

      if (outStream)
      {
        hr = WriteStream(outStream, buffer.data(), (UInt32)got);
        if (hr != S_OK)
        {
          opRes = NExtract::NOperationResult::kDataError;
          break;
        }
      }

      offset += got;
      remaining -= got;
      completed += got;
      extractCallback->SetCompleted(&completed);
    }

    outStream.Release();
    extractCallback->SetOperationResult(opRes);
  }

  return S_OK;
}

// Footer magic lives at EOF, so start-of-file IsArc cannot confirm the format.
static UInt32 WINAPI IsArc_Zar(const Byte * /* p */, size_t size)
{
  if (size < 16)
    return k_IsArc_Res_NEED_MORE;
  return k_IsArc_Res_NO;
}

struct PackOutCtx
{
  ISequentialOutStream *Stream = NULL;
  HRESULT Hr = S_OK;
  bool Opened = false;
};

static void Pack_NewOutputFile(const int32_t /* partIndex */, void *ctx)
{
  PackOutCtx *c = (PackOutCtx *)ctx;
  c->Opened = true;
}

static void Pack_WriteOutputData(const void *data, size_t length, void *ctx)
{
  PackOutCtx *c = (PackOutCtx *)ctx;
  if (c->Hr != S_OK || !c->Stream)
    return;
  c->Hr = WriteStream(c->Stream, data, (UInt32)length);
}

static std::string ParentPathUtf8(const std::string &path)
{
  const size_t pos = path.find_last_of("/\\");
  if (pos == std::string::npos)
    return std::string();
  return path.substr(0, pos);
}

Z7_COM7F_IMF(CHandler::GetFileTimeType(UInt32 *type))
{
  *type = NFileTimeType::kWindows;
  return S_OK;
}

struct AutoProp
{
  PROPVARIANT v;
  AutoProp() { memset(&v, 0, sizeof(v)); v.vt = VT_EMPTY; }
  ~AutoProp() { ::VariantClear((VARIANTARG *)&v); }
  PROPVARIANT *ptr() { return &v; }
};

Z7_COM7F_IMF(CHandler::UpdateItems(ISequentialOutStream *outStream,
    UInt32 numItems, IArchiveUpdateCallback *updateCallback))
{
  if (!outStream || !updateCallback)
    return E_INVALIDARG;

  // Creating a new archive only (no modify-in-place of existing .zar).
  if (_reader)
    return E_NOTIMPL;

  UInt64 totalSize = 0;
  for (UInt32 i = 0; i < numItems; i++)
  {
    Int32 newData = 0, newProps = 0;
    UInt32 indexInArchive = (UInt32)(Int32)-1;
    RINOK(updateCallback->GetUpdateItemInfo(i, &newData, &newProps, &indexInArchive))
    if (indexInArchive != (UInt32)(Int32)-1)
      return E_NOTIMPL;

    AutoProp prop;
    RINOK(updateCallback->GetProperty(i, kpidIsDir, prop.ptr()))
    const bool isDir = (prop.v.vt == VT_BOOL && prop.v.boolVal != VARIANT_FALSE);
    if (!isDir)
    {
      AutoProp sizeProp;
      RINOK(updateCallback->GetProperty(i, kpidSize, sizeProp.ptr()))
      if (sizeProp.v.vt == VT_UI8)
        totalSize += sizeProp.v.uhVal.QuadPart;
      else if (sizeProp.v.vt == VT_UI4)
        totalSize += sizeProp.v.ulVal;
    }
  }
  RINOK(updateCallback->SetTotal(totalSize))

  PackOutCtx packCtx;
  packCtx.Stream = outStream;
  ZArchiveWriter writer(Pack_NewOutputFile, Pack_WriteOutputData, &packCtx);
  if (!packCtx.Opened || packCtx.Hr != S_OK)
    return (packCtx.Hr != S_OK) ? packCtx.Hr : E_FAIL;

  std::vector<Byte> buffer(1024 * 1024);
  UInt64 completed = 0;

  for (UInt32 i = 0; i < numItems; i++)
  {
    Int32 newData = 0, newProps = 0;
    UInt32 indexInArchive = (UInt32)(Int32)-1;
    RINOK(updateCallback->GetUpdateItemInfo(i, &newData, &newProps, &indexInArchive))

    AutoProp pathProp;
    RINOK(updateCallback->GetProperty(i, kpidPath, pathProp.ptr()))
    if (pathProp.v.vt != VT_BSTR || !pathProp.v.bstrVal)
      return E_FAIL;

    std::string utf8Path = WideToUtf8(pathProp.v.bstrVal);
    for (size_t n = 0; n < utf8Path.size(); n++)
      if (utf8Path[n] == '\\')
        utf8Path[n] = '/';

    AutoProp dirProp;
    RINOK(updateCallback->GetProperty(i, kpidIsDir, dirProp.ptr()))
    const bool isDir = (dirProp.v.vt == VT_BOOL && dirProp.v.boolVal != VARIANT_FALSE);

    if (isDir)
    {
      if (!writer.MakeDir(utf8Path.c_str(), true))
        return E_FAIL;
      RINOK(updateCallback->SetOperationResult(NUpdate::NOperationResult::kOK))
      continue;
    }

    const std::string parent = ParentPathUtf8(utf8Path);
    if (!parent.empty() && !writer.MakeDir(parent.c_str(), true))
      return E_FAIL;

    if (!writer.StartNewFile(utf8Path.c_str()))
      return E_FAIL;

    CMyComPtr<ISequentialInStream> inStream;
    RINOK(updateCallback->GetStream(i, &inStream))

    if (!inStream)
    {
      RINOK(updateCallback->SetOperationResult(NUpdate::NOperationResult::kOK))
      continue;
    }

    for (;;)
    {
      UInt32 processed = 0;
      HRESULT hr = inStream->Read(buffer.data(), (UInt32)buffer.size(), &processed);
      if (hr != S_OK)
        return hr;
      if (processed == 0)
        break;
      writer.AppendData(buffer.data(), processed);
      if (packCtx.Hr != S_OK)
        return packCtx.Hr;
      completed += processed;
      RINOK(updateCallback->SetCompleted(&completed))
    }

    RINOK(updateCallback->SetOperationResult(NUpdate::NOperationResult::kOK))
  }

  writer.Finalize();
  if (packCtx.Hr != S_OK)
    return packCtx.Hr;
  return S_OK;
}

Func_IsArc GetIsArc_Zar() { return IsArc_Zar; }

IInArchive *CreateZarHandler() { return new CHandler(); }

}}

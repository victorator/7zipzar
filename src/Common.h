#ifndef ZAR_COMMON_H
#define ZAR_COMMON_H

#include "../sdk/CPP/Common/MyCom.h"
#include "../sdk/CPP/7zip/Archive/IArchive.h"

#include <string>
#include <vector>

inline HRESULT ReadStream(IInStream *stream, void *data, UInt32 size)
{
  Byte *buf = (Byte *)data;
  while (size > 0)
  {
    UInt32 processed = 0;
    HRESULT hr = stream->Read(buf, size, &processed);
    if (hr != S_OK) return hr;
    if (processed == 0) return E_FAIL;
    buf += processed;
    size -= processed;
  }
  return S_OK;
}

inline HRESULT SeekTo(IInStream *stream, UInt64 pos)
{
  return stream->Seek((Int64)pos, STREAM_SEEK_SET, NULL);
}

inline HRESULT WriteStream(ISequentialOutStream *stream, const void *data, UInt32 size)
{
  const Byte *buf = (const Byte *)data;
  while (size > 0)
  {
    UInt32 processed = 0;
    HRESULT hr = stream->Write(buf, size, &processed);
    if (hr != S_OK) return hr;
    if (processed == 0) return E_FAIL;
    buf += processed;
    size -= processed;
  }
  return S_OK;
}

inline void PropVarSet_WStr(PROPVARIANT *prop, const wchar_t *s)
{
  prop->vt = VT_BSTR;
  prop->bstrVal = ::SysAllocString(s);
}

inline std::wstring Utf8ToWide(const char *utf8, size_t len)
{
  std::wstring result;
  result.reserve(len);
  size_t i = 0;
  while (i < len)
  {
    unsigned char c = (unsigned char)utf8[i];
    wchar_t wc;
    if (c < 0x80)
    {
      wc = c;
      i += 1;
    }
    else if (c < 0xE0 && i + 1 < len)
    {
      wc = ((c & 0x1F) << 6) | (utf8[i + 1] & 0x3F);
      i += 2;
    }
    else if (c < 0xF0 && i + 2 < len)
    {
      wc = ((c & 0x0F) << 12) | ((utf8[i + 1] & 0x3F) << 6) | (utf8[i + 2] & 0x3F);
      i += 3;
    }
    else
    {
      wc = L'?';
      i += 1;
    }
    result += wc;
  }
  return result;
}

inline std::string WideToUtf8(const wchar_t *ws)
{
  if (!ws) return std::string();
  std::string out;
  for (const wchar_t *p = ws; *p; ++p)
  {
    const unsigned int c = (unsigned int)*p;
    if (c < 0x80)
      out.push_back((char)c);
    else if (c < 0x800)
    {
      out.push_back((char)(0xC0 | (c >> 6)));
      out.push_back((char)(0x80 | (c & 0x3F)));
    }
    else
    {
      out.push_back((char)(0xE0 | (c >> 12)));
      out.push_back((char)(0x80 | ((c >> 6) & 0x3F)));
      out.push_back((char)(0x80 | (c & 0x3F)));
    }
  }
  return out;
}

#endif


#include "StdAfx.h"

#define INITGUID
#include "../sdk/CPP/Common/MyGuidDef.h"

#include "../sdk/CPP/Common/MyCom.h"
#include "../sdk/CPP/7zip/Archive/IArchive.h"

namespace NArchive {
  namespace NZar {
    IInArchive *CreateZarHandler();
    Func_IsArc GetIsArc_Zar();
  }
}

const Byte k7z_PROPID_To_VARTYPE[kpid_NUM_DEFINED] =
{
  VT_EMPTY,     // kpidNoProperty
  VT_UI4,       // kpidMainSubfile
  VT_UI4,       // kpidHandlerItemIndex
  VT_BSTR,      // kpidPath
  VT_BSTR,      // kpidName
  VT_BSTR,      // kpidExtension
  VT_BOOL,      // kpidIsDir
  VT_UI8,       // kpidSize
  VT_UI8,       // kpidPackSize
  VT_UI4,       // kpidAttrib
  VT_FILETIME,  // kpidCTime
  VT_FILETIME,  // kpidATime
  VT_FILETIME,  // kpidMTime
  VT_BOOL,      // kpidSolid
  VT_BOOL,      // kpidCommented
  VT_BOOL,      // kpidEncrypted
  VT_BOOL,      // kpidSplitBefore
  VT_BOOL,      // kpidSplitAfter
  VT_UI4,       // kpidDictionarySize
  VT_UI4,       // kpidCRC
  VT_BSTR,      // kpidType
  VT_BOOL,      // kpidIsAnti
  VT_BSTR,      // kpidMethod
  VT_BSTR,      // kpidHostOS
  VT_BSTR,      // kpidFileSystem
  VT_BSTR,      // kpidUser
  VT_BSTR,      // kpidGroup
  VT_UI8,       // kpidBlock
  VT_BSTR,      // kpidComment
  VT_UI8,       // kpidPosition
  VT_BSTR,      // kpidPrefix
  VT_UI8,       // kpidNumSubDirs
  VT_UI8,       // kpidNumSubFiles
  VT_BSTR,      // kpidUnpackVer
  VT_UI4,       // kpidVolume
  VT_BOOL,      // kpidIsVolume
  VT_UI8,       // kpidOffset
  VT_UI8,       // kpidLinks
  VT_UI8,       // kpidNumBlocks
  VT_UI8,       // kpidNumVolumes
  VT_UI4,       // kpidTimeType
  VT_BOOL,      // kpidBit64
  VT_BOOL,      // kpidBigEndian
  VT_BSTR,      // kpidCpu
  VT_UI8,       // kpidPhySize
  VT_UI8,       // kpidHeadersSize
  VT_UI4,       // kpidChecksum
  VT_BSTR,      // kpidCharacts
  VT_UI8,       // kpidVa
  VT_BSTR,      // kpidId
  VT_BSTR,      // kpidShortName
  VT_BSTR,      // kpidCreatorApp
  VT_UI4,       // kpidSectorSize
  VT_UI4,       // kpidPosixAttrib
  VT_BSTR,      // kpidSymLink
  VT_BSTR,      // kpidError
  VT_UI8,       // kpidTotalSize
  VT_UI8,       // kpidFreeSpace
  VT_UI8,       // kpidClusterSize
  VT_BSTR,      // kpidVolumeName
  VT_BSTR,      // kpidLocalName
  VT_BSTR,      // kpidProvider
  VT_BSTR,      // kpidNtSecure
  VT_BOOL,      // kpidIsAltStream
  VT_BOOL,      // kpidIsAux
  VT_BOOL,      // kpidIsDeleted
  VT_BOOL,      // kpidIsTree
  VT_BSTR,      // kpidSha1
  VT_BSTR,      // kpidSha256
  VT_BSTR,      // kpidErrorType
  VT_UI8,       // kpidNumErrors
  VT_UI4,       // kpidErrorFlags
  VT_UI4,       // kpidWarningFlags
  VT_BSTR,      // kpidWarning
  VT_UI8,       // kpidNumStreams
  VT_UI8,       // kpidNumAltStreams
  VT_UI8,       // kpidAltStreamsSize
  VT_UI8,       // kpidVirtualSize
  VT_UI8,       // kpidUnpackSize
  VT_UI8,       // kpidTotalPhySize
  VT_UI8,       // kpidVolumeIndex
  VT_BSTR,      // kpidSubType
  VT_BSTR,      // kpidShortComment
  VT_BSTR,      // kpidCodePage
  VT_BOOL,      // kpidIsNotArcType
  VT_BOOL,      // kpidPhySizeCantBeDetected
  VT_BOOL,      // kpidZerosTailIsAllowed
  VT_UI8,       // kpidTailSize
  VT_UI8,       // kpidEmbeddedStubSize
  VT_BSTR,      // kpidNtReparse
  VT_BSTR,      // kpidHardLink
  VT_UI8,       // kpidINode
  VT_UI8,       // kpidStreamId
  VT_BOOL,      // kpidReadOnly
  VT_BSTR,      // kpidOutName
  VT_BSTR,      // kpidCopyLink
  VT_BSTR,      // kpidArcFileName
  VT_BOOL,      // kpidIsHash
  VT_FILETIME,  // kpidChangeTime
  VT_UI4,       // kpidUserId
  VT_UI4,       // kpidGroupId
  VT_UI4,       // kpidDeviceMajor
  VT_UI4,       // kpidDeviceMinor
  VT_UI4,       // kpidDevMajor
  VT_UI4        // kpidDevMinor
};

BSTR AllocBstrFromAscii(const char *s)
{
  if (!s) return NULL;
  size_t len = strlen(s);
  wchar_t *ws = new wchar_t[len + 1];
  for (size_t i = 0; i <= len; i++)
    ws[i] = (wchar_t)(unsigned char)s[i];
  BSTR bstr = ::SysAllocString(ws);
  delete[] ws;
  return bstr;
}

namespace NWindows { namespace NCOM {
  BSTR AllocBstrFromAscii(const char *s) throw() { return ::AllocBstrFromAscii(s); }
}}

// {23170F69-40C1-278A-1000-000110FA0000}
static const GUID CLSID_Base =
  { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00 } };

static const Byte kZarId = 0xFA;

struct CFormatInfo
{
  const char *Name;
  const char *Ext;
  Byte Id;
  Func_IsArc IsArc;
  IInArchive *(*CreateHandler)();
};

static CFormatInfo g_Format =
{
  "ZArchive", "zar wua", kZarId,
  NArchive::NZar::GetIsArc_Zar(), NArchive::NZar::CreateZarHandler
};

extern "C" {

BOOL WINAPI DllMain(
#ifdef _WIN32
  HINSTANCE /* hInstance */,
#else
  void * /* hInstance */,
#endif
  DWORD /* dwReason */, LPVOID /* lpReserved */)
{
  return TRUE;
}

STDAPI CreateObject(const GUID *clsid, const GUID *iid, void **outObject)
{
  *outObject = NULL;

  GUID expected = CLSID_Base;
  expected.Data4[5] = g_Format.Id;
  if (*clsid != expected)
    return CLASS_E_CLASSNOTAVAILABLE;

  if (*iid != IID_IInArchive && *iid != IID_IOutArchive)
    return E_NOINTERFACE;

  // CMyUnknownImp starts at refcount 0; AddRef before QI, Release after.
  IInArchive *handler = g_Format.CreateHandler();
  if (!handler)
    return E_OUTOFMEMORY;
  handler->AddRef();
  const HRESULT hr = handler->QueryInterface(*iid, outObject);
  handler->Release();
  return hr;
}

STDAPI GetNumberOfFormats(UInt32 *numFormats)
{
  *numFormats = 1;
  return S_OK;
}

STDAPI GetHandlerProperty2(UInt32 formatIndex, PROPID propID, PROPVARIANT *value)
{
  if (formatIndex != 0)
    return E_INVALIDARG;

  value->vt = VT_EMPTY;
  value->wReserved1 = 0;
  value->wReserved2 = 0;
  value->wReserved3 = 0;
  value->uhVal.QuadPart = 0;

  switch (propID)
  {
    case NArchive::NHandlerPropID::kName:
      value->bstrVal = AllocBstrFromAscii(g_Format.Name);
      if (value->bstrVal) value->vt = VT_BSTR;
      break;

    case NArchive::NHandlerPropID::kClassID:
    {
      GUID clsId = CLSID_Base;
      clsId.Data4[5] = g_Format.Id;
      value->bstrVal = ::SysAllocStringByteLen((const char *)&clsId, sizeof(clsId));
      if (value->bstrVal) value->vt = VT_BSTR;
      break;
    }

    case NArchive::NHandlerPropID::kExtension:
      value->bstrVal = AllocBstrFromAscii(g_Format.Ext);
      if (value->bstrVal) value->vt = VT_BSTR;
      break;

    case NArchive::NHandlerPropID::kUpdate:
      value->vt = VT_BOOL;
      value->boolVal = VARIANT_TRUE;
      break;

    case NArchive::NHandlerPropID::kFlags:
      value->vt = VT_UI4;
      // Footer magic only — open by extension, validate in Open().
      value->ulVal = NArcInfoFlags::kByExtOnlyOpen;
      break;
  }

  return S_OK;
}

STDAPI GetIsArc(UInt32 formatIndex, Func_IsArc *isArc)
{
  *isArc = NULL;
  if (formatIndex != 0)
    return E_INVALIDARG;
  *isArc = g_Format.IsArc;
  return S_OK;
}

STDAPI SetLargePageMode() { return S_OK; }
STDAPI SetCaseSensitive(Int32 /* caseSensitive */) { return S_OK; }
STDAPI SetCodecs(IUnknown * /* compressCodecsInfo */) { return S_OK; }

}

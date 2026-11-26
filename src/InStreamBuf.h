#ifndef IN_STREAM_BUF_H
#define IN_STREAM_BUF_H

#include "Common.h"

#include <istream>
#include <streambuf>
#include <vector>

// Adapts 7-Zip IInStream to std::istream for ZArchiveReader::OpenFromStream.
class InStreamBuf : public std::streambuf
{
public:
  explicit InStreamBuf(IInStream *stream)
    : _stream(stream)
  {
    _buffer.resize(64 * 1024);
    setg(_buffer.data(), _buffer.data(), _buffer.data());

    UInt64 cur = 0;
    if (_stream->Seek(0, STREAM_SEEK_CUR, &cur) != S_OK)
      _size = 0;
    else if (_stream->Seek(0, STREAM_SEEK_END, &_size) != S_OK)
      _size = 0;
    else
      _stream->Seek((Int64)cur, STREAM_SEEK_SET, NULL);

    _pos = cur;
  }

  UInt64 Size() const { return _size; }

protected:
  int_type underflow() override
  {
    if (gptr() < egptr())
      return traits_type::to_int_type(*gptr());

    if (_pos >= _size)
      return traits_type::eof();

    if (SeekTo(_stream, _pos) != S_OK)
      return traits_type::eof();

    const UInt32 toRead = (UInt32)std::min<UInt64>(_buffer.size(), _size - _pos);
    UInt32 processed = 0;
    if (_stream->Read((Byte *)_buffer.data(), toRead, &processed) != S_OK || processed == 0)
      return traits_type::eof();

    _pos += processed;
    setg(_buffer.data(), _buffer.data(), _buffer.data() + processed);
    return traits_type::to_int_type(*gptr());
  }

  pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override
  {
    if (!(which & std::ios_base::in))
      return pos_type(off_type(-1));

    UInt64 newPos = _pos - (egptr() - gptr());
    if (dir == std::ios_base::beg)
      newPos = (UInt64)off;
    else if (dir == std::ios_base::cur)
      newPos = (UInt64)((Int64)newPos + off);
    else if (dir == std::ios_base::end)
      newPos = (UInt64)((Int64)_size + off);
    else
      return pos_type(off_type(-1));

    if (newPos > _size)
      return pos_type(off_type(-1));

    _pos = newPos;
    setg(_buffer.data(), _buffer.data(), _buffer.data());
    return pos_type((off_type)_pos);
  }

  pos_type seekpos(pos_type pos, std::ios_base::openmode which) override
  {
    return seekoff(off_type(pos), std::ios_base::beg, which);
  }

private:
  CMyComPtr<IInStream> _stream;
  std::vector<char> _buffer;
  UInt64 _pos = 0;
  UInt64 _size = 0;
};

class InStreamIStream : public std::istream
{
public:
  explicit InStreamIStream(IInStream *stream)
    : std::istream(&_buf)
    , _buf(stream)
  {
  }

  UInt64 Size() const { return _buf.Size(); }

private:
  InStreamBuf _buf;
};

#endif

#undef _GLIBCXX_USE_FLOAT128
#include <crisp/comms/Buffer.hh>


namespace crisp
{
  namespace comms
  {
    EncodeBuffer::~EncodeBuffer()
    {}

    StreamEncodeBuffer::StreamEncodeBuffer(std::streambuf* buf)
      : stream ( buf )
    {}

    StreamEncodeBuffer::~StreamEncodeBuffer()
    {}


    EncodeResult
    StreamEncodeBuffer::write(const void* buf, size_t size)
    { stream.write(reinterpret_cast<const char*>(buf), size);
      return EncodeResult::SUCCESS; }


    MemoryEncodeBuffer::MemoryEncodeBuffer(size_t size)
      : buffer ( new Buffer(size) ),
	data ( buffer->data ),
	length ( buffer->length ),
	offset ( 0 )
    {}

    MemoryEncodeBuffer::MemoryEncodeBuffer(Buffer* b)
      : buffer ( b ),
	data ( b->data ),
	length ( b->length ),
	offset ( 0 )
    {}

    MemoryEncodeBuffer::MemoryEncodeBuffer(char*& use_data, const size_t& use_length)
      : buffer ( nullptr ),
	data ( use_data ),
	length ( use_length ),
	offset ( 0 )
    {}

    MemoryEncodeBuffer::~MemoryEncodeBuffer()
    {}

    EncodeResult
    MemoryEncodeBuffer::write(const void* buf, size_t size)
    {
      if ( length - offset < size)
	return EncodeResult::INSUFFICIENT_SPACE;
      memcpy(data + offset, buf, size);
      offset += size;
      return EncodeResult::SUCCESS;
    }


  }

}

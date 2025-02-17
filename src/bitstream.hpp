#include "bytestream.hpp"

#ifdef MSFL_DLL
#ifdef MSFL_EXPORTS
#define MSFL_EXP __declspec(dllexport)
#else
#define MSFL_EXP __declspec(dllimport)
#endif
#else
#define MSFL_EXP
#endif

#ifdef MSFL_DLL
#ifdef __cplusplus
extern "C" {
#endif
#endif

class BitStream : public ByteStream {
private:
    size_t subPos = 0;
protected:
    using ByteStream::skip;
public:
    MSFL_EXP void writeBytes(byte* dat, size_t sz) override;
	MSFL_EXP void writeInt(i64 val, size_t nBytes = 4) override;
	MSFL_EXP void writeUInt(u64 val, size_t nBytes = 4) override;
	MSFL_EXP void writeByte(byte b) override;

    MSFL_EXP byte* readBytes(size_t sz) override;
	MSFL_EXP i64 readInt(size_t nBytes) override;
	MSFL_EXP u64 readUInt(size_t nBytes) override;
	MSFL_EXP byte readByte() override;

    MSFL_EXP void skipBytes(size_t nBytes);
    MSFL_EXP void skipBits(size_t nBits);
}

#ifdef MSFL_DLL
#ifdef __cplusplus
}
#endif
#endif
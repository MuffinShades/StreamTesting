#include "bytestream.hpp"

/**
 *
 * Fast BitStream for Oxygen
 *
 * Copyright James Weigand 2025 All rights reserved
 *
 */

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
		MSFL_EXP void __f_writeBits(u64 val, size_t nBits);
		MSFL_EXP inline void adv_extra_check();
	protected:
		using ByteStream::skip;
		MSFL_EXP void pos_adv() override;
		MSFL_EXP inline void bit_adv(size_t nBits);
		MSFL_EXP inline void byte_adv(size_t nBytes);
		MSFL_EXP inline void byte_adv_nc(size_t nBytes);
		MSFL_EXP inline void block_adv_check();
	public:
		BitStream() : ByteStream() {};
		BitStream(size_t sz) : ByteStream(sz) {};
		BitStream(byte* dat, size_t sz) : ByteStream(dat, sz) {};
		MSFL_EXP inline void writeBytes(byte* dat, size_t sz) override;
		MSFL_EXP inline void writeInt(i64 val, size_t nBytes = 8) override;
		MSFL_EXP inline void writeUInt(u64 val, size_t nBytes = 8) override;
		MSFL_EXP inline void writeByte(byte b) override;

		MSFL_EXP inline void writeBit(bit b);
		MSFL_EXP inline void writeBits(u64 val, size_t nBits);

		MSFL_EXP inline byte* readBytes(size_t sz) override;
		MSFL_EXP inline i64 readInt(size_t nBytes) override;
		MSFL_EXP inline u64 readUInt(size_t nBytes) override;
		MSFL_EXP inline byte readByte() override;

		MSFL_EXP inline bit readBit();
		MSFL_EXP inline u64 readBits(size_t nBits);
		MSFL_EXP inline u64 nextBits(size_t nBits);

		MSFL_EXP void skipBytes(size_t nBytes);
		MSFL_EXP void skipBits(size_t nBits);

		MSFL_EXP void bitSeek(size_t bit);
		void __int_dbg();
	};

#ifdef MSFL_DLL
#ifdef __cplusplus
}
#endif
#endif
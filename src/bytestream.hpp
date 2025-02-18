#pragma once
#include "types.hpp"

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

constexpr size_t MAX_BLOCK_SIZE = 0xffffffff, MAX_BLOCK_SIZE_LOG16 = 8;
constexpr size_t MIN_BLOCK_SIZE = 16; //TODO: actually enforce this constant

enum ByteStream_Mode {
	ByteStream_BigEndian = 0,
	ByteStream_LittleEndian = 1
};

struct mem_block {
	size_t sz, pos;
	mem_block* next, *prev;
	byte* dat;
	bool data_own = true; //not used
};

#define BYTESTREAM_ALIGNED_16

class ByteStream {
private:
	bool sz_aligned = false;
protected:
	size_t blockAllocSz = 0xffff, 
		   blockAllocSzLog16 = 4, 
		   blockAllocSzLog2 = 16,
	#ifdef COMPILER_MODE_64_BIT
		   blockAllocSzAlignment = 5;
	#else
		   blockAllocSzAlignment = 4;
	#endif	
	mem_block* head_block = nullptr, *cur_block = nullptr, *tail_block = nullptr;
	size_t allocSz = 0, len = 0, pos = 0, nBlocks = 0, blockPos = 0;
	byte* cur = nullptr;
	mem_block* alloc_new_block(size_t blockSz);
	void block_append(mem_block* block);
	void add_new_block(const size_t blockSz);
	void add_new_block(byte* dat, const size_t blockSz);
	void set_stream_data(byte *dat, size_t sz);
	bool block_adv(bool pos_adv = false);
	virtual void pos_adv();
	virtual void pos_adv(const size_t sz);
	void block_end();
	void set_cur_block(mem_block *block);
	virtual void len_inc();
	virtual void len_inc(const size_t sz);
	mem_block *get_t_block(size_t pos);
	u32 mod_block_sz(const u64 val);
public:
	ByteStream_Mode int_mode = ByteStream_LittleEndian;

	MSFL_EXP ByteStream(byte* dat, size_t len);
	MSFL_EXP ByteStream(size_t allocSz);
	MSFL_EXP ByteStream();

	//write functions
	MSFL_EXP virtual void writeBytes(byte* dat, size_t sz);
	MSFL_EXP virtual void writeInt(i64 val, size_t nBytes = 4);
	MSFL_EXP virtual void writeUInt(u64 val, size_t nBytes = 4);
	MSFL_EXP virtual void writeByte(byte b);
	MSFL_EXP virtual void writeInt16(i16 val);
	MSFL_EXP virtual void writeUInt16(u16 val);
	MSFL_EXP virtual void writeInt24(i24 val);
	MSFL_EXP virtual void writeUInt24(u24 val);
	MSFL_EXP virtual void writeInt32(i32 val);
	MSFL_EXP virtual void writeUInt32(u32 val);
	MSFL_EXP virtual void writeInt48(i48 val);
	MSFL_EXP virtual void writeUInt48(u48 val);
	MSFL_EXP virtual void writeInt64(i64 val);
	MSFL_EXP virtual void writeUInt64(u64 val);
	MSFL_EXP virtual void multiWrite(u64 val, size_t valSz, size_t nCopy);

	//read functions
	MSFL_EXP virtual byte* readBytes(size_t sz);
	MSFL_EXP virtual i64 readInt(size_t nBytes);
	MSFL_EXP virtual u64 readUInt(size_t nBytes);
	MSFL_EXP virtual byte readByte();
	MSFL_EXP virtual i16 readInt16();
	MSFL_EXP virtual u16 readUInt16();
	MSFL_EXP virtual i24 readInt24();
	MSFL_EXP virtual u24 readUInt24();
	MSFL_EXP virtual i32 readInt32();
	MSFL_EXP virtual u32 readUInt32();
	MSFL_EXP virtual i48 readInt48();
	MSFL_EXP virtual u48 readUInt48();
	MSFL_EXP virtual i64 readInt64();
	MSFL_EXP virtual u64 readUInt64();

	//helper functions
	MSFL_EXP void copyTo(byte* buffer, size_t copyStart, size_t copySz);
	MSFL_EXP void pack(); //packs all data into 1 chunk
	MSFL_EXP byte* getBytePtr(); //calls pack then returns pointer to base block
	MSFL_EXP size_t seek(size_t pos);
	MSFL_EXP size_t size();
	MSFL_EXP size_t tell();
	MSFL_EXP void clear();
	MSFL_EXP void free();
	MSFL_EXP size_t home();
	MSFL_EXP size_t end();
	MSFL_EXP void resize(size_t sz);
	MSFL_EXP void clip();
	MSFL_EXP void skip(size_t nBytes);
	MSFL_EXP void setMode(ByteStream_Mode mode);
	MSFL_EXP std::string readStr(size_t len);
	MSFL_EXP ByteStream* extract_substream();
	MSFL_EXP void __printDebugInfo();

	//operators
	MSFL_EXP byte operator[](size_t i);

	//settings
#ifdef BYTESTREAM_ALIGNED_16
	MSFL_EXP void setBlockAllocSz(const size_t log16Sz);
#else
	MSFL_EXP void setBlockAllocSz(const size_t sz);
#endif
};

#ifdef MSFL_DLL
#ifdef __cplusplus
}
#endif
#endif

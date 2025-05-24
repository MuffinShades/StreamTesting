#include "bytestream.hpp"
#include "msutil.hpp"
#include "memcpy.hpp"
#include <cassert>

/*

Fully Tested functions:

WriteByte
ReadByte
WriteInt
WriteUInt
All write ints / uints
pack
seek
len_inc
pos_inc

*/

//NOTES:
//
// so for writeInt I allocate 1 extra byte per chunk so it can use that overflow space
// to perform the memcpy...
// maybe figure out how to fix this but idk how
//
//

constexpr size_t ALIGN_COMPUTE_THRESH = 0xff;

void free_block(mem_block* block) {
	if (!block) return;

	block->next = block->prev = nullptr;
	if (block->dat)
		_safe_free_a(block->dat);
	block->dat = nullptr;
	block->sz = 0;
	delete block;
}

void ByteStream::free() {

	//free memory
	if (this->head_block) {
		mem_block* c_block = this->head_block;

		while (c_block) {
			mem_block* next = c_block->next;
			free_block(c_block);
			c_block = next;
		}
	}

	//reset everything else
	this->pos = this->allocSz = this->len = 0;
	this->cur_block = this->head_block = this->tail_block = nullptr;
	this->blockPos = 0;
	this->cur = nullptr;
}

void ByteStream::set_cur_block(mem_block *block) {
	if (!block) return;
	this->cur_block = block;
	this->cur = this->cur_block->dat;
}

void ByteStream::block_append(mem_block* block) {
	if (!block) return;

	this->allocSz += block->sz;

	if (!this->head_block) {
		this->head_block = this->tail_block = block;
		block->pos = 0;
		this->set_cur_block(block);
		return;
	}

	//make blocks point to eachother and append
	if (this->tail_block) {
		this->tail_block->next = block;
		block->prev = this->tail_block;
	}
	else {
		std::cout << "Warning: block order is messed up!!" << std::endl;
		*((int*)(0x00000000)) = 0; //crash program :3
	}

	if (block->prev)
		block->pos = block->prev->pos + block->prev->sz;
	else {
		std::cout << "Warning: block order is messed up!!" << std::endl;
		*((int*)(0x00000000)) = 0; //crash program again :3
	}
	this->tail_block = block;
}

mem_block* ByteStream::alloc_new_block(size_t sz) {
#ifdef BYTESTREAM_ALIGNED_16
	//aligned size to 0xf if needed
	//sz = 1 << min(((fast_log16(sz)+1) << 2), MAX_BLOCK_SIZE_LOG16 << 2);
#endif
	assert(sz > 0);

	mem_block* blck = new mem_block{
		.sz = sz
	};

	blck->dat = new byte[sz + 1]; //alloc Extra byte since well for some odd reason writeInt needs it :shrug:

	if (!blck->dat) {
		blck->dat = nullptr; //safety thing
		_safe_free_b(blck);
		return nullptr;
	}

	blck->dat_end = blck->dat + blck->sz;
	ZeroMem(blck->dat, blck->sz);

	return blck;
}

void ByteStream::add_new_block(const size_t sz) {
	mem_block* block = this->alloc_new_block(sz);

	if (!block) return;

	this->block_append(block);
}

//returns mod block size, useful for getting sub-block position
u32 ByteStream::mod_block_sz(const u64 val) {
#ifdef BYTESTREAM_ALIGNED_16
	return val - ((val >> blockAllocSzLog2) << blockAllocSzLog2);
#else
	u64 b = val;
	do {}  while ((b -= this->allocSz) > this->allocSz);
	return (u32) b;
#endif
}

//jumps to last block from cur block
void ByteStream::block_end() {
	//while (this->cur_block->next)
	//	this->cur_block = this->cur_block->next;
	this->cur_block = this->tail_block;
	this->cur = this->cur_block->dat;
}

void ByteStream::set_stream_data(byte* dat, const size_t sz) {
	if (!dat || sz <= 0) return;
	this->free();

	size_t bytesLeft = sz, blck_sz = 0;

	do {
		mem_block* block = this->alloc_new_block(sz);

		if (!block) return;

		in_memcpy(block->dat, dat, blck_sz = sz);

		this->block_append(block);
	} while ((i64)(bytesLeft -= blck_sz) > 0);

	this->len = sz;
	this->blockPos = 0;

	//this->len = sz; //set length to allocation size since were appending a full block of data
	//this->pos = this->len - 1; //set pos to last byte we read
	//this->block_end(); //jump to last block
	//this->blockPos = this->mod_block_sz(this->pos); //compute block pos
}

void ByteStream::add_new_block(byte *dat, size_t sz) {
	if (!dat || sz <= 0) return;


}

/**
 * 
 * ByteStream::block_adv
 * 
 * Returns bool that indicates if advancing was
 * successful
 * 
 */
bool ByteStream::block_adv(bool pos_adv) {
	if (this->cur_block && this->cur_block->next) {
		this->cur_block = this->cur_block->next;
		this->blockPos = 0;
		this->cur = this->cur_block->dat;
		if (pos_adv)
			this->pos = this->cur_block->pos;
		return true;
	}

	return false;
}

void ByteStream::pos_adv() {
	if (++this->pos >= this->len) {
		this->pos = this->len - 1;
		return;
	}
	if (++this->blockPos >= this->cur_block->sz)
		this->block_adv();
	else
		this->cur++;
}

void ByteStream::len_inc() {
	this->len++;
	if (this->len >= this->allocSz)
		this->add_new_block(this->blockAllocSz);
}

void ByteStream::len_inc(const size_t sz) {
	this->len += sz;
	while (this->len > this->allocSz) {
		std::cout << "adding blok" << std::endl;
		this->add_new_block(this->blockAllocSz);
	}
}

//TODO: fix position change after increasing by sz
void ByteStream::pos_adv(const size_t sz) {
	if ((this->pos += sz) >= this->len) {
		this->pos = this->len - 1;
		return;
	}

	//
	size_t bytesLeft = sz;
	while ((this->blockPos += bytesLeft) >= this->cur_block->sz) {
		bytesLeft -= this->cur_block->sz;
		this->block_adv();
		this->blockPos = 0;
		this->cur = this->cur_block->dat;
	}

	this->cur += bytesLeft;
}


//TODO: this function
void ByteStream::writeBytes(byte *dat, size_t sz) {
	//if (!dat || sz <= 0)
		//return;
	assert(dat && sz > 0);
	this->end();
	this->len_inc(sz);
	size_t blockBytesLeft;
	size_t rCopy = min(sz, (blockBytesLeft = (this->cur_block->sz - this->blockPos)));

	in_memcpy(this->cur_block->dat, dat, rCopy);
	sz -= rCopy;

	byte *dp = dat + rCopy;

	//TODO: when writing blocks try computing the alignment for a block and use a
	//manual version of dy_memcpy to optimally copy over for a block of data
	//still have an if between dynamic copy and just normal memcpy since for
	//small byte writes memcpy is likely faster
	//
	//lol lost you thought dy_memcpy was fast
	//in_memcpy just better

	while (sz > this->blockAllocSz) {
		in_memcpy(this->cur_block->dat, dp, sz);
		sz -= this->blockAllocSz;
		dp += this->blockAllocSz;
	}

	this->block_adv();

	if (sz > 0) {
		in_memcpy(this->cur, dp, sz);
		this->cur += sz;
	}
}

//possible fix: if this function doesn't write to end of block then well whoops :3
void ByteStream::writeByte(byte b) {
	if (!this->cur) return;

	//increase length then block since len allocates and block just advances the current block
	this->pos = this->len;
	this->len_inc();

	while (++this->blockPos > this->cur_block->sz)
		if (!this->block_adv()) {
			this->add_new_block(this->blockAllocSz);
			this->block_adv();
		}

	//std::cout << "Write: " << (this->cur - this->cur_block->dat) << std::endl;
	if (this->cur && this->cur < this->cur_block->dat_end)
		*this->cur++ = b;
	else {
		if (this->cur >= this->cur_block->dat_end)
			std::cout << "Byte Write error, out of bounds! " << this->blockPos << " " << this->cur_block->sz << std::endl;
	}
}

void ByteStream::setMode(IntFormat mode) {
	this->int_mode = (IntFormat)(mode & 1);
}

void ByteStream::writeInt(i64 val, size_t nBytes) {
	//numeric clease :3
	if (nBytes <= 0) return;
	if (nBytes > 8) nBytes = 8;
	if (this->int_mode != __getOSEndian()) val = endian_swap(val, nBytes);
	this->len_inc(nBytes);

	//block overflow stuff
	if ((this->blockPos += nBytes) >= this->cur_block->sz) {
		const size_t left = this->cur_block->sz - (this->blockPos - nBytes);
		in_minicpy256(this->cur, &val, left);

		//block adv
		if (!this->block_adv()) {
			if (!this->cur_block)
				return;
			else {
				this->add_new_block(this->blockAllocSz);
				this->block_adv();
			}
		}

		//adjust int yk
		val >>= (left << 3);
		this->blockPos = (nBytes -= left);
		this->blockPos = nBytes;
	}

	//right hand copy / base copy
	in_minicpy256(this->cur, &val, nBytes);

	this->cur += nBytes;
	this->pos += nBytes;
}

void ByteStream::writeUInt(u64 val, size_t nBytes) {
	//numeric clease :3
	if (nBytes <= 0) return;
	if (nBytes > 8) nBytes = 8;
	if (this->int_mode != __getOSEndian()) val = endian_swap(val, nBytes);
	this->len_inc(nBytes);

	//block overflow stuff
	if ((this->blockPos += nBytes) >= this->cur_block->sz) {
		const size_t left = this->cur_block->sz - (this->blockPos - nBytes);
		in_minicpy256(this->cur, &val, left);

		//block adv
		if (!this->block_adv()) {
			if (!this->cur_block)
				return;
			else {
				this->add_new_block(this->blockAllocSz);
				this->block_adv();
			}
		}

		//adjust int yk
		val >>= (left << 3);
		this->blockPos = (nBytes -= left);
		this->blockPos = nBytes;
	}

	//right hand copy / base copy
	in_minicpy256(this->cur, &val, nBytes);

	this->cur += nBytes;
	this->pos += nBytes;
}

//int writes
void ByteStream::writeInt16(i16 val) {
	/*if (__getOSEndian() == this->int_mode) {
		*((i16*)this->cur) = val;
		this->skip(sizeof(i16));
	} else*/
		this->writeInt(val, sizeof(i16));
}

void ByteStream::writeUInt16(u16 val) {
	/*if (__getOSEndian() == this->int_mode) {
		*((u16*)this->cur) = val;
		this->skip(sizeof(u16));
	} else*/
		this->writeInt(val, sizeof(u16));
}

void ByteStream::writeInt24(i24 val) {
	this->writeInt(val, 3);
}

void ByteStream::writeUInt24(u24 val) {
	this->writeUInt(val, 3);
}

void ByteStream::writeInt32(i32 val) {
	/*if (__getOSEndian() == this->int_mode) {
		*((i32*)this->cur) = val;
		this->skip(sizeof(i32));
	} else*/
		this->writeInt(val, sizeof(i32));
}

void ByteStream::writeUInt32(u32 val) {
	/*if (__getOSEndian() == this->int_mode) {
		*((u32*)this->cur) = val;
		this->skip(sizeof(u32));
	} else*/
		this->writeInt(val, sizeof(u32));
}

void ByteStream::writeInt48(i48 val) {
	this->writeInt(val, 5);
}

void ByteStream::writeUInt48(u48 val) {
	this->writeUInt(val, 5);
}

void ByteStream::writeInt64(i64 val) {
	/*if (__getOSEndian() == this->int_mode) {
		*((i64*)this->cur) = val;
		this->skip(sizeof(i64));
	} else*/
		this->writeInt(val, sizeof(i64));
}

void ByteStream::writeUInt64(u64 val) {
	/*if (__getOSEndian() == this->int_mode) {
		*((u64*)this->cur) = val;
		this->skip(sizeof(u64));
	} else*/
		this->writeInt(val, sizeof(u64));
}

//TODO: int reads and other reads along with multi write

void ByteStream::multiWrite(u64 val, size_t valSz, size_t nCopy) {
    while (nCopy--)
		this->writeUInt(val, valSz);
}

byte* ByteStream::readBytes(size_t sz) {
	const size_t o_sz = sz;
    byte *r = new byte[o_sz];
	ZeroMem(r, o_sz);

	//l copy
	const size_t blockBytesLeft = this->cur_block->sz - this->blockPos;
	const size_t l = min(sz, blockBytesLeft);
	in_memcpy(r, this->cur, l);
	this->pos_adv(l);
	sz -= l;

	//quick prep
	if (sz > 0)
		if (!this->block_adv(true))
			return r;
	else
		return r;

	//now copy full blocks
	byte *rw = r + l;
	while (sz >= this->cur_block->sz) {
		in_memcpy(rw, this->cur_block->dat, this->cur_block->sz);
		rw += this->cur_block->sz;
		sz -= this->cur_block->sz;
		if (!this->block_adv(true))
			return r;
	}

	//copy over final bytes
	in_memcpy(rw, this->cur_block->dat, sz);
	this->pos_adv(sz);

	return r;
}

byte ByteStream::readByte() {
	byte b = *this->cur;
	this->pos_adv();
    return b;
} 

i64 ByteStream::readInt(size_t nBytes) {
	i64 res = 0;

	if (nBytes <= 0) return 0;
	if (nBytes > 8) nBytes = 8;

	if (this->int_mode == IntFormat_BigEndian)
		while (nBytes--) {
			res <<= 8;
			res |= this->readByte();
		}
	else {
		size_t off = 0;
		while (nBytes--) {
			res |= this->readByte() << off;
			off += 8;
		}
	}

	return res;
}

u64 ByteStream::readUInt(size_t nBytes) {
	u64 res = 0;
    if (nBytes <= 0) return 0;
	if (nBytes > 8) nBytes = 8;

    if (this->int_mode == IntFormat_BigEndian)
		while (nBytes--) {
			res <<= 8;
			res |= this->readByte();
		}
	else {
		size_t off = 0;
		while (nBytes--) {
			res |= this->readByte() << off;
			off += 8;
		}
	}

	//if ((this->blockPos += nBytes) > this->cur_block->sz) {

	//}

	return res;
}

i16 ByteStream::ByteStream::readInt16() {
	/*if (__getOSEndian() == this->int_mode) {
		const i16 v = *((i16*)this->cur);
		this->skip(sizeof(i16));
		return v;
	} else*/
    	return this->readInt(sizeof(i16));
}

u16 ByteStream::readUInt16() {
    /*if (__getOSEndian() == this->int_mode) {
		const u16 v = *((u16*)this->cur);
		this->skip(sizeof(u16));
		return v;
	} else*/
    	return this->readUInt(sizeof(u16));
}

i24 ByteStream::readInt24() {
    return this->readInt(3);
}

u24 ByteStream::readUInt24() {
    return this->readUInt(3);
}

i32 ByteStream::readInt32() {
    /*if (__getOSEndian() == this->int_mode) {
		const i32 v = *((i32*)this->cur);
		this->skip(sizeof(i32));
		return v;
	} else*/
    	return this->readInt(sizeof(i32));
}

u32 ByteStream::readUInt32() {
    /*if (__getOSEndian() == this->int_mode) {
		const u32 v = *((u32*)this->cur);
		this->skip(sizeof(u32));
		return v;
	} else*/
    	return this->readUInt(sizeof(u32));
} 

i48 ByteStream::readInt48() {
    return this->readInt(5);
}

u48 ByteStream::readUInt48() {
    return this->readUInt(5);
}

i64 ByteStream::readInt64() {
    /*if (__getOSEndian() == this->int_mode) {
		const i64 v = *((i64*)this->cur);
		this->skip(sizeof(i64));
		return v;
	} else*/
    	return this->readInt(sizeof(i64));
}

u64 ByteStream::readUInt64() {

	//TODO: make not slow :3
	//use function table or maybe not...
    /*if (__getOSEndian() == this->int_mode) {
		const u64 v = *((u64*)this->cur);
		this->skip(sizeof(u64));
		return v;
	} else*/
    	return this->readUInt(sizeof(u64));
}

//util stuf

void ByteStream::copyTo(byte* buffer, size_t copyStart, size_t copySz) {
	if (!buffer || copySz <= 0 || this->len - copyStart < copySz || copyStart > this->len) return;
	this->pack();
	in_memcpy(buffer, this->cur_block->dat + copyStart, copySz);
}

void ByteStream::pack() {
	byte *dat = new byte[this->len];
	ZeroMem(dat, this->len);

	mem_block *c_block = this->head_block;

	byte *dc = dat, *end = dat + this->len;
	size_t tsz = 0;

	mem_block* l_block = nullptr;

	//*(this->cur_block->dat + this->cur_block->sz) = 1; //see if this causes issue

	while (c_block) {
		if ((tsz + c_block->sz) > this->len) {
			l_block = c_block;
			memcpy(dc, c_block->dat, this->len - tsz);
			break;
		}

		memcpy(dc, c_block->dat, c_block->sz);
		dc += c_block->sz;
		tsz += c_block->sz;

		mem_block* p_block = c_block;
		c_block = c_block->next;
		free_block(p_block);
	
		if (dc > end) {
			l_block = c_block;
			break;
		}
	}

	//
	if (l_block) {
		while (l_block) {
			mem_block* f = l_block->next;
			free_block(l_block);
			l_block = f;
		}
	}

	mem_block* h_block = new mem_block{
		.sz = this->len,
		.dat = dat
	};

	//adjust a bunch of stuff
	h_block->pos = 0;
	this->allocSz = this->len;
	this->blockPos = this->pos;
	if (this->pos >= this->len)
		this->pos = this->blockPos = this->len - 1;
	this->cur = h_block->dat;
	this->cur_block = this->head_block = this->tail_block = h_block;
}

byte* ByteStream::getBytePtr() {
	this->pack();
	return this->cur_block->dat;
}

mem_block *ByteStream::get_t_block(size_t pos) {
	mem_block *t = this->cur_block;
	
	if (t->pos > pos)
		while (t->pos > pos) {
			if (!t->prev) break;
			t = t->prev;
		}
	else if (t->pos + t->sz < pos)
		while (t->pos + t->sz < pos) {
			if (!t->next) break;
			t = t->next;
		}

	return t;
}

size_t ByteStream::seek(size_t pos) {
	const size_t pSave = this->pos;
	if (pos >= this->len && this->len > 0)
		return this->seek(this->len - 1);
	this->pos = pos;
	
	//mem block jump
	mem_block *t = this->get_t_block(this->pos);

	if (!t) {
		this->pos = pSave;
		std::cout << "Byte Stream Error, invalid pos!" << std::endl;
		return this->pos;
	}

	this->cur_block = t;
	this->cur = this->cur_block->dat;

	std::cout << "Block Move: " << this->pos << " | " << this->cur_block->pos << std::endl;

	//block pos calculations
	this->blockPos = this->pos - this->cur_block->pos;

	std::cout << "Bp: " << this->blockPos << std::endl;
}

size_t ByteStream::size() {
	return this->len;
}

size_t ByteStream::tell() {
	return this->pos;
}

void ByteStream::clear() {
	this->free();
	this->allocSz = 0;
	this->len = 0;
	this->cur = nullptr;
	this->cur_block = nullptr;
}

size_t ByteStream::home() {
	size_t sPos = this->pos;
	this->pos = this->blockPos = 0;
	this->cur_block = this->head_block;
	return sPos;
}

size_t ByteStream::end() {
	size_t sPos = this->pos;
	this->cur_block = this->tail_block;
	this->blockPos = this->tail_block->sz - 1;
	this->pos = this->len - 1;
	return sPos;
}

void ByteStream::resize(size_t sz) {
	mem_block *resizeBlock = this->head_block;

	while (resizeBlock->pos + resizeBlock->sz < sz) {
		if (!resizeBlock->next)
			return;
		resizeBlock = resizeBlock->next;
	}

	const size_t nBlockSz = sz - resizeBlock->pos;

	//free extra blocks
	bool rePos = false;
	mem_block *tf_block = resizeBlock->next;
	while (tf_block) {
		mem_block *s = tf_block;
		if (s == this->cur_block)
			rePos = true;
		tf_block = tf_block->next;
		free_block(s);
	}

	//construct new last block
	mem_block* f_block = new mem_block{
		.sz = nBlockSz,
		.dat = new byte[nBlockSz]
	};

	f_block->pos = resizeBlock->pos;

	ZeroMem(f_block->dat, f_block->sz);
	in_memcpy(f_block->dat, resizeBlock->dat, f_block->sz);
	mem_block *append_block = resizeBlock->prev;
	free_block(resizeBlock); //free final block

	if (!append_block) {
		this->cur_block = this->tail_block = this->head_block = f_block;
		this->pos = min(this->pos, f_block->sz - 1);
		this->blockPos = f_block->sz - 1;
		return;
	} else {
		append_block->next = f_block;
		f_block->prev = append_block;
		this->tail_block = f_block;
	}

	//reposition cur_block and pos if needed
	if (rePos) {
		this->cur_block = f_block;
		this->pos = this->cur_block->pos + this->cur_block->sz - 1;
		this->blockPos = this->cur_block->sz - 1;
	}
}

void ByteStream::clip() {
	this->resize(this->len);
}

void ByteStream::skip(size_t nBytes) {
	this->pos_adv(nBytes);
}

std::string ByteStream::readStr(size_t len) {
	char *c_str = reinterpret_cast<char*>(this->readBytes(len));
	return std::string(c_str);
}

byte ByteStream::operator[](size_t i) {
	const size_t hLen = this->len >> 1;

	mem_block *i_block;

	if (i > hLen) {
		i_block = this->tail_block;

		while (i_block && i_block->pos > i)
			i_block = i_block->prev;
	} else {
		i_block = this->head_block;

		while (i_block && i_block->pos > i)
			i_block = i_block->next;
	}

	if (!i_block) return 0;

	const size_t sub_block_pos = i - i_block->pos;
	return *(i_block->dat + sub_block_pos);
}

ByteStream::ByteStream(size_t allocSz) {
	if (this->head_block)
		this->free();
	this->allocSz = allocSz;
#ifdef STREAM_ALIGN_TO_BLOCK
	while (allocSz > this->blockAllocSz) {
		this->add_new_block(this->blockAllocSz);
		allocSz -= blockAllocSz;
	}

	this->add_new_block(allocSz);
#else
	this->add_new_block(allocSz);
#endif

	this->cur = this->head_block->dat;
}

ByteStream::ByteStream(byte* dat, size_t len) {
	if (!dat || len <= 0) return;

	if (this->head_block)
		this->free();

	//add first block and we done
	this->set_stream_data(dat, len);
	this->cur = this->head_block->dat;
}

ByteStream::ByteStream() {
	//do some ini stuff maybe
	this->cur = nullptr;
}

//block alloc size stuff
#ifdef BYTESTREAM_ALIGNED_16
void ByteStream::setBlockAllocSz(const size_t log16Sz) {
	const size_t sz = 1 << (log16Sz << 2);
	this->blockAllocSz = min(sz, MAX_BLOCK_SIZE);
	//this->blockAllocSzLog16 = log16Sz;
	//this->blockAllocSzLog2 = log16Sz << 2;
	this->blockAllocSzLog16 = fast_log16(sz);
	this->blockAllocSzLog2 = fast_log2(sz);
	this->blockAllocSzAlignment = computeMaxMod(sz);
	this->sz_aligned = true;
}
#else
void ByteStream::setBlockAllocSz(const size_t sz) {
	this->blockAllocSz = min(sz, MAX_BLOCK_SIZE);
	this->blockAllocSzLog16 = fast_log16(sz);
	this->blockAllocSzLog2  = fast_log2(sz);
	this->blockAllocSzAlignment = computeMaxMod(sz);
}
#endif

void ByteStream::__printDebugInfo() {
	std::cout << "-- Stream Info --" << std::endl;
	std::cout << "Size: " << this->len << "\n";
	std::cout << "Pos: " << this->pos << "\n";
	std::cout << "Allocation Size: " << this->allocSz << "\n";
	std::cout << "Chunk Size: " << this->blockAllocSz << "\n";

	size_t nChunks = 0;

	mem_block *c_chunk = this->head_block;

	while (c_chunk) {
		nChunks++;
		c_chunk = c_chunk->next;
	}

	std::cout << "N Chunks: " << nChunks << std::endl;
}
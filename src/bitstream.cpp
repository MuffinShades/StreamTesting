#include "bitstream.hpp"
#include "msutil.hpp"

void BitStream::pos_adv() {
	this->subPos = 0;
	this->ByteStream::pos_adv();
}

void BitStream::byte_adv_nc(size_t nBytes) {
	this->cur += nBytes;
	this->blockPos += nBytes;
	this->pos += nBytes;
}

void BitStream::block_adv_check() {
	size_t itr = 0;

	std::cout << "Blck Cmp: " << this->blockPos << " / " << this->cur_block->sz << std::endl;

	while (this->blockPos >= this->cur_block->sz) {
		const size_t pSave = this->pos;
		if (!this->block_adv(true)) {
			std::cout << "Block adv failed!" << std::endl;
		}
		const size_t pDiff = pSave - this->pos;

		std::cout << "Block adv attempt: " << pSave << " " << pDiff << std::endl;

		//re-adjust positions
		this->byte_adv_nc(pDiff);

		if (++itr >= 10) {
			std::cout << "BitStream error! Blocksize too small!" << std::endl;
			break;
		}
	}
}

void BitStream::bit_adv(size_t nBits) {
	const size_t nbRead = nBits >> 3, bOverflowRead = nBits & 7;

	this->subPos += bOverflowRead; //sub pos change

	const size_t jb = this->subPos >= 9;
	this->byte_adv_nc(jb + nbRead);
	this->block_adv_check();
}

void BitStream::byte_adv(size_t nBytes) {
	this->byte_adv_nc(nBytes);
	this->block_adv_check();
}

void BitStream::writeBit(bit b) {
	if (!this->cur) return;

	if (this->pos >= this->len)
		this->len_inc(this->pos - this->len + 1);

	*this->cur |= (b & 1) << this->subPos;

	std::cout << "Write: " << b << " | " << this->len << " | " << this->subPos << " | "  << (int)*this->cur << " | Ptr: " << (uintptr_t)this->cur << std::endl;

	if (++this->subPos >= 8) {
		this->pos = this->len;
		this->subPos = 0;
		this->len_inc();
		//this->pos_adv();
		this->cur++; //TODO: make this less dangerous (block checking)
	}
}

void BitStream::writeByte(byte b) {
	if (!this->cur) return;
	size_t lsz = (7 - this->subPos) + 1;
	*this->cur |= (b & MAKE_MASK(lsz)) >> this->subPos; //left hand side
	this->pos = this->len;
	this->len_inc();
	*this->cur = b & this->subPos;
}

//main bit copy routine
void BitStream::__f_writeBits(u64 val, size_t nBits) {
	if(nBits <= 0) return;

	//normal copy
	/*size_t rBits = (nBits - this->subPos) - 0x38;
	bool extra = !(rBits & 0x7fffffffffffffffULL);

	//main copy routine
	size_t lBits;
	const size_t lMask = MAKE_MASK(lBits = (56 - this->subPos));

	switch (__getOSEndian()) {
	case IntFormat_LittleEndian:
		*((u64*)this->cur) |= (endian_swap(val, (lBits & 56) + 1 * (~(lBits & 7) & 1)) & lMask);
		break;
	default:
		*((u64*)this->cur) |= (val & lMask) << this->subPos;
		break;
	}

	//extra after le copy thing
	if (extra) {
		val >>= 0x38;
		val &= MAKE_MASK(rBits);
		*this->cur |= val;
		this->subPos = rBits;
	}*/

	//rewritten version cause idk what that other version was ;-; 
	i64 overflow = 64 - this->subPos - nBits;
	bool extra = overflow < 0;

	//swap endian of val to ensure formated in BIG_ENDIAN
	if (__getOSEndian() == IntFormat_LittleEndian)
		endian_swap(val, 8);

	//Compute number of left hand side bits and make a mask
	//
	//   Sub Pos            Left Hand Copy
	// +---------+ +.......................... (...)+
	// 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 ...->64
	//
	const size_t nLBits = 63 - this->subPos;
	const size_t lMask = MAKE_MASK(nLBits) << 1 | 1;

	std::cout << "Bulk Write: " << val << " | L: " << nLBits << " | N Bytes: " << ((nBits & 120) >> 3) << " | PrCur: " << (*(u64*)this->cur) << " | RVal: " << ((val & lMask) << this->subPos) << " | Len: " << this->len << std::endl;
	
	//left copy
	//takes first nLBits and copies them into the current u64
	u64* c64 = (u64*) this->cur;
	(*c64) |= (val & lMask) << this->subPos;

	std::cout << "PoCur: " << (*(u64*)this->cur) << " | " << this->subPos << " | " << val << " | " << lMask << std::endl;

	val >>= nLBits;

	//inc
	const size_t nby = (nBits & 120) >> 3; //get n bytes taken

	//advance that many bytes
	this->cur += nby;
	this->blockPos += nby;

	//is there an extra byte that needs to be skipped?
	const size_t ex_inc = (const size_t) ((this->subPos += (nBits & 7)) >= 8);

	std::cout << "Ex Inc: " << ex_inc << " | SP: " << this->subPos << std::endl;

	this->subPos -= ex_inc << 3;
	this->cur += ex_inc;
	this->blockPos += ex_inc;
	this->len_inc(ex_inc);

	if (extra) {
		std::cout << "Ext!" << std::endl;
	}

	/*switch (__getOSEndian()) {
	case IntFormat_LittleEndian:
		*((u64*)this->cur) |= (endian_swap(val, );
		break;
	default:
		*((u64*)this->cur) |= (val & lMask) << this->subPos;
		break;
	}*/
}

//writes the bits of a val to the stream
void BitStream::writeBits(u64 val, size_t nBits) {
	if (nBits <= 0) return;
	const size_t msk = MAKE_MASK(nBits);
	size_t req = (nBits & 120) >> 3 + ((this->subPos + (nBits & 7)) >= 8);
	val &= msk;

	//std::cout << "REQ: " << req << std::endl;

	this->pos += req;
	this->len_inc(req);

	//split needed, do what is done with writeInt in ByteStream
	if (this->blockPos + req >= this->cur_block->sz) {
		std::cout << "Block thing.........." << std::endl;
		//left write then just like write whats left :D
		const size_t cbl = ((this->cur_block->sz - (this->blockPos + 1)) << 3) + (7 - this->subPos) + 1; //get num bits left
		this->__f_writeBits(val & MAKE_MASK(cbl), cbl);
		val >>= cbl;
		nBits -= cbl;
		this->block_adv();
		req = ((nBits & 120) >> 3) + 1;
	}

	this->__f_writeBits(val, nBits);
}

void BitStream::writeBytes(byte* dat, size_t sz) {
	if (!dat || sz <= 0) return;
}

void BitStream::writeInt(i64 val, size_t nBytes) {
	this->writeBits(val, nBytes << 3);
}

void BitStream::writeUInt(u64 val, size_t nBytes) {
	this->writeBits(val, nBytes << 3);
}

void BitStream::adv_extra_check() {
	const size_t jb = this->subPos >= 8;
	this->subPos -= jb << 3;
	this->byte_adv(jb);
}

//read functionality
bit BitStream::readBit() {
	bit b = (*this->cur >> this->subPos) & 1;
	this->subPos++;

	//jump thingy
	this->adv_extra_check();

	//return :O (no way!)
	return b;
}

u64 BitStream::readBits(size_t nBits) {
	std::cout << "Read Start: " << this->pos << ", " << this->subPos << std::endl;
	nBits &= 63;
	const size_t bMask = MAKE_MASK(nBits), adv = nBits;

	const size_t nLBits = 63 - this->subPos;
	const size_t lMask = MAKE_MASK(nLBits) << 1 | 1;

	std::cout << (uintptr_t) this->cur << std::endl;

	u64 c64 = (*((u64*)this->cur)) >> this->subPos;
	c64 &= lMask;

	u64 r = c64 & bMask;

	const size_t lAdv = min(nLBits, nBits); //TODO: maybe dont use min since it's slow
	this->bit_adv(lAdv);
	nBits -= lAdv;

	//TODO: read the right hand side :3
	
	return r;
}

byte* BitStream::readBytes(size_t sz) {
	return nullptr;
}

i64 BitStream::readInt(size_t nBytes) {
	return 0;
}

u64 BitStream::readUInt(size_t nBytes) {
	return 0;
}

byte BitStream::readByte() {
	return 0;
}

u64 BitStream::nextBits(size_t nBits) {
	return 0;
}

void BitStream::skipBytes(size_t nBytes) {

}

void BitStream::skipBits(size_t nBits) {
	const size_t nByteSkip = nBits >> 3, nBitSkip = nBits & 7;
}

void BitStream::bitSeek(size_t bit) {
	this->seek(bit >> 3);
	this->subPos = bit & 7;
}

void BitStream::__int_dbg() {
	for (int i = 0; i < 3; i++) {
		std::cout << (int)(*(this->cur_block->dat + i)) << " | " << (uintptr_t)this->cur_block->dat << ", " << (uintptr_t)(this->cur_block->dat+i) << ", " << (uintptr_t)(this->cur) << std::endl;
 	}
}
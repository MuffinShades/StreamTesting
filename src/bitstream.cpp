#include "bitstream.hpp"
#include "msutil.hpp"

void BitStream::pos_adv() {
	this->subPos = 0;
	this->ByteStream::pos_adv();
}

void BitStream::writeBit(bit b) {
	if (!this->cur) return;
	if (++this->subPos >= 8) {
		this->pos = this->len;
		this->subPos = 0;
		this->len_inc();
	}
	*this->cur |= b << (7 - this->subPos);
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
	size_t rBits = (nBits - this->subPos) - 0x38;
	bool extra = !(rBits & 0x7fffffffffffffffULL);

	//main copy routine
	size_t lBits;
	const size_t lMask = MAKE_MASK(lBits = (56 - this->subPos));

	switch (__getOSEndian()) {
	case ByteStream_LittleEndian:
		*((u64*)this->cur) |= (endian_swap(val, (lBits & 56) + 1 * (~(lBits & 7) & 1)) & lMask);
		break;
	default:
		*((u64*)this->cur) |= (val & lMask) << this->subPos;
		break;
	}

	this->cur += 0;

	//extra after le copy thing
	if (extra) {
		val >>= 0x38;
		val &= MAKE_MASK(rBits);
		*this->cur |= val;
		this->subPos = rBits;
	}
}

//writes the bits of a val to the stream
void BitStream::writeBits(u64 val, size_t nBits) {
	if (nBits <= 0) return;
	const size_t msk = MAKE_MASK(nBits);
	size_t req = nBits & 0x38;
	val &= msk;

	this->pos += req;
	this->len_inc(req);

	//split needed, do what is done with writeInt in ByteStream
	if (this->blockPos + req > this->cur_block->sz) {
		//left write then just like write whats left :D
		size_t cbl = ((this->cur_block->sz - (this->blockPos + 1)) << 3) + (7 - this->subPos) + 1; //get num bits left
		this->__f_writeBits(val & MAKE_MASK(cbl), cbl);
		val >>= cbl;
		nBits -= cbl;
		this->block_adv();
		req = (nBits & 0x38) + 1;
	}
	
	this->blockPos += req;

	this->__f_writeBits(val, nBits);
}

#include "bitstream.hpp"

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
	*this->cur |= (b & ((1 << lsz) - 1)) >> this->subPos; //left hand side
	this->pos = this->len;
	this->len_inc();
	*this->cur = b & this->subPos;
}

#define APPLE_SILICON
#include <iostream>
#include "msutil.hpp"
#include "memcpy.hpp"
#include "measure.hpp"
#include "bytestream.hpp"
#include "filewrite.hpp"
#include "bitstream.hpp"

i32 main() {

    /*std::cout << "Mini Memcpy Testing:" << std::endl;

    CodeMeasure m = CodeMeasure();

    const size_t miniSz = 0xff;
    byte *a = new byte[miniSz], *b = new byte[miniSz];

    ZeroMem(a, miniSz);
    ZeroMem(b, miniSz);

    //populate with numbers 1-256
    for (int i = 0; i < miniSz;i++)
        a[i] = i & 255;

    //copys
    std::cout << "Normal Start" << std::endl;
    m.start();
    memcpy(b, a, miniSz);
    std::cout << "Normal End..." << std::endl;
    //std::cout << "Normal Time: " << m.ms_end() << "ms " << std::endl;
    ZeroMem(b, miniSz);

    std::cout << "Optimized Start" << std::endl;
    m.start();
    in_memcpy(b, a, miniSz);
    std::cout << "Optimized End..." << std::endl;
    //std::cout << "Optimized Time: " << m.ms_end() << "ms " << std::endl;

    std::cout << "Optimized Accuracy check (1-256)" << std::endl;
    
    for (size_t sz = 1; sz <= 256; sz++) {
        in_memcpy(b, a, sz);
        for (int i = 0; i < sz; i++)
            if (a[i] != b[i]) {
                std::cout << "Copy failed at: " << i << " | Got: " << (int)b[i] << " Expected: " << (int)a[i] << std::endl; 
            }

        ZeroMem(b, miniSz);
    }

    ZeroMem(b, miniSz);

    //large copy testing
    delete[] a, b;

    const size_t largeSz = 0xffff;

    a = new byte[largeSz];
    b = new byte[largeSz];

    ZeroMem(a, largeSz);
    ZeroMem(b, largeSz);

    for (int i = 0; i < largeSz; i++)
        a[i] = i & 255;

    in_memcpy(a, b, largeSz);

    for (int i = 0; i < largeSz; i++)
        if (a[i] != b[i]) {
            std::cout << "Large Copy Fail " << i << " Got: " << (int)b[i] << " Expected: " << (int)a[i] << std::endl;
        }

    delete[] a, b;*/

    CodeMeasure m = CodeMeasure();

    //stream testing
    /*ByteStream tStream = ByteStream(2009);
    std::cout << "Ini Size: " << tStream.size() << std::endl;

    //write
    m.start();
    for (int i = 0; i < 100000; i++) {
        tStream.writeByte(i & 255);
        tStream.writeInt16(i & 0xffff);
        tStream.writeInt32(i);
        tStream.writeInt64(i);
        tStream.writeInt64(i*2);
        //std::cout << "Write " << (i+1) << " / 10,000" << "\n";
    }
    const float exe_time = m.ms_end();
    std::cout << "Write Speed: " << (((100000*23)*(1000 / exe_time))/1e6) << "mb/s " << std::endl;
    std::cout << "Write Time: " << exe_time << "ms " << std::endl;

    //pack
    std::cout << "packing stream... " << tStream.size() << std::endl;
    //tStream.pack();

    //dump contents to disk
    byte *stream_dat = tStream.getBytePtr();
    const size_t len = tStream.size();

    std::cout << "Writing Stream..." << std::endl;

    FileWrite::writeToBin("stream.bin", stream_dat, len);

    std::cout << "Stream Read Testing / Integrety" << std::endl;

    tStream.seek(0);

    m.start();
    for (int i = 0; i < 100000; i++) {
        byte b;
        i64 got;
        if ((got = b = tStream.readByte()) != (i & 255)) {
            std::cout << "Byte Fail " << (i+1) << " / 10,000 | " << (int)got << " " << (i&255) << std::endl;
            break;
        }
        if ((got = tStream.readUInt16()) != (i & 0xffff)) {
            std::cout << "Int16 Fail " << (i+1) << " / 10,000" << " | " << got << " " << (i & 0xffff) << std::endl;
            break;
        }
        if (tStream.readUInt32() != i) {
            std::cout << "Int32 Fail " << (i+1) << " / 10,000" << std::endl;
            break;
        }
        if ((got = tStream.readUInt64()) != i) {
            std::cout << "Int64 Fail 1 " << (i+1) << " / 10,000" << " | " << (unsigned)got << " " << (i * 2) << std::endl;
            break;
        }
        if ((got = tStream.readUInt64()) != i*2) {
            std::cout << "Int64 Fail 2 " << (i+1) << " / 10,000" << " | " << got << " " << (i*2) << std::endl;
            break;
        }
        //std::cout << "Read Check Pass: " << (i+1) << " / 10,000\n"; 
    }

    float exe_time2 = m.ms_end();
    std::cout << "Read Speed: " << (((100000*23)*(1000 / exe_time2))/1e6) << "mb/s " << std::endl;
    std::cout << "Read Time: " << exe_time2 << "ms " << std::endl;*/

    //tStream.free();

    //write to file
    std::cout << "BitStream Testing..." << std::endl;

    BitStream bitStream = BitStream(512);

    /*bitStream.writeBit(1);
    bitStream.writeBit(0);
    bitStream.writeBit(1);

    bitStream.writeBits(7, 4);
    bitStream.writeBit(1);
    bitStream.writeBits(7, 8);
    bitStream.writeBit(1);
    //bitStream.writeBits(7, 16);
    bitStream.writeBit(1);
    //bitStream.writeBits(7, 32); 
    bitStream.writeBit(1);
    //bitStream.writeBits(7, 64);*/

    bitStream.writeBit(1);
    bitStream.writeBit(0);
    bitStream.writeBit(1);
    bitStream.writeBit(0);

    bitStream.writeBits(7, 4);
    bitStream.writeBits(7, 8);

    bitStream.writeBits((1 << 19) - 1, 19);

    forrange(100) {
        bitStream.writeBits(7, 4);
    }

    bitStream.bitSeek(0);

    std::cout << "Read Test:" << std::endl;
    std::cout << bitStream.readBit() << std::endl;
    std::cout << bitStream.readBit() << std::endl;
    std::cout << bitStream.readBit() << std::endl;
    std::cout << bitStream.readBit() << std::endl;
    //TODO: see if these actually works lol... not lol
    const u64 read1 = bitStream.readBits(4);
    const u64 read2 = bitStream.readBits(8);
    std::cout << "Read Try 1 " << read1 << std::endl;
    std::cout << "Read Try 2 " << read2 << std::endl;

    bitStream.__int_dbg();


    byte* stream_dat2 = bitStream.getBytePtr();
    size_t len2 = bitStream.size();
    FileWrite::writeToBin("bit_stream.bin", stream_dat2, len2);

    //bitStream.free();

    return 0;
}
#include <iostream>
//#include "bmp_functions.cpp"
#pragma pack(push, 1) // Setting the alignment of the structure to 1 byte //without this programm don't work

struct BMPHeader {
    char signature[2];          // signature "BM"
    uint32_t fileSize;          // not important for us
    uint16_t reserved1;         // Reserved (must be 0)
    uint16_t reserved2;         // Reserved (must be 0)
    uint32_t dataOffset;        // doesn't matter
    uint32_t headerSize;        // doesn't matter (usually 40 byte)
    int32_t width;              // width in pixels
    int32_t height;             // in pixels
    uint16_t planes;            // this is so stranges thing
    uint16_t bitsPerPixel;      // not important for us
    uint32_t compression;       // method of compression (if it equal 0 then it didn't compressed)
    uint32_t dataSize;          // It seems like really important things, but it's always zero if not compressed, so we won't use it.
    int32_t horizontalResolution; // not important for us
    int32_t verticalResolution; // not important for us
    uint32_t colors;            // not important for us
    uint32_t importantColors;   // not important for us:)

    int get_bytes_width_with_offset() const{//we need to know the real width to rotate or blur
        int width_with_offset = width * bitsPerPixel/8;
        width_with_offset += 3 - (width_with_offset-1)%4;
        return width_with_offset;
    }
    int get_dataSize_with_offset() const{//also we need to know the real size of file, course dataSize sometimes equal to zero
        return get_bytes_width_with_offset()*height;
    }
};

#pragma pack(pop) // Restoring the default alignment
#include "head.h"
unsigned char* flipImageright(unsigned char* ImageData, BMPHeader &head) {
    uint8_t BytesPerPixel = head.bitsPerPixel / 8;
    std::swap(head.height, head.width); // we are swapping width and height to see new parameter
    uint32_t NEW_width_bytes_with_offset = head.get_bytes_width_with_offset();
    uint32_t new_dataSize_with_offset = head.get_dataSize_with_offset();
    std::swap(head.height, head.width); // swap to normal
    unsigned char* newImageData = new unsigned char[new_dataSize_with_offset]; // alocate new data for retern
    uint32_t OLD_width_bytes_with_offset = head.get_bytes_width_with_offset();

    int column_pix, row_pix, byte;
    for (int i=0; i<head.height;i++){//i is column
        column_pix = i * OLD_width_bytes_with_offset - BytesPerPixel;//pre calculate for fast
        row_pix = i * BytesPerPixel + head.width * NEW_width_bytes_with_offset;//pre calculate for fast
        for (int j=0; j<head.width;j++){//j is row
            column_pix += BytesPerPixel; //find old position
            row_pix -= NEW_width_bytes_with_offset;//find new position
            for (byte=0;byte<BytesPerPixel;byte++){//byte is for working with one part of pix(may be red, blue, green or alpha channel)
                newImageData[row_pix + byte] = ImageData[column_pix + byte]; //put old pix in new rotated-right file.     
            }
        }
    }
    std::swap(head.height, head.width);
    delete [] ImageData;
    return newImageData;
}
unsigned char* flipImageleft(unsigned char* ImageData, BMPHeader &head) {
    uint8_t BytesPerPixel = head.bitsPerPixel / 8;
    std::swap(head.height, head.width); // we are swapping width and height to see new parameter
    uint32_t NEW_width_bytes_with_offset = head.get_bytes_width_with_offset();
    uint32_t new_dataSize_with_offset = head.get_dataSize_with_offset();
    std::swap(head.height, head.width); // swap to normal
    unsigned char* newImageData = new unsigned char[new_dataSize_with_offset]; // alocate new data for retern
    uint32_t OLD_width_bytes_with_offset = head.get_bytes_width_with_offset();

    int column_pix, row_pix, byte;
    for (int i=0; i<head.height;i++){//i is column
        column_pix = (head.height - i-1) * OLD_width_bytes_with_offset - BytesPerPixel;//pre calculate for fast
        row_pix = i * BytesPerPixel - NEW_width_bytes_with_offset;//pre calculate for fast
        for (int j=0; j<head.width;j++){//j is row
            column_pix += BytesPerPixel; //find old position
            row_pix += NEW_width_bytes_with_offset;//find new position
            for (byte=0;byte<BytesPerPixel;byte++){//byte is for working with one part of pix(may be red, blue, green or alpha channel)
                newImageData[row_pix + byte] = ImageData[column_pix + byte]; //put old pix in new rotated-right file.     
            }
        }
    }
    std::swap(head.height, head.width);
    delete [] ImageData;
    return newImageData;
}

unsigned char* approximateGaussianBlurThreePass(unsigned char* ImageData, BMPHeader &head, float sigma, int accuracy = 3){
    //
    const uint8_t BytesPerPixel = head.bitsPerPixel / 8; // constant information about one pix
    const uint32_t data_size = head.get_dataSize_with_offset(); // real datasize(with offset) for allocate memory
    const uint32_t bytes_width_with_offset = head.get_bytes_width_with_offset();

    unsigned char* newImageData = new unsigned char[data_size];//allocate memory for service work

    const float radius_f = sqrt(12 * sigma * sigma / accuracy + 1);
    /*this formula is needed in order
     to find the radius for the step and
      get the desired approximation*/
    const int radius = static_cast<int>(radius_f); //make radious for one step(using accuracy)

    short sum; //this is sum that going from one side to next
    int divider;//this is for working program around bottom, top, left and right of picture
    //course we can't sum pix thet under or lefter.. of the picture
    for (int step = 0; step < accuracy; step++){//this is steps for approximation accuracy
        for (int j = 0; j < head.height; j++){//j is row
            const uint32_t yOffset = j * bytes_width_with_offset;
            for (int byte = 0; byte < BytesPerPixel; byte++){
                sum = 0;
                divider = 0;
                for (int i = 0; i < head.width; i++){//i is column//first part of one of steps of bluring(go from left to right)
                    const uint32_t offset = yOffset + i * BytesPerPixel + byte;
                    sum += ImageData[offset];
                    if (i - radius >= 0) sum -= ImageData[offset - radius * BytesPerPixel];

                    else divider++;//if we near the adge of the picture
                    newImageData[offset] = sum / divider;//put in pix average color
                }

                sum = 0;
                divider = 0;
                for (int i = head.width - 1; i >= 0; i--){//srcond part of one of steps of bluring(go from right to left)
                    const uint32_t offset = yOffset + i * BytesPerPixel + byte;//pixel coordinate in a one-dimensional array
                    sum += newImageData[offset];
                    if (i + radius < head.width) sum -= newImageData[offset + radius * BytesPerPixel];//the pix that is further than the radius must be substracted

                    else divider++;
                    ImageData[offset] = sum / divider;
                }
            }
        }

        for (int i = 0; i < head.width; i++){//the same operations but from top to bottom and vice versa
            const uint32_t xOffset = i * BytesPerPixel;
            for (int byte = 0; byte < BytesPerPixel; byte++){
                sum = 0;
                divider = 0;
                for (int j = 0; j < head.height; j++){
                    const uint32_t offset = j * bytes_width_with_offset + xOffset + byte;
                    sum += ImageData[offset];
                    if (j - radius >= 0) sum -= ImageData[offset - radius * bytes_width_with_offset];

                    else divider++;
                    newImageData[offset] = sum / divider;
                }

                sum = 0;
                divider = 0;
                for (int j = head.height - 1; j >= 0; j--){
                    const uint32_t offset = j * bytes_width_with_offset + xOffset + byte;
                    sum += newImageData[offset];
                    if (j + radius < head.height) sum -= newImageData[offset + radius * bytes_width_with_offset];

                    else divider++;
                    ImageData[offset] = sum / divider;
                }
            }
        }
    }

    delete[] newImageData;
    return ImageData;
}

#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>

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


unsigned char* flipImage(unsigned char* ImageData, BMPHeader &head) {
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

bool saveImage(const char* filename, unsigned char* imageData, const BMPHeader& header) {
    std::ofstream bmpFile(filename, std::ios::binary);

    if (!bmpFile.is_open()) {
        std::cerr << "File open error." << std::endl;
        return false;
    }

    // write header bmp
    bmpFile.write(reinterpret_cast<const char*>(&header), sizeof(header));
    // write data of pixels bmp
    bmpFile.write(reinterpret_cast<const char*>(imageData), header.get_dataSize_with_offset());

    // close savind file
    bmpFile.close();//123123

    return true;
}
int getintinput(std::string name){
    int sigma;

    std::cout << "Enter an integer for " << name << ": ";
    
    while (!(std::cin >> sigma) || std::cin.peek() != '\n') {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Please enter a valid integer for " << name << ": ";
    }

    return sigma;
}
int main() {
    // Open Bmp file in binary
    std::string open_file_name;
    std::cout<<"Please, enter a filename whithout part bmp: ";
    std::cin >> open_file_name;
    std::ifstream bmpFile(open_file_name + ".bmp", std::ios::binary);

    if (!bmpFile.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    // Read Header of BMP-file
    BMPHeader header;
    bmpFile.read(reinterpret_cast<char*>(&header), sizeof(header));

    // If signature is not BM then close programm
    if (header.signature[0] != 'B' || header.signature[1] != 'M') {
        std::cerr << "File is not a BMP image." << std::endl;
        bmpFile.close();
        return 1;
    }

    // If image is compression(compression != 0) then close programm
    if (header.compression != 0) {
        std::cerr << "Compressed BMP images are not supported." << std::endl;
        bmpFile.close();
        return 1;
    }

    // alocate new data for Imagedata(pixels unformation)
    unsigned char* imageData = new unsigned char[header.get_dataSize_with_offset()];

    // Read file header in our struct
    bmpFile.read(reinterpret_cast<char*>(imageData), header.get_dataSize_with_offset());

    // Close file becouse all data was geted
    bmpFile.close();

    // part of code where we rotating and bluring image(also get time of operation complete)
    BMPHeader & newHeader = header;
    unsigned int start_time, rotation_time, blur_time;
    start_time =  clock();
    imageData = flipImage(imageData, newHeader);
    rotation_time = clock() - start_time; // just for test (optimisation and check speed for examiner)
    
    int sigma = getintinput("sigma");
    start_time = clock();
    imageData = approximateGaussianBlurThreePass(imageData, newHeader, sigma);
    blur_time = clock() - start_time;
    
    
    std::cout << "Rotation execution time: " << rotation_time <<" ms"<< std::endl;
    std::cout << "Blur execution time: " << blur_time <<" ms"<< std::endl;

    const char* newFilename = "NewRainier.bmp";
    if (saveImage(newFilename, imageData, header)) { //if the program is not saved, then we will see it
        std::cout << "Image successfully saved to file \"" << newFilename << "\"." << std::endl;
    } else {
        std::cerr << "Error while saving the image." << std::endl;
    }
    delete[] imageData;
    // we have to free up all our memory
    std::cout<<"Press Enter on any character to end the program: ";
    
    std::cin>>open_file_name; //wait for check information, to see the speed of programm
    return 0;
}
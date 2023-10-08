#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include "bmp_functions.cpp"


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
    imageData = flipImageleft(imageData, newHeader);
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
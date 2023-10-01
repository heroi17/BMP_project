#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>

#pragma pack(push, 1) // Устанавливаем выравнивание структуры на 1 байт

// Заголовок BMP-файла
struct BMPHeader {
    char signature[2]; // Сигнатура "BM"
    uint32_t fileSize; // Размер файла
    uint16_t reserved1; // Зарезервировано (должно быть 0)
    uint16_t reserved2; // Зарезервировано (должно быть 0)
    uint32_t dataOffset; // Смещение данных от начала файла
    uint32_t headerSize; // Размер заголовка (обычно 40 байт)
    int32_t width; // Ширина изображения
    int32_t height; // Высота изображения
    uint16_t planes; // Количество цветовых плоскостей (должно быть 1)
    uint16_t bitsPerPixel; // Глубина цвета (бит на пиксель)
    uint32_t compression; // Метод сжатия (обычно 0 для несжатых изображений)
    uint32_t dataSize; // Размер данных изображения (можно оставить 0 для несжатых)
    int32_t horizontalResolution; // Горизонтальное разрешение (пикселей на метр)
    int32_t verticalResolution; // Вертикальное разрешение (пикселей на метр)
    uint32_t colors; // Количество цветов в палитре (0 для полноцветных изображений)
    uint32_t importantColors; // Количество важных цветов (0 для всех цветов)
    void printData() {
        printf("Signature: %c%c\n", signature[0], signature[1]);
        printf("File Size: %u\n", fileSize);
        printf("Reserved1: %u\n", reserved1);
        printf("Reserved2: %u\n", reserved2);
        printf("Data Offset: %u\n", dataOffset);
        printf("Header Size: %u\n", headerSize);
        printf("Width: %d\n", width);
        printf("Height: %d\n", height);
        printf("Planes: %u\n", planes);
        printf("Bits Per Pixel: %u\n", bitsPerPixel);
        printf("Compression: %u\n", compression);
        printf("Data Size: %u\n", dataSize);
        printf("Horizontal Resolution: %d\n", horizontalResolution);
        printf("Vertical Resolution: %d\n", verticalResolution);
        printf("Colors: %u\n", colors);
        printf("Important Colors: %u\n", importantColors);
    }
    int get_bytes_width_with_offset() const{
        int w = width * bitsPerPixel/8;
        w += 3 - (w-1)%4;
        return w;
    }
    int get_dataSize_with_offset() const{
        return get_bytes_width_with_offset()*height;
    }
};

#pragma pack(pop) // Восстанавливаем выравнивание по умолчанию


unsigned char* flipImage(unsigned char* ImageData, BMPHeader &head) {
    uint8_t BytesPerPixel = head.bitsPerPixel / 8;
    std::swap(head.height, head.width);
    uint32_t h1 = head.get_bytes_width_with_offset();
    uint32_t new_dataSize_with_offset = head.get_dataSize_with_offset();
    std::swap(head.height, head.width);
    unsigned char* newImageData = new unsigned char[new_dataSize_with_offset];
    uint32_t w1 = head.get_bytes_width_with_offset();

    int xx, yy;
    for (int y=0; y<head.height;y++){
        for (int x=0; x<head.width;x++){
            xx = y * w1 + x * BytesPerPixel;
            yy = y * BytesPerPixel + (head.width - x-1) * h1;
            for (int i=0;i<BytesPerPixel;i++){
                newImageData[yy + i] = ImageData[xx + i];          
            }
        }
    }
    std::swap(head.height, head.width);
    delete [] ImageData;
    return newImageData;
}


unsigned char* approximateGaussianBlurThreePass(unsigned char* ImageData, BMPHeader &head, float sigma, int accuracy = 2){
    const uint8_t BytesPerPixel = head.bitsPerPixel / 8;
    const uint32_t data_size = head.get_dataSize_with_offset();
    const uint32_t bytes_width_with_offset = head.get_bytes_width_with_offset();

    unsigned char* newImageData = new unsigned char[data_size];

    const float radius_f = sqrt(12 * sigma * sigma / accuracy + 1);
    const int radius = static_cast<int>(radius_f);

    short sum;
    int z;
    for (int n = 0; n < accuracy; n++){
        for (int y = 0; y < head.height; y++){
            const uint32_t yOffset = y * bytes_width_with_offset;
            for (int i = 0; i < BytesPerPixel; i++){
                sum = 0;
                z = 0;
                for (int x = 0; x < head.width; x++){
                    const uint32_t offset = yOffset + x * BytesPerPixel + i;
                    sum += ImageData[offset];
                    if (x - radius >= 0) sum -= ImageData[offset - radius * BytesPerPixel];

                    else z++;
                    newImageData[offset] = sum / z;
                }

                sum = 0;
                z = 0;
                for (int x = head.width - 1; x >= 0; x--){
                    const uint32_t offset = yOffset + x * BytesPerPixel + i;
                    sum += newImageData[offset];
                    if (x + radius < head.width) sum -= newImageData[offset + radius * BytesPerPixel];

                    else z++;
                    ImageData[offset] = sum / z;
                }
            }
        }

        for (int x = 0; x < head.width; x++){
            const uint32_t xOffset = x * BytesPerPixel;
            for (int i = 0; i < BytesPerPixel; i++){
                sum = 0;
                z = 0;
                for (int y = 0; y < head.height; y++){
                    const uint32_t offset = y * bytes_width_with_offset + xOffset + i;
                    sum += ImageData[offset];
                    if (y - radius >= 0) sum -= ImageData[offset - radius * bytes_width_with_offset];

                    else z++;
                    newImageData[offset] = sum / z;
                }

                sum = 0;
                z = 0;
                for (int y = head.height - 1; y >= 0; y--){
                    const uint32_t offset = y * bytes_width_with_offset + xOffset + i;
                    sum += newImageData[offset];
                    if (y + radius < head.height) sum -= newImageData[offset + radius * bytes_width_with_offset];

                    else z++;
                    ImageData[offset] = sum / z;
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
        std::cerr << "Ошибка открытия файла для сохранения." << std::endl;
        return false;
    }

    // Записываем заголовок BMP-файла
    bmpFile.write(reinterpret_cast<const char*>(&header), sizeof(header));
    // Записываем данные изображения
    bmpFile.write(reinterpret_cast<const char*>(imageData), header.get_dataSize_with_offset());

    // Закрываем файл
    bmpFile.close();

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
    // Открываем BMP-файл в бинарном режиме
    std::ifstream bmpFile("Rainier.bmp", std::ios::binary);

    if (!bmpFile.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    // Считываем заголовок BMP-файла
    BMPHeader header;
    bmpFile.read(reinterpret_cast<char*>(&header), sizeof(header));

    // Проверяем сигнатуру BMP
    if (header.signature[0] != 'B' || header.signature[1] != 'M') {
        std::cerr << "File is not a BMP image." << std::endl;
        bmpFile.close();
        return 1;
    }

    // Получаем ширину и высоту изображения из заголовка
    int width = header.width;
    int height = header.height;

    // Проверяем, что изображение не является сжатым (compression = 0)
    if (header.compression != 0) {
        std::cerr << "Compressed BMP images are not supported." << std::endl;
        bmpFile.close();
        return 1;
    }

    // Вычисляем размер данных изображения
    if (header.dataSize == 0) {
        header.dataSize = static_cast<uint32_t>(width * height * (header.bitsPerPixel / 8));
    }

    // Выделяем память для данных изображения
    unsigned char* imageData = new unsigned char[header.get_dataSize_with_offset()];

    // Считываем данные изображения в выделенную память
    bmpFile.read(reinterpret_cast<char*>(imageData), header.get_dataSize_with_offset());

    // Закрываем файл
    bmpFile.close();

    // Теперь у вас есть данные изображения в памяти (хранятся в imageData)
    // Вы можете обрабатывать изображение по своему усмотрению.
    //header.printData();
    BMPHeader & newHeader = header;
    unsigned int end_time, start_time;
    start_time =  clock();
    imageData = flipImage(imageData, newHeader);
    end_time = clock();
    std::cout << "Rotation execution time: " << end_time - start_time <<" ms"<< std::endl;
    
    int sigma = getintinput("sigma");
    start_time = clock();
    imageData = approximateGaussianBlurThreePass(imageData, newHeader, sigma);
    end_time = clock();
    std::cout << "Blur execution time: " << end_time - start_time <<" ms"<< std::endl;

    const char* newFilename = "NewRainier.bmp";
    if (saveImage(newFilename, imageData, header)) {
        std::cout << "Image successfully rotated and saved to file \"" << newFilename << "\"." << std::endl;
    } else {
        std::cerr << "Error while saving the image." << std::endl;
    }
    delete[] imageData;
    // Не забудьте освободить память, когда она больше не нужна.
    return 0;
}
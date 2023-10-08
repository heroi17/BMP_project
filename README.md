# BMP_project

The following features are implemented in this project: rotation and (approximate) Gaussian blur functions with parameters.
-----------------------------------------------------------------------------------------------
Project Contents:

1. head.h contains the BMPHeader struct.

2. main.cpp contains:
   1) Inputting the filename to open.
   2) Opening the file.
   3) Calling the rotation and blur functions.
   4) Displaying the execution time of the blur and rotation functions.
   5) Saving the rotated and blurred image as NewRainier.bmp.
   6) Waiting for any input to terminate the program.

3. bmp_functions.cpp contains:
   1) Right rotation function for uncompressed BMP files.
   2) Left rotation function for uncompressed BMP files.
   3) Approximate Gaussian blur function for uncompressed BMP files.

4. Additionally, there are 5 test images included.
-----------------------------------------------------------------------------------------------
Instructions:

1. Enter the name of a BMP file (without .bmp extension) located in the same directory as the compiled program.
2. Enter the sigma value - the blur parameter.
3. Observe the execution time printed in the console.
4. Enter any character in the console and press enter.
-----------------------------------------------------------------------------------------------
The blur implementation is based on the following material:
file:///C:/Users/dom/Downloads/Telegram%20Desktop/1775.dokl.pdf

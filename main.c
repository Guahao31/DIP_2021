#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ABS(x) (((x) > 0) ? (x) : (-(x)))

typedef unsigned char BYTE;
typedef unsigned short int WORD;
typedef unsigned int DWORD;
typedef int LONG;
// header for a .bmp-file
#pragma pack(1)
typedef struct tagBITMAPFILEHEADER{
    WORD    bfType;         // Must be 0x4D42 for a .bmp-file
    DWORD   bfSize;         // the bitmap file size with Byte
    WORD    bfReserved1;    // must be 0
    WORD    bfReserved2;    // must be 0
    DWORD   bfOffBits;      // the offset from the beginning of the fileheader
                            // to the real image data with BYTEs
}   BITMAPFILEHEADER, *PBITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER{
    DWORD   biSize;             // number of BYTEs to define BITMAPINFOHEADER structure
                                // may be 44 in application
    LONG    biWidth;            // image width  with pixels
    LONG    biHeight;           // image height with pixels
                                // also if the image is upright or not(Positive for inverted)
    WORD    biPlanes;           // Number of planes, always be 1
    WORD    biBitCount;         // Bits per pixel, which is 1, 4, 8, 16, 24 or 32
    DWORD   biCompression;      // Compression type, 0 for non-compression
    DWORD   biSizeImage;        // Image size with BYTEs
    LONG    biXPelsPerMeter;    // horizontal resolution, pixels/meter
    LONG    biYPelsPerMeter;    // vertical resolution, pixels/meter
    DWORD   biClrUsed;          // number of color indices used in the bitmap
                                // 0 for all the palette items are used
    DWORD   biClrImportant;     // number of important color indeices for image display
                                // 0 for all the items are important
}   BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack()

typedef struct tagRGBQUAD{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
}   RGBQUAD, *PRGBQUAD;

typedef struct tagRGB{
    BYTE RED;
    BYTE GREEN;
    BYTE BLUE;
} RGB, *PRGB;

typedef struct tagYUV{
    BYTE Y;
    BYTE U;
    BYTE V;
} YUV, *PYUV;

#define READ_ERROR 1
#define READ_SUCCESS 0

int readBMP(PBITMAPFILEHEADER fileHeaderPtr, PBITMAPINFOHEADER infoHeaderPtr, PRGB *rgbDataPtr, char *fileName_in)
{
    FILE *file = fopen(fileName_in, "rb");

    if (file == NULL)
    {
        printf("No such file named %s\n", fileName_in);
        return READ_ERROR;
    }

    fread(fileHeaderPtr, sizeof(BITMAPFILEHEADER), 1, file);
    fread(infoHeaderPtr, sizeof(BITMAPINFOHEADER), 1, file);

    int sizeOfPalette, sizeOfRow; // sizeOfRow with BYTEs
    // if(infoHeaderPtr->biBitCount < 16){
    //     sizeOfPalette = (int)pow(2, infoHeaderPtr->biBitCount);
    //     *rgbPelettePtrPtr = (PRGBQUAD)malloc(sizeof(RGBQUAD) * sizeOfPalette);

    //     fread(*rgbPelettePtrPtr, sizeof(RGBQUAD), sizeOfPalette, file);
    // }
    fseek(file, fileHeaderPtr->bfOffBits, 0);

    int biSize = infoHeaderPtr->biSizeImage / 3;
    *rgbDataPtr = (PRGB)malloc(sizeof(RGB) * biSize);
    fread(*rgbDataPtr, sizeof(RGB), biSize, file);
    fclose(file);
    return READ_SUCCESS;
}

void saveBMP(PBITMAPFILEHEADER fileHeaderPtr, PBITMAPINFOHEADER infoHeaderPtr, PRGB *rgbDataPtr, char *fileName_out)
{
    FILE *file = fopen(fileName_out, "wb");
    fwrite(fileHeaderPtr, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(infoHeaderPtr, sizeof(BITMAPINFOHEADER), 1, file);
    int biSize = infoHeaderPtr->biSizeImage / 3;
    fwrite(*rgbDataPtr, sizeof(RGB), biSize, file);

    fclose(file);
}

BYTE controlNum(BYTE num)
{
    if(num < 0)
        num = 0;
    if(num > 255)
        num = 255;
    return num;
}

PYUV bmp_RGBtoYUV(const BITMAPINFOHEADER infoHeader, PRGB rgbData)
{
    int biSize = infoHeader.biSizeImage / 3;
    PYUV yuvData = (PYUV)malloc(sizeof(YUV) * biSize);

    int i;
    for (i = 0; i < biSize; i++){
        yuvData[i].Y = controlNum((BYTE)(0.299 * rgbData[i].RED + 0.587 * rgbData[i].GREEN + 0.114 * rgbData[i].BLUE));
        yuvData[i].U = controlNum((BYTE)(-0.1687 * rgbData[i].RED - 0.3313 * rgbData[i].GREEN + 0.5 * rgbData[i].BLUE + 128));
        yuvData[i].V = controlNum((BYTE)(0.5 * rgbData[i].RED - 0.4187 * rgbData[i].GREEN - 0.0813 * rgbData[i].BLUE + 128));
    }
    return yuvData;
}

PRGB bmp_RGBtoGray(const BITMAPINFOHEADER infoHeader, PYUV yuvData)
{
    PRGB gray;
    int biSize = infoHeader.biSizeImage / 3;
    
    int i;
    gray = (PRGB)malloc(sizeof(RGB) * biSize);
    for (i = 0; i < biSize; i++){
        // There we don't need to rerange the value of YUV
        // cause we've done it when getting YUV
        gray[i].RED = yuvData[i].Y;
        gray[i].GREEN = yuvData[i].Y;
        gray[i].BLUE = yuvData[i].Y;
    }
    return gray;
}


int main(void)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    RGBQUAD *rgbPalette;

    PRGB rgbData;

    // Read in
    char fileName_in[] = "test.bmp";
    readBMP(&fileHeader, &infoHeader, &rgbData, fileName_in);

    int biSize = infoHeader.biSizeImage / 3; // cause RGB use 3 channels
    int i;

    // RGB -> YUV
    PYUV yuvData = bmp_RGBtoYUV(infoHeader, rgbData);

    // to gray
    PRGB gray = bmp_RGBtoGray(infoHeader, yuvData);
    char fileName_gray[] = "test_gray.bmp";
    saveBMP(&fileHeader, &infoHeader, &gray, fileName_gray);

    // change the value of Y
    PRGB rgbChanged = (PRGB)malloc(sizeof(RGB) * biSize);
    for (i = 0; i < biSize; i++)
    {
        yuvData[i].Y = controlNum((BYTE)(yuvData[i].Y + 50));
        rgbChanged[i].RED = controlNum((BYTE)(yuvData[i].Y+ 1.402 * (yuvData[i].V - 128)));
        rgbChanged[i].GREEN = controlNum((BYTE)(yuvData[i].Y - 0.34414 * (yuvData[i].U - 128) - 0.71414 * (yuvData[i].V - 128)));
        rgbChanged[i].BLUE = controlNum((BYTE)(yuvData[i].Y + 1.772 * (yuvData[i].U - 128)));
    }
    char fileName_changeY[] = "test_changeY.bmp";
    saveBMP(&fileHeader, &infoHeader, &rgbChanged, fileName_changeY);
    
    return 0;
}
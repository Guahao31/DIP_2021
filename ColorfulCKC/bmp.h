#ifndef __BMP_H__
#define __BMP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define _USE_MATH_DEFINES
#define PI M_PI

#define ABS(x) (((x) > 0) ? (x) : (-(x)))
#define MAX(a, b) (( (a) > (b) ) ? (a) : (b))
#define GRAYLEVELS (ABS((BI_WHITE)-(BI_BLACK))+1)

#define READ_ERROR 1
#define READ_SUCCESS 0
#define BI_BLACK 0
#define BI_WHITE 255
#define TYPE_EROSION 0
#define TYPE_DILATION 1
#define MIRROR_HORIZONTAL 0
#define MIRROR_VERTICAL 1
#define WINDOW_D 19
#define VAR_D 2333333333333333333333333.0
#define VAR_R 2333333333333333333333333.0

typedef unsigned char BYTE;
typedef unsigned short int WORD;
typedef unsigned int DWORD;
typedef int LONG;
typedef int bool;
typedef BYTE biType;
typedef BYTE* PbiType;


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

typedef struct muskTag{
    double *weight;
    int width;
    int height;
    double weight_sum;
} Musk, *PMusk;

int readBMP(PBITMAPFILEHEADER fileHeaderPtr, PBITMAPINFOHEADER infoHeaderPtr, PRGB *rgbDataPtr, char *fileName_in);
void saveBMP(PBITMAPFILEHEADER fileHeaderPtr, PBITMAPINFOHEADER infoHeaderPtr, PRGB *rgbDataPtr, char *fileName_out);
BYTE controlNum(BYTE num);
PYUV bmp_RGBtoYUV(const BITMAPINFOHEADER infoHeader, PRGB rgbData);
PRGB bmp_RGBtoGray(const BITMAPINFOHEADER infoHeader, PYUV yuvData);
PbiType imgBinarization(const BITMAPINFOHEADER infoHeader, PRGB gray);
PRGB biToRGB(BITMAPINFOHEADER infoHeader, PbiType biData);
bool ifErosion(const PbiType biData, int row, int col, int num_col, int N_ero);
bool ifDilation(const PbiType biData, int row, int col, int num_col, int N_dil);
PbiType biImgErosion_Dilation(BITMAPINFOHEADER infoHeader, const PbiType biData, int FUN_TYPE, int N_EorD);
PbiType biImgOpening(BITMAPINFOHEADER infoHeader, const PbiType biData, int N_ero, int N_dil);
PbiType biImgClosing(BITMAPINFOHEADER infoHeader, const PbiType biData, int N_ero, int N_dil);
PRGB bmp_YUVtoRGB(const BITMAPINFOHEADER infoHeader, PYUV yuvData);
PRGB RGB_logImage(const BITMAPINFOHEADER infoHeader, PRGB rgbData);
BYTE *histogramEqualization(const BITMAPINFOHEADER infoHeader, BYTE* channel);
PRGB bmp_RGBHistogramEqualization(const BITMAPINFOHEADER infoHeader, PRGB rgbData);
PRGB bmp_RGB_translation(BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader,\
                         PBITMAPFILEHEADER PfileHeader_changed, PBITMAPINFOHEADER PinfoHeader_changed,\
                         int delta_x, int delta_y, PRGB rgbData);
PRGB bmp_RGB_rotation(BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader,\
                  PBITMAPFILEHEADER PfileHeader_changed, PBITMAPINFOHEADER PinfoHeader_changed,\
                  double angle, PRGB rgbData);
PRGB bmp_RGB_scale(BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader,\
                   PBITMAPFILEHEADER PfileHeader_changed, PBITMAPINFOHEADER PinfoHeader_changed,\
                   double ratio_x, double ratio_y, PRGB rgbData);
PRGB bmp_RGB_shear(BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader,\
                   PBITMAPFILEHEADER PfileHeader_changed, PBITMAPINFOHEADER PinfoHeader_changed,\
                   double dx, double dy, PRGB rgbData);
PRGB bmp_RGB_mirror(BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader,\
                    PBITMAPFILEHEADER PfileHeader_changed, PBITMAPINFOHEADER PinfoHeader_changed,\
                    int mirror_type, PRGB rgbData);
PMusk meanMusk(int width, int height);
BYTE *getChannelFromGray(const BITMAPINFOHEADER infoHeader, PRGB gray_rgbData);
BYTE *Channel_Filtering(const BITMAPINFOHEADER infoHeader, BYTE *channel, PMusk musk);
PMusk LaplacianMusk(void);
BYTE *Laplacian_Enhancement(const BITMAPINFOHEADER infoHeader, BYTE *channel, PMusk musk);
BYTE *Channel_Bilateral_Filtering(const BITMAPINFOHEADER infoHeader, BYTE *channel, int windowWidth, double VarD, double VarR);
void RGBtoChannels(const BITMAPINFOHEADER infoHeader, PRGB rgb_Data, BYTE *R, BYTE *G, BYTE *B);
PRGB channelsToRGB(const BITMAPINFOHEADER infoHeader, BYTE *R, BYTE *G, BYTE *B);
PRGB RGB_Bilateral_Filtering(const BITMAPINFOHEADER infoHeader, PRGB rgbData);



#endif
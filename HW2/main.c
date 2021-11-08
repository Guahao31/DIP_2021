#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ABS(x) (((x) > 0) ? (x) : (-(x)))

typedef unsigned char BYTE;
typedef unsigned short int WORD;
typedef unsigned int DWORD;
typedef int LONG;
typedef int bool;
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

typedef int biType;
typedef int* PbiType;

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

    int biSize = ABS(infoHeaderPtr->biWidth * infoHeaderPtr->biHeight);
    int num_row = ABS(infoHeaderPtr->biHeight);
    int num_col = infoHeaderPtr->biWidth;
    int delta = num_col * 3 - (num_col * 3) / 4 * 4;

    *rgbDataPtr = (PRGB)malloc(sizeof(RGB) * biSize);
    int i;
    int cnt = 0;
    // sizeOfRow = (infoHeaderPtr->biWidth * sizeof(RGB) + 3) / 4 * 4;
    sizeOfRow = (infoHeaderPtr->biWidth * sizeof(RGB));
    for(i = 0; i < num_row; i++){
        fread((BYTE *)(*rgbDataPtr)+i*sizeOfRow, sizeof(RGB), num_col, file);
        fseek(file, delta, SEEK_CUR);
        // cnt += (sizeof(RGB) * num_col + delta);
    }
    fclose(file);
    return READ_SUCCESS;
}

void saveBMP(PBITMAPFILEHEADER fileHeaderPtr, PBITMAPINFOHEADER infoHeaderPtr, PRGB *rgbDataPtr, char *fileName_out)
{
    FILE *file = fopen(fileName_out, "wb");
    fwrite(fileHeaderPtr, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(infoHeaderPtr, sizeof(BITMAPINFOHEADER), 1, file);
    
    int num_row = ABS(infoHeaderPtr->biHeight);
    int num_col = infoHeaderPtr->biWidth;
    int delta = num_col * 3 - (num_col * 3) / 4 * 4;

    int cnt = 0;
    // int sizeOfRow = (num_col * sizeof(RGB) + 3) / 4 * 4;
    int sizeOfRow = (num_col * sizeof(RGB));
    int i;
    BYTE delta_arr[4] = {0, 0, 0, 0};
    for(i = 0; i < num_row; i++){
        fwrite((BYTE*)(*rgbDataPtr)+i*sizeOfRow, sizeof(RGB), num_col, file);
        fwrite(delta_arr, sizeof(BYTE), delta, file);
    }
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
    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);
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
    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);
    
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

#define BI_BLACK 0
#define BI_WHITE 255
PbiType imgBinarization(const BITMAPINFOHEADER infoHeader, PRGB gray)
{
    PbiType biData;
    int biSize = ABS(infoHeader.biWidth * infoHeader.biHeight);
    biData = (PbiType)malloc(sizeof(biType) * biSize);
    
    // To find the minimum and maximum value of gray
    BYTE min_gray, max_gray;
    min_gray = 255, max_gray = 0;
    int i;
    for(i = 0; i < biSize; i++){
        if(gray[i].RED < min_gray) min_gray = gray[i].RED;
        if(gray[i].RED > max_gray) max_gray = gray[i].RED;
    }

    // To find the pixels in each gray_value
    int numOfGrayValue[256];
    memset(numOfGrayValue, 0, sizeof(int)*256);
    for(i = 0; i < biSize; i++)
        numOfGrayValue[gray[i].RED]++;

    // To find the largest sigma with each threshold in range of [min_gray, max_gray]
    BYTE temp_threshold = min_gray;
    BYTE grayValueOfMaxSigma = 0;
    double maxOfSigma = 0.0;
    for(temp_threshold = min_gray; temp_threshold < max_gray; temp_threshold++){
        int N_f, N_b;
        N_f = 0, N_b = 0;

        // To calculate wf = N_{f_grd} / N, wb = N_{b_grd} / N
        // using wf+wb = 1 to simplify the problem
        if(temp_threshold < 127){
            for(i = min_gray; i < temp_threshold; i++)
                N_f += numOfGrayValue[i];
            N_b = biSize - N_f;
        } else{
            for(i = temp_threshold; i < max_gray; i++)
                N_b += numOfGrayValue[i];
            N_f = biSize - N_b;
        }
        double wf, wb;
        // There I forget to trans int to double at first :(
        wf = (double)N_f / biSize, wb = 1.0 - wf;
        // printf("%f, %f\n", wf, wb);

        double miu_f, miu_b;
        double sum_f, sum_b;
        sum_f = 0.0, sum_b = 0.0;
        for(i = min_gray; i < temp_threshold; i++)
            sum_f += (1.0 * i * numOfGrayValue[i]);
        for(i = temp_threshold; i < max_gray; i++)
            sum_b += (1.0 * i * numOfGrayValue[i]);
        miu_f = sum_f / N_f;
        miu_b = sum_b / N_b;

        double sigma_now = wb * wf * (miu_b - miu_f) * (miu_b - miu_f);
        if(sigma_now > maxOfSigma){
            maxOfSigma = sigma_now;
            grayValueOfMaxSigma = temp_threshold;
        }

        //printf("threshold:%d, sigma:%f, max_sigma:%f, grayValue: %d\n", temp_threshold, sigma_now, maxOfSigma, grayValueOfMaxSigma);
    }

    for(i = 0; i < biSize; i++)
        biData[i] = (gray[i].RED >= grayValueOfMaxSigma) ? BI_WHITE : BI_BLACK;
        // biData[i] = gray[i].RED >= 127 ? 255 : 0;
    return biData;
}

PRGB biToRGB(BITMAPINFOHEADER infoHeader, PbiType biData)
{
    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);

    PRGB rgbData = (PRGB)malloc(sizeof(RGB) * biSize);
    
    int i;
    for(i = 0; i < biSize; i++){
        rgbData[i].RED = biData[i];
        rgbData[i].GREEN = biData[i];
        rgbData[i].BLUE = biData[i];
    }

    return rgbData;
}

// bool ifErosion(const PbiType biData, int row, int col, int num_col)
// {
//     return biData[(row-1)*num_col+col-1] && biData[(row-1)*num_col+col] && biData[(row-1)*num_col+col+1]
//         && biData[row*num_col+col-1] && biData[row*num_col+col] && biData[row*num_col+col+1]
//         && biData[(row+1)*num_col+col-1] && biData[(row+1)*num_col+col] && biData[(row+1)*num_col+col+1];
// }

// bool ifDilation(const PbiType biData, int row, int col, int num_col)
// {
//     return biData[(row-1)*num_col+col-1] || biData[(row-1)*num_col+col] || biData[(row-1)*num_col+col+1]
//         || biData[row*num_col+col-1] || biData[row*num_col+col] || biData[row*num_col+col+1]
//         || biData[(row+1)*num_col+col-1] || biData[(row+1)*num_col+col] || biData[(row+1)*num_col+col+1];
// }

bool ifErosion(const PbiType biData, int row, int col, int num_col, int N_ero)
{
    int delta = N_ero / 2;
    int i, j;
    int cnt = 0;

    for(i = row - delta; i <= row + delta; i++){
        for(j = col - delta; j <= col +delta; j++)
        {
            if(biData[i * num_col + j] != 0) cnt++;
            else break;
        }
    }

    return (cnt == N_ero * N_ero);
}

bool ifDilation(const PbiType biData, int row, int col, int num_col, int N_dil)
{
    int delta = N_dil / 2;
    int i, j;
    int cnt = 0;

    for(i = row - delta; i <= row + delta; i++){
        for(j = col - delta; j <= col +delta; j++)
        {
            if(biData[i * num_col + j] != 0){
                cnt++;
                break;
            }
        }
    }

    return (cnt > 0);
}

#define TYPE_EROSION 0
#define TYPE_DILATION 1
PbiType biImgErosion_Dilation(BITMAPINFOHEADER infoHeader, const PbiType biData, int FUN_TYPE, int N_EorD)
{
    int num_row = ABS(infoHeader.biHeight);
    int biSize = ABS(infoHeader.biWidth * infoHeader.biHeight);
    int num_col = infoHeader.biWidth;

    // printf("%d %d %d", num_row, biSize, num_col);

    PbiType biData_proc = (PbiType)malloc(sizeof(biType) * biSize);
    memset(biData_proc, BI_BLACK, sizeof(biType) * biSize);

    int out_i, in_i;
    int delta = N_EorD/2;
    for(out_i = delta; out_i < num_row-delta; out_i++){
        // For one row
        int index_i0 = out_i * num_col;
        int i, j;
        for(in_i = delta; in_i < num_col-delta; in_i++){
            if(FUN_TYPE == TYPE_EROSION)
            {
                if(ifErosion(biData, out_i, in_i, num_col, N_EorD))
                biData_proc[index_i0+in_i] = BI_WHITE;
            }
            if(FUN_TYPE == TYPE_DILATION)
            {
                if(ifDilation(biData, out_i, in_i, num_col, N_EorD))
                for(i = out_i - delta; i <= out_i + delta; i++){
                    for(j = in_i - delta; j <= in_i + delta; j++)
                        biData_proc[i * num_col + j] = BI_WHITE;
                }
            }
        }
    }

    return biData_proc;
}

PbiType biImgOpening(BITMAPINFOHEADER infoHeader, const PbiType biData, int N_ero, int N_dil)
{
    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);

    PbiType biData_opening, temp;
    biData_opening = biImgErosion_Dilation(infoHeader, biData, TYPE_EROSION, N_dil);
    temp = biData_opening;
    biData_opening = biImgErosion_Dilation(infoHeader, biData_opening, TYPE_DILATION, N_ero);
    free(temp);

    return biData_opening;
}

PbiType biImgClosing(BITMAPINFOHEADER infoHeader, const PbiType biData, int N_ero, int N_dil)
{
    int biSize = ABS(infoHeader.biWidth * infoHeader.biHeight);

    PbiType biData_closing, temp;
    biData_closing = biImgErosion_Dilation(infoHeader, biData, TYPE_DILATION, N_dil);
    temp = biData_closing;
    biData_closing = biImgErosion_Dilation(infoHeader, biData_closing, TYPE_EROSION, N_ero);
    free(temp);

    return biData_closing;
}

#define N_EROSION 5
#define N_DILATION 5

int main(void)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    RGBQUAD *rgbPalette;

    PRGB rgbData;

    // Read in
    char fileName_in[] = "test.bmp";
    readBMP(&fileHeader, &infoHeader, &rgbData, fileName_in);

    saveBMP(&fileHeader, &infoHeader, &rgbData, "test_.bmp");

    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);
    int i;

    // RGB -> YUV
    PYUV yuvData = bmp_RGBtoYUV(infoHeader, rgbData);

    // to gray
    PRGB gray = bmp_RGBtoGray(infoHeader, yuvData);
    char fileName_gray[] = "test_gray.bmp";
    saveBMP(&fileHeader, &infoHeader, &gray, fileName_gray);

    PbiType biData = imgBinarization(infoHeader, gray);
    PRGB biToRGB_Data = biToRGB(infoHeader, biData);
    saveBMP(&fileHeader, &infoHeader, &biToRGB_Data, "binarization.bmp");

    PbiType biData_erosion = biImgErosion_Dilation(infoHeader, biData, TYPE_EROSION, N_EROSION);
    PRGB biErosionToRGB_Data = biToRGB(infoHeader, biData_erosion);
    saveBMP(&fileHeader, &infoHeader, &biErosionToRGB_Data, "erosion.bmp");
    
    PbiType biData_dilation = biImgErosion_Dilation(infoHeader, biData, TYPE_DILATION, N_DILATION);
    PRGB biDilationToRGB_Data = biToRGB(infoHeader, biData_dilation);
    saveBMP(&fileHeader, &infoHeader, &biDilationToRGB_Data, "dilation.bmp");

    PbiType biData_opening = biImgOpening(infoHeader, biData, N_EROSION, N_DILATION);
    PRGB biOpeningToRGB_Data = biToRGB(infoHeader, biData_opening);
    saveBMP(&fileHeader, &infoHeader, &biOpeningToRGB_Data, "opening.bmp");

    PbiType biData_closing = biImgClosing(infoHeader, biData, N_EROSION, N_DILATION);
    PRGB biClosingToRGB_Data = biToRGB(infoHeader, biData_closing);
    saveBMP(&fileHeader, &infoHeader, &biClosingToRGB_Data, "closing.bmp");

    return 0;
}
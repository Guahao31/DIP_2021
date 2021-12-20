#include "bmp.h"

#define WINDOW_D 19
#define VAR_D 2333333333333333333333333.0
#define VAR_R 2333333333333333333333333.0

BYTE *Channel_Bilateral_Filtering(const BITMAPINFOHEADER infoHeader, BYTE *channel, int windowWidth, double VarD, double VarR)
{
    int img_width = ABS(infoHeader.biWidth);
    int img_height = ABS(infoHeader.biHeight);
    int biSize = img_width * img_height;

    int left = windowWidth/2;
    int right = img_width - left - 1;
    int upper = windowWidth/2;
    int bottom = img_height - upper - 1;

    int x, y, k, l;
    double w, power, sumW, totalSumW;
    w = 0;
    power = 0;
    sumW = 0;
    totalSumW = 0;

    BYTE *result = (BYTE *)malloc(sizeof(BYTE) * biSize);
    memcpy(result, channel, sizeof(BYTE) * biSize);

    for(x = upper; x <= bottom; x++){
        for(y = left; y <= right; y++){
            // To calculate the w = d * r
            for(k = x - windowWidth/2; k <= x + windowWidth/2; k++){
                for(l = y - windowWidth/2; l <= y + windowWidth/2; l++){
                    power = ((x-k)*(x-k) + (y-l)*(y-l)) / (2*VarD*VarD);
                    power += (channel[x*img_width+y]-channel[k*img_width+l])*(channel[x*img_width+y]-channel[k*img_width+l])\
                             / (2*VarR*VarR);
                    w = exp(-power);

                    totalSumW += channel[k*img_width+l] * w;
                    sumW += w;
                }
            }

            result[x*img_width+y] = totalSumW / sumW;
            totalSumW = 0;
            sumW = 0;
        }
    }

    return result;
}

void RGBtoChannels(const BITMAPINFOHEADER infoHeader, PRGB rgb_Data, BYTE *R, BYTE *G, BYTE *B)
{
    int img_width = ABS(infoHeader.biWidth);
    int img_height = ABS(infoHeader.biHeight);
    int biSize = img_width * img_height;

    int i;
    for(i = 0; i < biSize; i++){
        R[i] = rgb_Data[i].RED;
        G[i] = rgb_Data[i].GREEN;
        B[i] = rgb_Data[i].BLUE;
    }
}

PRGB channelsToRGB(const BITMAPINFOHEADER infoHeader, BYTE *R, BYTE *G, BYTE *B)
{
    int img_width = ABS(infoHeader.biWidth);
    int img_height = ABS(infoHeader.biHeight);
    int biSize = img_width * img_height;

    PRGB rtn = (PRGB)malloc(sizeof(RGB) * biSize);
    
    int i;
    for(i = 0; i < biSize; i++){
        rtn[i].RED = R[i];
        rtn[i].GREEN = G[i];
        rtn[i].BLUE = B[i];
    }

    return rtn;
}

PRGB RGB_Bilateral_Filtering(const BITMAPINFOHEADER infoHeader, PRGB rgbData)
{
    int img_width = ABS(infoHeader.biWidth);
    int img_height = ABS(infoHeader.biHeight);
    int biSize = img_width * img_height;

    BYTE *Red = (BYTE *)malloc(sizeof(BYTE) * biSize);
    BYTE *Green = (BYTE *)malloc(sizeof(BYTE) * biSize);
    BYTE *Blue = (BYTE *)malloc(sizeof(BYTE) * biSize);
    RGBtoChannels(infoHeader, rgbData, Red, Green, Blue);

    BYTE *new_R = Channel_Bilateral_Filtering(infoHeader, Red, WINDOW_D, VAR_D, VAR_R);
    BYTE *new_G = Channel_Bilateral_Filtering(infoHeader, Green, WINDOW_D, VAR_D, VAR_R);
    BYTE *new_B = Channel_Bilateral_Filtering(infoHeader, Blue, WINDOW_D, VAR_D, VAR_R);

    free(Red);
    free(Green);
    free(Blue);

    PRGB result = channelsToRGB(infoHeader, new_R, new_G, new_B);
    free(new_R);
    free(new_G);
    free(new_B);
    return result;
}

int main(void)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    PRGB rgbData;

    // Read in
    char fileName_in[] = "test.bmp";
    readBMP(&fileHeader, &infoHeader, &rgbData, fileName_in);

    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);
    int i;

    // BYTE *Red = (BYTE *)malloc(sizeof(BYTE) * biSize);
    // BYTE *Green = (BYTE *)malloc(sizeof(BYTE) * biSize);
    // BYTE *Blue = (BYTE *)malloc(sizeof(BYTE) * biSize);
    // RGBtoChannels(infoHeader, rgbData, Red, Green, Blue);
    // PRGB test_ = channelsToRGB(infoHeader, Red, Green, Blue);
    // saveBMP(&fileHeader, &infoHeader, &test_, "test_.bmp");
    

    PRGB bilateraFiltering = RGB_Bilateral_Filtering(infoHeader, rgbData);
    saveBMP(&fileHeader, &infoHeader, &bilateraFiltering, "BilateraFiltering.bmp");
    free(bilateraFiltering);

   
    return 0;
}
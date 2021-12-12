#include "bmp.h"

typedef struct muskTag{
    double *weight;
    int width;
    int height;
    double weight_sum;
} Musk, *PMusk;

PMusk meanMusk(int width, int height)
{
    PMusk rtn = (PMusk)malloc(sizeof(Musk));
    rtn->height = height;
    rtn->width = width;
    rtn->weight_sum = 1;
    rtn->weight = (double *)malloc(sizeof(double) * width * height);

    int i,j;
    for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
            rtn->weight[i*width + j] = 1.0/(width * height);

    return rtn;
}

BYTE *getChannelFromGray(const BITMAPINFOHEADER infoHeader, PRGB gray_rgbData)
{
    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);
    BYTE *channel = (BYTE *)malloc(sizeof(BYTE) * biSize);

    int i = 0;
    for(i = 0; i < biSize; i++)
        channel[i] = gray_rgbData[i].RED;
    
    return channel;
}

BYTE *Channel_Filtering(const BITMAPINFOHEADER infoHeader, BYTE *channel, PMusk musk)
{
    int img_width = ABS(infoHeader.biWidth);
    int img_height = ABS(infoHeader.biHeight);
    int biSize = img_width * img_height;

    int left = musk->width/2;
    int right = img_width - left - 1;
    int upper = musk->height/2;
    int bottom = img_height - upper - 1;

    BYTE *rtn = (BYTE *)malloc(sizeof(BYTE) * biSize);
    memcpy(rtn, channel, sizeof(BYTE) * biSize);
    int i, j;
    double sum;
    int in_i, in_j;

    for(i = upper; i <= bottom; i++){
        for(j = left; j <= right; j++){
            sum = 0;
            for(in_i = 0; in_i < musk->height; in_i++){
                for(in_j = 0; in_j < musk->width; in_j++){
                    sum += musk->weight[in_i * musk->width + in_j] * channel[(i + in_i - musk->width/2) * img_width + j + in_j - musk->height/2];
                }
            }
            rtn[i * img_width + j] = controlNum((BYTE)sum);
        }
    }

    return rtn;
}

PMusk LaplacianMusk(void)
{
    PMusk rtn = (PMusk)malloc(sizeof(Musk));
    rtn->width = 3;
    rtn->height = 3;
    rtn->weight_sum = 0;
    rtn->weight = (double *)malloc(sizeof(double) * 3 * 3);

    rtn->weight[0] = 0;
    rtn->weight[2] = 0;
    rtn->weight[6] = 0;
    rtn->weight[8] = 0;

    rtn->weight[1] = -1;
    rtn->weight[3] = -1;
    rtn->weight[5] = -1;
    rtn->weight[7] = -1;

    rtn->weight[4] = 4;

    return rtn;
}

BYTE *Laplacian_Enhancement(const BITMAPINFOHEADER infoHeader, BYTE *channel, PMusk musk)
{
    BYTE *laplacian = Channel_Filtering(infoHeader, channel, musk);
    int img_width = ABS(infoHeader.biWidth);
    int img_height = ABS(infoHeader.biHeight);
    int biSize = img_width * img_height;

    int left = musk->width/2;
    int right = img_width - left - 1;
    int upper = musk->height/2;
    int bottom = img_height - upper - 1;

    BYTE *rtn = (BYTE *)malloc(sizeof(BYTE) * biSize);
    memcpy(rtn, channel, sizeof(BYTE) * biSize);

    int i, j;
    for(i = upper; i <= bottom; i++){
        for(j = left; j <= right; j++){
            int index = i * img_width + j;
            rtn[index] = controlNum((BYTE)(channel[index] + ABS(laplacian[index])));
        }
    }

    free(laplacian);
    return rtn;
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

    PMusk mean_musk = meanMusk(15, 15);
    PMusk laplacian_musk = LaplacianMusk();

    // Get the bmp into gray image
    PRGB gray_rgbData = bmp_RGBtoGray(infoHeader, bmp_RGBtoYUV(infoHeader, rgbData));
    saveBMP(&fileHeader, &infoHeader, &gray_rgbData, "gray.bmp");

    BYTE *gray_BiData = getChannelFromGray(infoHeader, gray_rgbData);
    // PRGB test = biToRGB(infoHeader, gray_BiData);
    // saveBMP(&fileHeader, &infoHeader, &test, "test.bmp");

    BYTE *meanFiltering = Channel_Filtering(infoHeader, gray_BiData, mean_musk);
    PRGB meanFiltering_RGB = biToRGB(infoHeader, meanFiltering);
    saveBMP(&fileHeader, &infoHeader, &meanFiltering_RGB, "meanFiltering.bmp");

    BYTE *LaplacianEnhancement = Laplacian_Enhancement(infoHeader, gray_BiData, laplacian_musk);
    PRGB LaplacianEnhancement_RGB = biToRGB(infoHeader, LaplacianEnhancement);
    saveBMP(&fileHeader, &infoHeader, &LaplacianEnhancement_RGB, "LaplacianEnhancement.bmp");
    
    //saveBMP(&fileHeader, &infoHeader, &RGB_translation, "translation.bmp");
    // free();

   
    return 0;
}
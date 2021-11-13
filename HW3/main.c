#include "bmp.h"

int main(void)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    RGBQUAD *rgbPalette;

    PRGB rgbData;

    // Read in
    char fileName_in[] = "test.bmp";
    readBMP(&fileHeader, &infoHeader, &rgbData, fileName_in);

    // To test the process of read and save bmp image
    //saveBMP(&fileHeader, &infoHeader, &rgbData, "test_.bmp");

    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);
    int i;

    // For test
    // PRGB rgbTest = bmp_YUVtoRGB(infoHeader, bmp_RGBtoYUV(infoHeader, rgbData));
    // saveBMP(&fileHeader, &infoHeader, &rgbTest, "rgbTest.bmp");
    // free(rgbTest);

    PRGB RGBImage_enhanced = RGB_logImage(infoHeader, rgbData);
    saveBMP(&fileHeader, &infoHeader, &RGBImage_enhanced, "VisibilityEnhancement.bmp");
    free(RGBImage_enhanced);

    PRGB gray = bmp_RGBtoGray(infoHeader, bmp_RGBtoYUV(infoHeader, rgbData));
    saveBMP(&fileHeader, &infoHeader, &gray, "gray.bmp");

    PRGB grayAfterHistogramEqu = bmp_RGBHistogramEqualization(infoHeader, gray);
    saveBMP(&fileHeader, &infoHeader, &grayAfterHistogramEqu, "HistogramEqualization.bmp");
    free(grayAfterHistogramEqu);

    PRGB RGBafterHistogramEqu = bmp_RGBHistogramEqualization(infoHeader, rgbData);
    saveBMP(&fileHeader, &infoHeader, &RGBafterHistogramEqu, "RGB_HistogramEqualization.bmp");
    free(RGBafterHistogramEqu);
   
    return 0;
}
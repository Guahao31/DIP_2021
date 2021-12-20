#include "bmp.h"
#define _TEST_

RGB base_color;

void get_color(PRGB color)
{
    int test;
    scanf("%x", &test);

    color->BLUE = test & 0xff;
    test >>= 8;
    color->GREEN = test & 0xff;
    test >>= 8;
    color->RED = test & 0xff;
}

PRGB get_img(const BITMAPINFOHEADER infoHeader, PRGB rgbData, RGB switch_color)
{
    int img_width = ABS(infoHeader.biWidth);
    int img_height = ABS(infoHeader.biHeight);
    int biSize = img_width * img_height;

    PRGB result = (PRGB)malloc(sizeof(RGB) * biSize);
    BYTE now_R, now_G, now_B;
    int delta_R, delta_G, delta_B;

    int i;
    for(i = 0; i < biSize; i++){
        now_R = rgbData[i].RED;
        now_G = rgbData[i].GREEN;
        now_B = rgbData[i].BLUE;

        if(now_R > 240 && now_G > 240 && now_B > 240){
            result[i].RED = 0xff;
            result[i].GREEN = 0xff;
            result[i].BLUE = 0xff;
        }else{
            delta_R = base_color.RED - now_R;
            delta_G = base_color.GREEN - now_G;
            delta_B = base_color.BLUE - now_B;

            result[i].RED = controlNum((BYTE)(delta_R + switch_color.RED));
            result[i].GREEN = controlNum((BYTE)(delta_G + switch_color.GREEN));
            result[i].BLUE = controlNum((BYTE)(delta_B + switch_color.BLUE));
        }
    }

    return result;
}



int main(void)
{
    base_color.RED = 0xc0;
    base_color.GREEN = 0x2e;
    base_color.BLUE = 0x3a;

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    PRGB rgbData;

    // Read in
    char fileName_in[] = "test.bmp";
    readBMP(&fileHeader, &infoHeader, &rgbData, fileName_in);

    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);
    int i;

    printf("Input what color you like with format ffaa00 (also FFAA00) in RGB:\n");
    RGB color_want;
    get_color(&color_want);
    #ifdef _TEST_
        printf("%02x %02x %02x\n", color_want.RED, color_want.GREEN, color_want.BLUE);
    #endif

    char output_name[90];
    printf("Please enter the name of your output: ");
    scanf("%s", output_name);
    for(i = 0; i < strlen(output_name); i++) if(output_name[i] == '.') output_name[i] = '\0';
    strcpy(output_name + strlen(output_name), ".bmp");
    
    rgbData = get_img(infoHeader, rgbData, color_want);
    saveBMP(&fileHeader, &infoHeader, &rgbData, output_name);
    free(rgbData);

    return 0;
}
#include "bmp.h"

#define _USE_MATH_DEFINES
#define PI M_PI

#define MIRROR_HORIZONTAL 0
#define MIRROR_VERTICAL 1

#define MAX(a, b) (( (a) > (b) ) ? (a) : (b))

PRGB bmp_RGB_translation(BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader,\
                         PBITMAPFILEHEADER PfileHeader_changed, PBITMAPINFOHEADER PinfoHeader_changed,\
                         int delta_x, int delta_y, PRGB rgbData)
{
    // To change the infoHeader
    memcpy(PinfoHeader_changed, &infoHeader, sizeof(BITMAPINFOHEADER));
    (*PinfoHeader_changed).biWidth = infoHeader.biWidth + delta_x;
    (*PinfoHeader_changed).biHeight = ABS(infoHeader.biHeight) / infoHeader.biHeight * (ABS(infoHeader.biHeight) + delta_y);

    int new_Width;
    int new_Height;
    new_Width = ABS((*PinfoHeader_changed).biWidth);
    new_Height = ABS((*PinfoHeader_changed).biHeight);

    int num_col = new_Width;
    int delta = num_col * 3 - (num_col * 3) / 4 * 4;
    (*PinfoHeader_changed).biSizeImage = (new_Width * sizeof(RGB) + delta)*new_Height;

    int old_Width;
    int old_Height;
    old_Width = ABS(infoHeader.biWidth);
    old_Height = ABS(infoHeader.biHeight);
 
    // To change the fileHeader
    (*PfileHeader_changed).bfOffBits = fileHeader.bfOffBits;
    (*PfileHeader_changed).bfReserved1 = fileHeader.bfReserved1;
    (*PfileHeader_changed).bfReserved2 = fileHeader.bfReserved2;
    (*PfileHeader_changed).bfType = fileHeader.bfType;
    (*PfileHeader_changed).bfSize =  sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + (*PinfoHeader_changed).biSizeImage;

    int biSize = ABS(new_Width * new_Height);
    PRGB rgbData_changed = (PRGB)malloc(sizeof(RGB) * biSize);

    // For the blank above
    int i, j;
    for(i = 0; i < delta_y; i++){
        for(j = 0; j < new_Width; j++){
            rgbData_changed[i*new_Width + j].RED = BI_BLACK;
            rgbData_changed[i*new_Width + j].GREEN = BI_BLACK;
            rgbData_changed[i*new_Width + j].BLUE = BI_BLACK;
        }
    }
    
    // For the other image pixels
    for(i = delta_y; i < new_Height; i++){
        // For the left part as white
        for(j = 0; j < delta_x; j++){
            rgbData_changed[i*new_Width + j].RED = BI_BLACK;
            rgbData_changed[i*new_Width + j].GREEN = BI_BLACK;
            rgbData_changed[i*new_Width + j].BLUE = BI_BLACK;
        }

        for(j = delta_x; j < new_Width; j++){
            int old_coord = (i-delta_y)*old_Width + (j-delta_x);
            rgbData_changed[i*new_Width + j].RED = rgbData[old_coord].RED;
            rgbData_changed[i*new_Width + j].GREEN = rgbData[old_coord].GREEN;
            rgbData_changed[i*new_Width + j].BLUE = rgbData[old_coord].BLUE;
        }
    }

    return rgbData_changed;
}

PRGB bmp_RGB_rotation(BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader,\
                  PBITMAPFILEHEADER PfileHeader_changed, PBITMAPINFOHEADER PinfoHeader_changed,\
                  double angle, PRGB rgbData)
{
    int old_Width;
    int old_Height;
    old_Width = ABS(infoHeader.biWidth);
    old_Height = ABS(infoHeader.biHeight);

    double sint, cost;
    sint = sin(angle);
    cost = cos(angle);

    memcpy(PinfoHeader_changed, &infoHeader, sizeof(BITMAPINFOHEADER));
    (*PinfoHeader_changed).biHeight = infoHeader.biHeight / old_Height * 2 * (LONG)MAX(
        ABS(-1.0 * old_Width/2 * sint + 1.0 * old_Height/2 * cost),
        ABS(1.0 * old_Width/2 * sint + 1.0 * old_Height/2 * cost)
    );
    (*PinfoHeader_changed).biWidth = 2 * (LONG)MAX(
        ABS(-1.0 * old_Width/2 * cost - 1.0 * old_Height/2 * sint),
        ABS(1.0 * old_Width/2 * cost - 1.0 * old_Height/2 * sint)
    );

    int new_Width;
    int new_Height;
    new_Width = ABS((*PinfoHeader_changed).biWidth);
    new_Height = ABS((*PinfoHeader_changed).biHeight);

    int num_col = new_Width;
    int delta = num_col * 3 - (num_col * 3) / 4 * 4;
    (*PinfoHeader_changed).biSizeImage = (new_Width * sizeof(RGB) + delta)*new_Height;

    // To change the fileHeader
    (*PfileHeader_changed).bfOffBits = fileHeader.bfOffBits;
    (*PfileHeader_changed).bfReserved1 = fileHeader.bfReserved1;
    (*PfileHeader_changed).bfReserved2 = fileHeader.bfReserved2;
    (*PfileHeader_changed).bfType = fileHeader.bfType;
    (*PfileHeader_changed).bfSize =  sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + (*PinfoHeader_changed).biSizeImage;

    int biSize = ABS(new_Width * new_Height);
    PRGB rgbData_changed = (PRGB)malloc(sizeof(RGB) * biSize);
    memset(rgbData_changed, BI_BLACK, sizeof(RGB) * biSize);

    int *ifFilled = (int *)malloc(sizeof(int) * biSize);
    memset(ifFilled, 0, sizeof(int) * biSize); // To store the pixel being filled

    int i, j;
    int x, y;
    int x_, y_;
    for(i = 0; i < old_Height; i++){
        for(j = 0; j < old_Width; j++){
            x = j - old_Width/2;
            y = old_Height/2 - i;

            x_ = x*cost - y*sint;
            y_ = x*sint + y*cost;

            x_ = x_ + new_Width/2;
            y_ = new_Height/2 - y_;

            int index = y_ * new_Width + x_;
            rgbData_changed[index].RED = rgbData[i*old_Width+j].RED;
            rgbData_changed[index].GREEN = rgbData[i*old_Width+j].GREEN;
            rgbData_changed[index].BLUE = rgbData[i*old_Width+j].BLUE;

            ifFilled[index] = 1;
        }
    }

    // Now interpolation
    for(i = 0; i < biSize; i++){
        if(ifFilled[i] == 1) continue;
        if(i%new_Width == 0){
            // Left
            rgbData_changed[i].RED = rgbData_changed[i+1].RED;
            rgbData_changed[i].GREEN = rgbData_changed[i+1].GREEN;
            rgbData_changed[i].BLUE = rgbData_changed[i+1].BLUE;
        } else if(i % new_Width == new_Width - 1){
            // Right
            rgbData_changed[i].RED = rgbData_changed[i-1].RED;
            rgbData_changed[i].GREEN = rgbData_changed[i-1].GREEN;
            rgbData_changed[i].BLUE = rgbData_changed[i-1].BLUE;
        } else{
            rgbData_changed[i].RED = (rgbData_changed[i-1].RED+rgbData_changed[i+1].RED)/2;
            rgbData_changed[i].GREEN = (rgbData_changed[i-1].GREEN+rgbData_changed[i+1].GREEN)/2;
            rgbData_changed[i].BLUE = (rgbData_changed[i-1].BLUE+rgbData_changed[i+1].BLUE)/2;
        }
    }

    return rgbData_changed;
}

PRGB bmp_RGB_scale(BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader,\
                   PBITMAPFILEHEADER PfileHeader_changed, PBITMAPINFOHEADER PinfoHeader_changed,\
                   double ratio_x, double ratio_y, PRGB rgbData)
{
    int old_Width;
    int old_Height;
    old_Width = ABS(infoHeader.biWidth);
    old_Height = ABS(infoHeader.biHeight);

    memcpy(PinfoHeader_changed, &infoHeader, sizeof(BITMAPINFOHEADER));
    (*PinfoHeader_changed).biHeight = (LONG)(infoHeader.biHeight * ratio_y);
    (*PinfoHeader_changed).biWidth = (LONG)(old_Width * ratio_x);

    int new_Width;
    int new_Height;
    new_Width = ABS((*PinfoHeader_changed).biWidth);
    new_Height = ABS((*PinfoHeader_changed).biHeight);

    int num_col = new_Width;
    int delta = num_col * 3 - (num_col * 3) / 4 * 4;
    (*PinfoHeader_changed).biSizeImage = (new_Width * sizeof(RGB) + delta)*new_Height;

    // To change the fileHeader
    (*PfileHeader_changed).bfOffBits = fileHeader.bfOffBits;
    (*PfileHeader_changed).bfReserved1 = fileHeader.bfReserved1;
    (*PfileHeader_changed).bfReserved2 = fileHeader.bfReserved2;
    (*PfileHeader_changed).bfType = fileHeader.bfType;
    (*PfileHeader_changed).bfSize =  sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + (*PinfoHeader_changed).biSizeImage;

    int biSize = ABS(new_Width * new_Height);
    PRGB rgbData_changed = (PRGB)malloc(sizeof(RGB) * biSize);

    int *ifFilled = (int *)malloc(sizeof(int) * biSize);
    memset(ifFilled, 0, sizeof(int) * biSize); // To store the pixel being filled

    int i, j;
    int i_, j_;
    int index, index_;

    for(i = 0; i < old_Height; i++){
        for(j = 0; j < old_Width; j++){
            i_ = (int)(i * ratio_y);
            j_ = (int)(j * ratio_x);

            index = i * old_Width + j;
            index_ = i_ * new_Width + j_;

            rgbData_changed[index_].RED = rgbData[index].RED;
            rgbData_changed[index_].GREEN = rgbData[index].GREEN;
            rgbData_changed[index_].BLUE = rgbData[index].BLUE;

            if(ratio_x >= 1 || ratio_y >= 1) ifFilled[index_] = 1;
        }
    }

    // Fill the row
    if(ratio_y > 1)
    for(i = 0; i < new_Height; i++){
        for(j = 0; j < new_Width; j++){
            index = i * new_Width + j;
            int index_above = (i-1) * new_Width + j;
            int index_below = (i+1) * new_Width + j;
            if(ifFilled[index] == 1) continue;
            if(i == 0){
                rgbData_changed[index].RED = rgbData_changed[index_below].RED;
                rgbData_changed[index].GREEN = rgbData_changed[index_below].GREEN;
                rgbData_changed[index].BLUE = rgbData_changed[index_below].BLUE;
            } else if(i == new_Height-1){
                rgbData_changed[index].RED = rgbData_changed[index_above].RED;
                rgbData_changed[index].GREEN = rgbData_changed[index_above].GREEN;
                rgbData_changed[index].BLUE = rgbData_changed[index_above].BLUE;
            } else{
                rgbData_changed[index].RED = (rgbData_changed[index_above].RED + rgbData_changed[index_below].RED)/2;
                rgbData_changed[index].GREEN = (rgbData_changed[index_above].GREEN + rgbData_changed[index_below].GREEN)/2;
                rgbData_changed[index].BLUE = (rgbData_changed[index_above].BLUE + rgbData_changed[index_below].BLUE)/2;
            }
        }
    }

    // Fill the cal
    if(ratio_x > 1)
    for(i = 0; i < biSize; i++){
        if(ifFilled[i] == 1) continue;
        if(i%new_Width == 0){
            // Left
            rgbData_changed[i].RED = rgbData_changed[i+1].RED;
            rgbData_changed[i].GREEN = rgbData_changed[i+1].GREEN;
            rgbData_changed[i].BLUE = rgbData_changed[i+1].BLUE;
        } else if(i % new_Width == new_Width - 1){
            // Right
            rgbData_changed[i].RED = rgbData_changed[i-1].RED;
            rgbData_changed[i].GREEN = rgbData_changed[i-1].GREEN;
            rgbData_changed[i].BLUE = rgbData_changed[i-1].BLUE;
        } else{
            rgbData_changed[i].RED = (rgbData_changed[i-1].RED+rgbData_changed[i+1].RED)/2;
            rgbData_changed[i].GREEN = (rgbData_changed[i-1].GREEN+rgbData_changed[i+1].GREEN)/2;
            rgbData_changed[i].BLUE = (rgbData_changed[i-1].BLUE+rgbData_changed[i+1].BLUE)/2;
        }
    }

    return rgbData_changed;
}

PRGB bmp_RGB_shear(BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader,\
                   PBITMAPFILEHEADER PfileHeader_changed, PBITMAPINFOHEADER PinfoHeader_changed,\
                   double dx, double dy, PRGB rgbData)
{
    int old_Width;
    int old_Height;
    old_Width = ABS(infoHeader.biWidth);
    old_Height = ABS(infoHeader.biHeight);

    memcpy(PinfoHeader_changed, &infoHeader, sizeof(BITMAPINFOHEADER));
    (*PinfoHeader_changed).biWidth = (LONG)(old_Width + old_Height * dx);
    (*PinfoHeader_changed).biHeight = (LONG)(old_Height/infoHeader.biHeight * (old_Height + old_Width * dy));

    int new_Width;
    int new_Height;
    new_Width = ABS((*PinfoHeader_changed).biWidth);
    new_Height = ABS((*PinfoHeader_changed).biHeight);

    int num_col = new_Width;
    int delta = num_col * 3 - (num_col * 3) / 4 * 4;
    (*PinfoHeader_changed).biSizeImage = (new_Width * sizeof(RGB) + delta)*new_Height;

    // To change the fileHeader
    (*PfileHeader_changed).bfOffBits = fileHeader.bfOffBits;
    (*PfileHeader_changed).bfReserved1 = fileHeader.bfReserved1;
    (*PfileHeader_changed).bfReserved2 = fileHeader.bfReserved2;
    (*PfileHeader_changed).bfType = fileHeader.bfType;
    (*PfileHeader_changed).bfSize =  sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + (*PinfoHeader_changed).biSizeImage;

    int biSize = ABS(new_Width * new_Height);
    PRGB rgbData_changed = (PRGB)malloc(sizeof(RGB) * biSize);
    memset(rgbData_changed, BI_BLACK, sizeof(RGB) * biSize); // Print white

    int i, j;
    int i_, j_;
    int index, index_;
    for(i = 0; i < old_Height; i++){
        for(j = 0; j < old_Width; j++){
            i_ = i + dy * j;
            j_ = j + dx * i;

            index = i * old_Width + j;
            index_ = i_ * new_Width + j_;
            rgbData_changed[index_].RED = rgbData[index].RED;
            rgbData_changed[index_].GREEN = rgbData[index].GREEN;
            rgbData_changed[index_].BLUE = rgbData[index].BLUE;
        }
    }

    return rgbData_changed;
}

PRGB bmp_RGB_mirror(BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader,\
                    PBITMAPFILEHEADER PfileHeader_changed, PBITMAPINFOHEADER PinfoHeader_changed,\
                    int mirror_type, PRGB rgbData)
{
    int old_Width;
    int old_Height;
    old_Width = ABS(infoHeader.biWidth);
    old_Height = ABS(infoHeader.biHeight);

    memcpy(PinfoHeader_changed, &infoHeader, sizeof(BITMAPINFOHEADER));
    (*PinfoHeader_changed).biWidth = (LONG)(infoHeader.biWidth);
    (*PinfoHeader_changed).biHeight = (LONG)(infoHeader.biHeight);

    if(mirror_type == MIRROR_HORIZONTAL) (*PinfoHeader_changed).biWidth *= 2;
    if(mirror_type == MIRROR_VERTICAL) (*PinfoHeader_changed).biHeight *= 2;

    int new_Width;
    int new_Height;
    new_Width = ABS((*PinfoHeader_changed).biWidth);
    new_Height = ABS((*PinfoHeader_changed).biHeight);

    int num_col = new_Width;
    int delta = num_col * 3 - (num_col * 3) / 4 * 4;
    (*PinfoHeader_changed).biSizeImage = (new_Width * sizeof(RGB) + delta)*new_Height;

    // To change the fileHeader
    (*PfileHeader_changed).bfOffBits = fileHeader.bfOffBits;
    (*PfileHeader_changed).bfReserved1 = fileHeader.bfReserved1;
    (*PfileHeader_changed).bfReserved2 = fileHeader.bfReserved2;
    (*PfileHeader_changed).bfType = fileHeader.bfType;
    (*PfileHeader_changed).bfSize =  sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + (*PinfoHeader_changed).biSizeImage;

    int biSize = ABS(new_Width * new_Height);
    PRGB rgbData_changed = (PRGB)malloc(sizeof(RGB) * biSize);
    memset(rgbData_changed, BI_WHITE, sizeof(RGB) * biSize); // Print white

    int i, j;
    int i_, j_;
    int index, index_;
    for(i = 0; i < old_Width; i++){
        for(j = 0; j < old_Width; j++){
            index = i * old_Width + j;

            if(mirror_type == MIRROR_HORIZONTAL){
                i_ = i;
                j_ = new_Width - j;
            } else if(MIRROR_VERTICAL){
                i_ = new_Height - i;
                j_ = j;
            }
            index_ = i_ * new_Width + j_;
            rgbData_changed[index_].RED = rgbData[index].RED;
            rgbData_changed[index_].GREEN = rgbData[index].GREEN;
            rgbData_changed[index_].BLUE = rgbData[index].BLUE;
        }
    } 
    
    return rgbData_changed;
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

    BITMAPFILEHEADER fileHeader_1;
    BITMAPINFOHEADER infoHeader_1;

    PRGB RGB_translation = bmp_RGB_translation(fileHeader, infoHeader, &fileHeader_1, &infoHeader_1, 100, 100, rgbData);
    saveBMP(&fileHeader_1, &infoHeader_1, &RGB_translation, "translation.bmp");
    free(RGB_translation);

    PRGB RGB_rotation = bmp_RGB_rotation(fileHeader, infoHeader, &fileHeader_1, &infoHeader_1, PI/6, rgbData);
    saveBMP(&fileHeader_1, &infoHeader_1, &RGB_rotation, "rotation.bmp");
    free(RGB_rotation);

    PRGB RGB_scale = bmp_RGB_scale(fileHeader, infoHeader, &fileHeader_1, &infoHeader_1, 0.9, 1.3, rgbData);
    saveBMP(&fileHeader_1, &infoHeader_1, &RGB_scale, "scale.bmp");
    free(RGB_scale);

    PRGB RGB_shear = bmp_RGB_shear(fileHeader, infoHeader, &fileHeader_1, &infoHeader_1, 0, 0.3, rgbData);
    saveBMP(&fileHeader_1, &infoHeader_1, &RGB_shear, "shear.bmp");
    free(RGB_shear);

    PRGB RGB_mirror = bmp_RGB_mirror(fileHeader, infoHeader, &fileHeader_1, &infoHeader_1, MIRROR_HORIZONTAL, rgbData);
    saveBMP(&fileHeader_1, &infoHeader_1, &RGB_mirror, "mirror.bmp");
    free(RGB_mirror);
   
    return 0;
}
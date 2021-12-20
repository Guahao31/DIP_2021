#include "bmp.h"

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
        // yuvData[i].Y = controlNum((BYTE)(((66 * rgbData[i].RED + 129 * rgbData[i].GREEN + 25 * rgbData[i].BLUE + 128)>>8) + 16));
        // yuvData[i].U = controlNum((BYTE)(((-38 * rgbData[i].RED - 74 * rgbData[i].GREEN + 112 * rgbData[i].BLUE + 128)>>8) + 128));
        // yuvData[i].V = controlNum((BYTE)(((112 * rgbData[i].RED - 94 * rgbData[i].GREEN - 18 * rgbData[i].BLUE + 128)>>8) + 128));
        yuvData[i].Y = controlNum((BYTE)(0.299 * rgbData[i].RED + 0.587 * rgbData[i].GREEN + 0.114 * rgbData[i].BLUE));
        yuvData[i].U = controlNum((BYTE)(-0.1687 * rgbData[i].RED - 0.3313 * rgbData[i].GREEN + 0.5 * rgbData[i].BLUE + 128));
        yuvData[i].V = controlNum((BYTE)(0.5 * rgbData[i].RED - 0.4187 * rgbData[i].GREEN - 0.0813 * rgbData[i].BLUE + 128));
    }
    return yuvData;
}

PRGB bmp_YUVtoRGB(const BITMAPINFOHEADER infoHeader, PYUV yuvData)
{
    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);
    PRGB rgbData = (PRGB)malloc(sizeof(RGB) * biSize);

    int i;
    for (i = 0; i < biSize; i++){
        // int A = yuvData[i].Y - 16;
        // int B = yuvData[i].U - 128;
        // int C = yuvData[i].V - 128;
        // rgbData[i].RED = controlNum((BYTE)((298*A + 409*C + 128)>>8));
        // rgbData[i].GREEN = controlNum((BYTE)((298*A - 100*B - 208*C + 128)>>8));
        // rgbData[i].BLUE = controlNum((BYTE)((298*A + 516*B + 128)>>8));
        rgbData[i].RED = controlNum((BYTE)(yuvData[i].Y+ 1.402 * (yuvData[i].V - 128)));
        rgbData[i].GREEN = controlNum((BYTE)(yuvData[i].Y - 0.34414 * (yuvData[i].U - 128) - 0.71414 * (yuvData[i].V - 128)));
        rgbData[i].BLUE = controlNum((BYTE)(yuvData[i].Y + 1.772 * (yuvData[i].U - 128)));
    }

    return rgbData;
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

PRGB RGB_logImage(const BITMAPINFOHEADER infoHeader, PRGB rgbData)
{
    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);

    PRGB rgbChanged = (PRGB)malloc(sizeof(RGB) * biSize);

    BYTE maxRed, maxGreen, maxBlue;
    maxRed = 0;
    maxGreen = 0;
    maxBlue = 0;

    int i;
    for(i = 0; i < biSize; i++){
        maxRed = maxRed > rgbData[i].RED ? maxRed : rgbData[i].RED;
        maxGreen = maxGreen > rgbData[i].GREEN ? maxGreen : rgbData[i].GREEN;
        maxBlue = maxBlue > rgbData[i].BLUE ? maxBlue : rgbData[i].BLUE;
    }

    for(i = 0; i < biSize; i++){
        rgbChanged[i].RED = (BYTE)(BI_WHITE * log(rgbData[i].RED + 1) / log(maxRed + 1));
        rgbChanged[i].GREEN = (BYTE)(BI_WHITE * log(rgbData[i].GREEN + 1) / log(maxGreen + 1));
        rgbChanged[i].BLUE = (BYTE)(BI_WHITE * log(rgbData[i].BLUE + 1) / log(maxBlue + 1));
    }
    return rgbChanged;
}

BYTE *histogramEqualization(const BITMAPINFOHEADER infoHeader, BYTE* channel)
{
    double s[GRAYLEVELS];
    int pixel_num[GRAYLEVELS];
    memset(pixel_num, 0, sizeof(int) * GRAYLEVELS);

    int biSize = ABS(infoHeader.biWidth * infoHeader.biHeight);
    BYTE *byteResult = (BYTE *)malloc(sizeof(BYTE) * biSize);

    int i;
    for(i = 0; i < biSize; i++)
        pixel_num[ channel[i] ]++;

    int *pixel_num_s = (int *)malloc(sizeof(int) * GRAYLEVELS);
    pixel_num_s[0] = pixel_num[0];
    for(i = 1; i < GRAYLEVELS; i++)
        pixel_num_s[i] = pixel_num_s[i-1] + pixel_num[i];

    for(i = 0; i < GRAYLEVELS; i++)
        s[i] = 1.0 * pixel_num_s[i] / biSize;
    free(pixel_num_s);

    double flag_axis[GRAYLEVELS];
    for(i = 0; i < GRAYLEVELS; i++)
        flag_axis[i] = 1.0 * i / GRAYLEVELS;
    
    int *s2 = (int *)malloc(sizeof(int) * GRAYLEVELS);
    
    int flag_i = 0;
    for(i = 0; i < GRAYLEVELS; i++){
        if(flag_i < GRAYLEVELS-1)
        while(s[i] > (1.0 * (flag_axis[flag_i]+flag_axis[flag_i+1]) / 2)) {
            flag_i++;
            if(flag_i == GRAYLEVELS-1) break;
        }
        s2[i] = flag_i;
    }

    for(i = 0; i < biSize; i++)
        byteResult[i] = s2[channel[i]];

    return byteResult;
}

PRGB bmp_RGBHistogramEqualization(const BITMAPINFOHEADER infoHeader, PRGB rgbData)
{
    int i;
    int biSize = ABS(infoHeader.biHeight * infoHeader.biWidth);

    BYTE *channel_red = (BYTE *)malloc(sizeof(BYTE) * biSize);
    BYTE *channel_green = (BYTE *)malloc(sizeof(BYTE) * biSize);
    BYTE *channel_blue = (BYTE *)malloc(sizeof(BYTE) * biSize);

    for(i = 0; i < biSize; i++)
    {
        channel_red[i] = rgbData[i].RED;
        channel_green[i] = rgbData[i].GREEN;
        channel_blue[i] = rgbData[i].BLUE;
    }
    BYTE *after_red = histogramEqualization(infoHeader, channel_red);
    BYTE *after_green = histogramEqualization(infoHeader, channel_green);
    BYTE *after_blue = histogramEqualization(infoHeader, channel_blue);

    PRGB RGBafterHistogramEqu = (PRGB)malloc(sizeof(RGB) * biSize);
    for(i = 0; i < biSize; i++){
        RGBafterHistogramEqu[i].RED = after_red[i];
        RGBafterHistogramEqu[i].GREEN = after_green[i];
        RGBafterHistogramEqu[i].BLUE = after_blue[i];
    }
    return RGBafterHistogramEqu;
}

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

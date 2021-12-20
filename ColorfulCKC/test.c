#include "bmp.h"

int main(void)
{
    int test;
    scanf("%x", &test);

    RGB myRGB;
    myRGB.BLUE = test & 0xff;
    test >>= 8;
    myRGB.GREEN = test & 0xff;
    test >>= 8;
    myRGB.RED = test & 0xff;

    printf("%d %d %d", myRGB.RED, myRGB.GREEN, myRGB.BLUE);
}
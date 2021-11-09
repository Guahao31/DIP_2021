HW2

Your tasks:

* Image binarization;
* Binary image erosion;
* Binary image delation;
* Binary image opening;
* Binary image closing;

In the original version, there were two major problems: the image after binarization has stripes (due to the problem of complement not being processed in HW1), and the image characteristics of the binarization are not obvious.

For the first problem, I change the funtion `int readBMP(...)` and `void saveBMP(...)`.

The second problem is caused by this piece of code:

```c
typedef BYTE unsigned char;
// To find the pixels in each gray_value
    BYTE numOfGrayValue[256];
    memset(numOfGrayValue, 0, sizeof(BYTE)*256);
    for(i = 0; i < biSize; i++)
        numOfGrayValue[gray[i].RED]++;
```

There I want to know how many pixels for each degree of gray, BUT, `BYTE`(or `unsigned char`) is too small to hold the image which may have thousands of pixels for each degree of gray, so you can change this piece of code to fix this problem:

```c
// To find the pixels in each gray_value
    int numOfGrayValue[256];
    memset(numOfGrayValue, 0, sizeof(int)*256);
    for(i = 0; i < biSize; i++)
        numOfGrayValue[gray[i].RED]++;
```


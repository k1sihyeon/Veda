#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "bmpHeader.h"

typedef unsigned char ubyte;

#define LIMIT_UBYTE(n) ((n) > UCHAR_MAX) ? UCHAR_MAX : ((n) < 0) ? 0 : (n)

extern int readBmp(char* filename, ubyte** pData, int* cols, int* rows, int* color);

int main(int argc, char** argv) {

    FILE *fp;
    BITMAPFILEHEADER bmpFileHeader;
    BITMAPINFOHEADER bmpInfoHeader;
    RGBQUAD *palrgb;

    //int cols, rows, color;
    int imageSize;
    int width, height;
    ubyte *inImg, *outImg;
    ubyte r, g, b, a, gray = 255;

    if (argc != 3) {
        fprintf(stderr, "Usage : ./%s <input.bmp> <output.bmp>\n", argv[0]);
        return -1;
    }

    // if (readBmp(argv[1], &inImg, &cols, &rows, &color) < 0) {
    //     perror("readBmp()");
    //     return -1;
    // }


    // file read
    if ((fp = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "Error : Failed to open file...\n");
        return -1;
    }

    fread(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&bmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    // true color(24)가 아니면 변환 불가
    if (bmpInfoHeader.biBitCount != 24) {
        perror("This image file do not supports 24bits color\n");
        fclose(fp);
        return -1;
    }

    width = bmpInfoHeader.biWidth;
    height = bmpInfoHeader.biHeight;

    int elemSize = bmpInfoHeader.biBitCount / 8;
    int size = width * elemSize;
    imageSize = size * height;

    printf("Resolution : %d x %d\n", width, height);
    printf("Bit Count : %d (%d)\n", bmpInfoHeader.biBitCount, elemSize);
    printf("Image Size : %d\n", imageSize);

    inImg = (ubyte *)malloc(sizeof(ubyte) * imageSize);
    outImg = (ubyte *)malloc(sizeof(ubyte) * width * height);
    
    fread(inImg, sizeof(ubyte), imageSize, fp);

    fclose(fp);

    // convert color 2 gray
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < size; x += elemSize) {
            b = inImg[x + y*size + 0];
            g = inImg[x + y*size + 1];
            r = inImg[x + y*size + 2];

            outImg[x / elemSize + y * width] = (r * 0.299F) + (g * 0.587F) + (b * 0.114F);
            // ((66 * r + 129 * g + 25 * b + 128) >> +) + 16 // 비트 연산으로 변환
        }
    }

    // write bmp
    if ((fp = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr, "Error : Failed to open file...\n");
        return -1;
    }

    // palette
    palrgb = (RGBQUAD *)malloc(sizeof(RGBQUAD) * 256);
    for (int x = 0; x < 256; x++) {
        palrgb[x].rgbBlue = palrgb[x].rgbGreen = palrgb[x].rgbRed = x;
        palrgb[x].rgbReserved = 0;
    }

    // set header
    bmpInfoHeader.biBitCount = 8;
    bmpInfoHeader.SizeImage = width * height * bmpInfoHeader.biBitCount / 8;
    bmpInfoHeader.biCompression = 0;
    bmpInfoHeader.biClrUsed = 0;
    bmpInfoHeader.biClrImportant = 0;

    bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;
    bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + bmpInfoHeader.SizeImage;

    // header write
    fwrite(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&bmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
    fwrite(palrgb, sizeof(RGBQUAD), 256, fp);
    fwrite(outImg, sizeof(ubyte), width * height, fp);

    fclose(fp);

    free(inImg);
    free(outImg);
    
    return 0;
}
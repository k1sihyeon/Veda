#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmpHeader.h"

#define BYTE unsigned char
#define widthbytes(bits) (((bits) + 31) / 32 * 4)

#ifndef M_PI
#define M_PI 3.141592654
#endif

int main(int argc, char **argv) {
    FILE *fp;
    BITMAPFILEHEADER bmpFileHeader;
    BITMAPINFOHEADER bmpInfoHeader;
    RGBQUAD palrgb[256];

    BYTE *inimg, *outimg;
    int width, height, size, imgSize;

    char input[128], output[128];

    if (argc != 3) {
        fprintf(stderr, "Usage : %s input.bmp output.bmp", argv[0]);
        return -1;
    }

    strcpy(input, argv[1]);
    strcpy(output, argv[2]);

    // file read
    if ((fp = fopen(input, "rb")) == NULL) {
        fprintf(stderr, "Error : Failed to open file(read)...\n");
        return -1;
    }

    fread(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&bmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    width = bmpInfoHeader.biWidth;
    height = bmpInfoHeader.biHeight;
    size = widthbytes(bmpInfoHeader.biBitCount * width);

    if (!bmpInfoHeader.SizeImage) 
        bmpInfoHeader.SizeImage = height * size;
    imgSize = bmpInfoHeader.SizeImage;

    inimg = (BYTE *)malloc(sizeof(BYTE) * imgSize);
    outimg = (BYTE *)malloc(sizeof(BYTE) * imgSize);

    fread(inimg, sizeof(BYTE), imgSize, fp);
    fclose(fp);

    // set rotation info
    int degree = 45;
    double radius = degree * (M_PI / 180.0f);
    double sin_v = sin(radius);
    double cos_v = cos(radius);
    int centerX = height / 2;
    int centerY = width / 2;

    // rotation
    for (int i = 0; i < height; i++) {
        int index = (height - i - 1) * size;

        for (int j = 0; j < width; j++) {
            double nx = (i-centerX) * cos_v  -  (j-centerY) * sin_v  +  centerX;
            double ny = (i-centerY) * sin_v  +  (j-centerY) * cos_v + centerY;

            if (nx < 0 || nx > height) {
                for (int k = 0; k < 3; k++)
                    outimg[index + 3*j + k] = 0;
            }
            else if (ny < 0 || ny > width) {
                for (int k = 0; k < 3; k++)
                    outimg[index + 3*j + k] = 0;
            }
            else {
                for (int k = 0; k < 3; k++)
                    outimg[index + 3*j + k] = inimg[(int)(height-nx-1)*size + (int)ny*3 + k];
            }
        }
    }

    bmpFileHeader.bfOffBits += 256 * sizeof(RGBQUAD);

    // file write
    if ((fp = fopen(output, "wb")) == NULL) {
        fprintf(stderr, "Error : Failed to open file(write)...\n");
        return -1;
    }

    fwrite(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&bmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
    fwrite(palrgb, sizeof(RGBQUAD), 256, fp);
    fwrite(outimg, sizeof(BYTE), imgSize, fp);

    free(inimg);
    free(outimg);

    fclose(fp);

    return 0;
}
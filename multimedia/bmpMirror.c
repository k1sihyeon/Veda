#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "bmpHeader.h"

#define LIMIT_UBYTE(n) ((n)>UCHAR_MAX)?UCHAR_MAX:((n)<0)?0:(n)

typedef unsigned char ubyte;

int main(int argc, char **argv) {

    FILE *fp;
    BITMAPFILEHEADER bmpFileHeader;
    BITMAPINFOHEADER bmpInfoHeader;
    RGBQUAD *palrgb;
    ubyte *inimg, outimg;
    int x, y, z, imgSize;

    if (argc != 4) {
        fprintf(stderr, "Usage : %s -[h/v] input.bmp output.bmp\n", argv[0]);
        return -1;
    }

    if ((fp = fopen(argv[2], "rb")) == NULL) {
        fprintf(stderr, "Failed to open file(read)... \n");
        return -1;
    }

    fread(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, fp);

    if (bmpInfoHeader.biBitCount != 24) {
        perror("This image Do NOT supports 24bit color!\n");
        fclose(fp);

        return -1;
    }

    unsigned int width = bmpInfoHeader.biWidth;
    unsigned int height = bmpInfoHeader.biHeight;

    int elemSize = bmpInfoHeader.biBitCount / 8;
    int size = bmpInfoHeader.biWidth * elemSize;
    imgSize = size * height;

    printf("Resolution : %d x %d\n", width, height);
    printf("Bit Count : %d\n", bmpInfoHeader.biBitCount);
    printf("Image Size : %d\n", imgSize);

    inimg = (ubyte *)malloc(sizeof(ubyte) * imgSize);
    outimg = (ubyte *)malloc(sizeof(ubyte) * imgSize);
    fread(inimg, sizeof(ubyte), imgSize, fp);

    fclose(fp);

    char ch = getopt(argc, argv, "hv:");

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < size; x += elemSize) {
            for (int z = 0; z < elemSize; z++) {
                switch (ch) {
                    case 'h':
                        outimg[x + y * size + z] = inimg[size - x - elemSize + y * size + z];
                        break;

                    case 'v': default:
                        outimg[x + y*size + z] = inimg[x + (height - y) * size + z];
                        break;
                }
            }
        }
    }

    if ((fp = fopen(argv[3], "wb")) == NULL) {
        fprintf(stderr, "Error : Failed to open file...\n");
        return -1;
    }

    fwrite(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&bmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
    fwrite(outimg, sizeof(ubyte), imgSize, fp);

    fclose(fp);

    free(inimg);
    free(outimg);

    return 0;
}

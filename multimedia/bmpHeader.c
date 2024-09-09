#include <stdio.h>

#include "bmpHeader.h"

int readBmp(char* filename, unsigned char** data, int* cols, int* rows) {
	
	BITMAPFILEHEADER bmpHeader;
	BITMAPINFOHEADER bmpInfoHeader;
	FILE *fp;
	
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		perror("ERROR\n");
		return -1;
	}
	
	fread(&bmpHeader, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&bmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
	
	// 트루컬러 지원 여부 확인
	if (bmpInfoHeader.biBitCount != 24) {
		perror("this image file does not supports 24bit true color!\n");
		fclose(fp);
		return -1;
	}
	
	*cols = bmpInfoHeader.biWidth;
	*rows = bmpInfoHeader.biHeight;
	
	printf("Resolution : %d x %d\n", bmpInfoHeader.biWidth, bmpInfoHeader.biHeight);
	printf("Bit Count : %d\n", bmpInfoHeader.biBitCount);
	
	// 실제 이미지 데이터가 있는 위치를 찾음
	fseek(fp, bmpHeader.bfOffBits, SEEK_SET);
	fread(*data, 1, bmpHeader.bfSize - bmpHeader.bfOffBits, fp);
	
	fclose(fp);
	
	return 0;
}


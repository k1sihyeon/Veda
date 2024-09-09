#ifndef __BMP_FILE_H__
#define __BMP_FILE_H__

typedef struct __attribute__((__packed__)) {
	unsigned short 	bfType;			// "BM" 표시
	unsigned int 	bfSize;			// 파일 크기
	unsigned short 	bfReserved1;	// 예약 1
	unsigned short 	bfReserved2;	// 예약 2
	unsigned int 	bfOffBits;		// 실제 이미지까지의 offset (byte)
} BITMAPFILEHEADER;

typedef struct {
	unsigned int 	biSize;				// 현 구조체의 크기
	unsigned int 	biWidth;			// 이미지 폭 (픽셀)
	unsigned int 	biHeight;			// 이미지 높이 (픽셀)
	unsigned short 	biPlanes;			// 비트 플레인 수 (항상 1)
	unsigned short 	biBitCount;			// 픽셀당 비트 수
	unsigned int 	biCompression;		// 압축 유형
	unsigned int 	SizeImage;			// 이미지 크기 (압축 전, 바이트)
	unsigned int 	biXPelsPerMeter;	// 가로 해상도
	unsigned int 	biYPelsPerMeter;	// 세로 해상도
	unsigned int 	biClrUsed;			// 실제 사용되는 색상 수
	unsigned int 	biClrImportant;		// 중요한 색상 인덱스 (0: 모두 중요)
} BITMAPINFOHEADER;

typedef struct {
	unsigned char rgbBlue;		//
	unsigned char rgbGreen;		//
	unsigned char rgbRed;		//
	unsigned char rgbReserved;	//
} RGBQUAD;

#endif

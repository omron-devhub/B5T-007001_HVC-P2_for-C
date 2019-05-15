/*---------------------------------------------------------------------------*/
/* Copyright(C)  2017  OMRON Corporation                                     */
/*                                                                           */
/* Licensed under the Apache License, Version 2.0 (the "License");           */
/* you may not use this file except in compliance with the License.          */
/* You may obtain a copy of the License at                                   */
/*                                                                           */
/*     http://www.apache.org/licenses/LICENSE-2.0                            */
/*                                                                           */
/* Unless required by applicable law or agreed to in writing, software       */
/* distributed under the License is distributed on an "AS IS" BASIS,         */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  */
/* See the License for the specific language governing permissions and       */
/* limitations under the License.                                            */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

typedef unsigned char	BYTE;
typedef unsigned char	UBYTE;
typedef          int	LONG;
typedef unsigned short	UWORD;
typedef unsigned int	DWORD;
typedef unsigned int	ULONG;

#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {
	UWORD	bfType;
	ULONG	bfSize;
	UWORD	bfReserved1;
	UWORD	bfReserved2;
	ULONG	bfOffBits;
} BITMAPFILEHEADER;             /* 14 Bytes */

typedef struct tagBITMAPINFOHEADER {
	ULONG	biSize;
	LONG	biWidth;
	LONG	biHeight;
	UWORD	biPlanes;
	UWORD	biBitCount;
	ULONG	biCompression;
	ULONG	biSizeImage;
	LONG	biXPelsPerMeter;
	LONG	biYPelsPerMeter;
	ULONG	biClrUsed;
	ULONG	biClrImportant;
} BITMAPINFOHEADER;				/* 40 Bytes */

typedef struct tagRGBQUAD {
	UBYTE	rgbBlue;
	UBYTE	rgbGreen;
	UBYTE	rgbRed;
	UBYTE	rgbReserved;
} RGBQUAD;						/* 4 Bytes */
#pragma pack(0)

void SaveBitmapFile(int nWidth, int nHeight, unsigned char *unImageBuffer, const char *szFileName)
{
    int nI;
    FILE *fp;
    DWORD   dwHeaderSize;
    BYTE    *pHeaderBuffer;
    RGBQUAD *pBmpInfoRGB;
    BITMAPINFOHEADER    *pBmpInfoHdr;
    BITMAPFILEHEADER    bmpFileHeader;

    dwHeaderSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;
    pHeaderBuffer = (BYTE*)malloc(dwHeaderSize);
    if ( pHeaderBuffer == NULL ) return;
    memset(pHeaderBuffer, 0, dwHeaderSize);

    pBmpInfoHdr = (BITMAPINFOHEADER*)pHeaderBuffer;
    pBmpInfoHdr->biSize = sizeof(BITMAPINFOHEADER);
    pBmpInfoHdr->biWidth  = nWidth;
    pBmpInfoHdr->biHeight = nHeight;
    pBmpInfoHdr->biPlanes = 1;
    pBmpInfoHdr->biCompression = 0/*BI_RGB*/;
    pBmpInfoHdr->biSizeImage = 0;
    pBmpInfoHdr->biXPelsPerMeter = 0;
    pBmpInfoHdr->biYPelsPerMeter = 0;
    pBmpInfoHdr->biBitCount = 8;
    pBmpInfoHdr->biClrUsed = 256;
    pBmpInfoHdr->biClrImportant = 0;

    pBmpInfoRGB = (RGBQUAD*)(pBmpInfoHdr+1);
    for (nI = 0; nI < 256; nI++) {
        pBmpInfoRGB->rgbRed = (BYTE)nI;
        pBmpInfoRGB->rgbGreen = (BYTE)nI;
        pBmpInfoRGB->rgbBlue = (BYTE)nI;
        pBmpInfoRGB->rgbReserved = 0;
        pBmpInfoRGB++;
    }

    bmpFileHeader.bfType = 0x4d42;  /* "BM" */
    bmpFileHeader.bfReserved1 = 0;
    bmpFileHeader.bfReserved2 = 0;

    bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + dwHeaderSize;
    bmpFileHeader.bfSize = nWidth * nHeight + bmpFileHeader.bfOffBits;

    fp = fopen(szFileName, "wb");
    if ( fp != NULL ) {
        fwrite(&bmpFileHeader, 1, sizeof(BITMAPFILEHEADER), fp);
        fwrite(pHeaderBuffer, 1, dwHeaderSize, fp);
        for ( nI=0; nI<nHeight; nI++ ) {
            fwrite(unImageBuffer+(nHeight-nI-1)*nWidth, 1, nWidth, fp);
        }
        fclose(fp);
    }
    free(pHeaderBuffer);
}

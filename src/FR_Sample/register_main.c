/*---------------------------------------------------------------------------*/
/* Copyright(C)  2018  OMRON Corporation                                     */
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
#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "uart.h"
#include "HVCApi.h"
#include "HVCDef.h"
#include "HVCExtraUartFunc.h"

#define LOGBUFFERSIZE   8192

#define UART_SETTING_TIMEOUT              1000            /* HVC setting command signal timeout period */ 
#define UART_DETECT_EXECUTE_TIMEOUT       6000            /* HVC Face Detection command signal timeout period */
#define UART_REGIST_EXECUTE_TIMEOUT       7000            /* HVC registration command signal timeout period */
#define UART_SAVE_ALBUM_TIMEOUT           860000          /* HVC save album command signal timeout period */
#define UART_LOAD_ALBUM_TIMEOUT           860000          /* HVC load album command signal timeout period */

#define SENSOR_ROLL_ANGLE_DEFAULT            0            /* Camera angle setting (0‹) */

#define BODY_THRESHOLD_DEFAULT             500            /* Threshold for Human Body Detection */
#define FACE_THRESHOLD_DEFAULT             500            /* Threshold for Face Detection */
#define HAND_THRESHOLD_DEFAULT             500            /* Threshold for Hand Detection */
#define REC_THRESHOLD_DEFAULT              500            /* Threshold for Face Recognition */

#define BODY_SIZE_RANGE_MIN_DEFAULT         30            /* Human Body Detection minimum detection size */
#define BODY_SIZE_RANGE_MAX_DEFAULT       8192            /* Human Body Detection maximum detection size */
#define HAND_SIZE_RANGE_MIN_DEFAULT         40            /* Hand Detection minimum detection size */
#define HAND_SIZE_RANGE_MAX_DEFAULT       8192            /* Hand Detection maximum detection size */
#define FACE_SIZE_RANGE_MIN_DEFAULT         64            /* Face Detection minimum detection size */
#define FACE_SIZE_RANGE_MAX_DEFAULT       8192            /* Face Detection maximum detection size */

#define FACE_POSE_DEFAULT                    0            /* Face Detection facial pose (frontal face)*/
#define FACE_ANGLE_DEFAULT                   0            /* Face Detection roll angle (}15‹)*/

#define REGISTER_USERID_NUMBER               0            /* Registered user ID number */


#ifndef WIN32
void SaveBitmapFile(int nWidth, int nHeight, unsigned char *unImageBuffer, const char *szFileName);
#define sprintf_s(buf, num, ...) sprintf(buf, __VA_ARGS__)
int kbhit(void);
#else
void SaveBitmapFile(int nWidth, int nHeight, UINT8 *unImageBuffer, const char *szFileName);

#endif

void SaveAlbumData(const char *inFileName, int inDataSize, unsigned char *inAlbumData);
void LoadAlbumData(const char *inFileName, int *outDataSize, unsigned char *outAlbumData);

/*----------------------------------------------------------------------------*/
/* UART send signal                                                           */
/* param    : int   inDataSize  send signal data                              */
/*          : UINT8 *inData     data length                                   */
/* return   : int               send signal complete data number              */
/*----------------------------------------------------------------------------*/
int UART_SendData(int inDataSize, UINT8 *inData)
{
    /* UART send signal */
    int ret = com_send(inData, inDataSize);
    return ret;
}

/*----------------------------------------------------------------------------*/
/* UART receive signal                                                        */
/* param    : int   inTimeOutTime   timeout time (ms)                         */
/*          : int   *inDataSize     receive signal data size                  */
/*          : UINT8 *outResult      receive signal data                       */
/* return   : int                   receive signal complete data number       */
/*----------------------------------------------------------------------------*/
int UART_ReceiveData(int inTimeOutTime, int inDataSize, UINT8 *outResult)
{
    /* UART receive signal */
    int ret = com_recv(inTimeOutTime, outResult, inDataSize);
    return ret;
}

/* Print Log Message */
static void PrintLog(char *pStr)
{
    puts(pStr);
}


/* HVC Execute Processing  */
int main(int argc, char *argv[])
{
    INT32 ret = 0;  /* Return code */

    INT32 inRate;
    int listBaudRate[] = {
                              9600,
                             38400,
                            115200,
                            230400,
                            460800,
                            921600
                         };

    UINT8 status;
    HVC_VERSION version;
    HVC_IMAGE *pImage = NULL;

    INT32 angleNo;
    INT32 timeOutTime;
    INT32 userID;
    INT32 dataID;
    INT32 dataNo;

    UINT8 *pAlbumData = NULL;
    INT32 albumDataSize = 0;

    int i;
    int ch = 0;
    int revision;
    char *pStr;                     /* String Buffer for logging output */

    S_STAT serialStat;              /* Serial port set value */

    serialStat.com_num = 0;
    serialStat.BaudRate = 0;        /* Default Baudrate = 9600 */
    if ( argc >= 2 ){
        serialStat.com_num  = atoi(argv[1]);
    }
    if ( com_init(&serialStat) == 0 ) {
        PrintLog("Failed to open COM port.\n");
        return (-1);
    }

    if ( argc >= 3 ){
        serialStat.BaudRate = atoi(argv[2]);
        for ( inRate = 0; (int)inRate<(sizeof(listBaudRate)/sizeof(int)); inRate++ ) {
            if ( listBaudRate[inRate] == (int)serialStat.BaudRate ) {
                break;
            }
        }
        if ( inRate >= (int)(sizeof(listBaudRate)/sizeof(int)) ) {
            PrintLog("Failed to set baudrate.\n");
            return (-1);
        }

        /* Change Baudrate */
        ret = HVC_SetBaudRate(UART_SETTING_TIMEOUT, inRate, &status);
        if ( (ret != 0) || (status != 0) ) {
            PrintLog("HVCApi(HVC_SetBaudRate) Error.\n");
            return (-1);
        }

        if ( com_init(&serialStat) == 0 ) {
            PrintLog("Failed to open COM port.\n");
            return (-1);
        }
    }

    /*****************************/
    /* Logging Buffer allocation */
    /*****************************/
    pStr = (char *)malloc(LOGBUFFERSIZE);
    if ( pStr == NULL ) {
        PrintLog("Failed to allocate Logging Buffer.\n");
        return (-1);
    }
    memset(pStr, 0, LOGBUFFERSIZE);

    do {
        /*********************************/
        /* Image Structure allocation    */
        /*********************************/
        pImage = (HVC_IMAGE *)malloc(sizeof(HVC_IMAGE));
        if ( pImage == NULL ) { /* Error processing */
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08x\n", (unsigned int)sizeof(HVC_IMAGE));
            break;
        }
        
        /*********************************/
        /* Album data allocation         */
        /*********************************/
        pAlbumData = (UINT8*)malloc(sizeof(UINT8) * (HVC_ALBUM_SIZE_MAX + 8));
        if ( pAlbumData == NULL ) { /* Error processing */
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08x\n", (unsigned int)(sizeof(UINT8) * (HVC_ALBUM_SIZE_MAX + 8)));
            break;
        }

        /*********************************/
        /* Get Model and Version         */
        /*********************************/
        ret = HVC_GetVersion(UART_SETTING_TIMEOUT, &version, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetVersion) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetVersion Response Error : 0x%02X\n", status);
            break;
        }
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetVersion : ");
        for(i = 0; i < 12; i++){
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "%c", version.string[i] );
        }
        revision = version.revision[0] + (version.revision[1]<<8) + (version.revision[2]<<16) + (version.revision[3]<<24);
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "%d.%d.%d.%d", version.major, version.minor, version.relese, revision);

        /*********************************/
        /* Set Camera Angle              */
        /*********************************/
        angleNo = SENSOR_ROLL_ANGLE_DEFAULT;
        ret = HVC_SetCameraAngle(UART_SETTING_TIMEOUT, angleNo, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetCameraAngle) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetCameraAngle Response Error : 0x%02X\n", status);
            break;
        }
        angleNo = 0xff;
        ret = HVC_GetCameraAngle(UART_SETTING_TIMEOUT, &angleNo, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetCameraAngle) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetCameraAngle Response Error : 0x%02X\n", status);
            break;
        }
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetCameraAngle : 0x%02x", angleNo);
                
        /*********************************/
        /* Load Album                    */
        /*********************************/
        LoadAlbumData("HVCAlbum.alb", &albumDataSize, pAlbumData);
        if ( albumDataSize != 0 ) {
            ret = HVC_LoadAlbum(UART_LOAD_ALBUM_TIMEOUT, pAlbumData, albumDataSize, &status);
            if ( ret != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_LoadAlbum) Error : %d\n", ret);
                break;
            }
            if ( status != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_LoadAlbum Response Error : 0x%02X\n", status);
                break;
            }
        }
        
        do {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nPress the Enter Key to register a face and theSpace Key to end: ");

            /******************/
            /* Log Output     */
            /******************/
#ifdef WIN32
            system("cls");
#else
            system("clear");
#endif
            PrintLog(pStr);

            memset(pStr, 0, LOGBUFFERSIZE);

            userID = REGISTER_USERID_NUMBER;
            /*********************************/
            /* Get Registration Info         */
            /*********************************/
            ret = HVC_GetUserData(UART_SETTING_TIMEOUT, userID, &dataNo, &status);
            if ( ret != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetUserData) Error : %d\n", ret);
                break;
            }
            if ( status != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetUserData Error : 0x%02X\n", status);
                break;
            }
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetUserData : 0x%04x", dataNo);

            dataID = 0;
            for ( i=0x01; i<0x400; i<<=1 ) {
                if ( (dataNo & i) == 0 ) break;
                dataID++;
            }
            if ( dataID >= 10 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMaximum registration number reached.");
                break;
            }

#ifdef WIN32
            ch = 0;
            if ( _kbhit() ) {
                ch = _getch();
                ch = toupper( ch );
            }
            
            if ( ch == '\r' ) {
#else
            ch = kbhit();
            
            if ( ch == '\n' ) {
#endif
                /*********************************/
                /* Execute Registration          */
                /*********************************/
                timeOutTime = UART_REGIST_EXECUTE_TIMEOUT;
                ret = HVC_Registration(timeOutTime, userID, dataID, pImage, &status);
                if ( ret != 0 ) {
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_Registration) Error : %d\n", ret);
                    break;
                }
                if ( status != 0 ) {
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_Registration Error : 0x%02X\n", status);
                    break;
                }
                SaveBitmapFile(pImage->width, pImage->height, pImage->image, "RegisterImage.bmp");
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nRegistration complete.");
            }
        } while( ch != ' ' );

        /*********************************/
        /* Save Album                    */
        /*********************************/
        ret = HVC_SaveAlbum(UART_SAVE_ALBUM_TIMEOUT, pAlbumData, &albumDataSize, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SaveAlbum) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SaveAlbum Response Error : 0x%02X\n", status);
            break;
        }
        SaveAlbumData("HVCAlbum.alb", albumDataSize, pAlbumData);
    } while(0);
    /******************/
    /* Log Output     */
    /******************/
    PrintLog(pStr);

    /********************************/
    /* Free image area             */
    /********************************/
    if( pImage != NULL ){
        free(pImage);
    }

    /********************************/
    /* Free album data area         */
    /********************************/
    if ( pAlbumData != NULL ){
        free(pAlbumData);
        pAlbumData = NULL;
    }

    com_close();

    /* Free Logging Buffer */
    if ( pStr != NULL ) {
        free(pStr);
    }
    return (0);
}

#ifndef WIN32
/*----------------------------------------------------------------------------*/
/* kbhit for Linux                                                            */
/* param    : void                                                            */
/* return   : int                    input keyboard character code            */
/*----------------------------------------------------------------------------*/
int kbhit(void)
{
    struct termios oldt, newt;
    int ch,ret;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch == EOF) {
        ret = -1;
    }
    else {
        ret = ch;
    }

    return ret;
}
#endif

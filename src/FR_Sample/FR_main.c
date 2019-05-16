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
#include "STBWrap.h"

#define LOGBUFFERSIZE   8192

#define UART_SETTING_TIMEOUT              1000            /* HVC setting command signal timeout period */
#define UART_EXECUTE_TIMEOUT              ((10+10+6+3+15+15+1+1+15+10)*1000)
                                                          /* HVC execute command signal timeout period */
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

#define STB_RETRYCOUNT_DEFAULT               2            /* Retry Count for STB */
#define STB_POSSTEADINESS_DEFAULT           30            /* Position Steadiness for STB */
#define STB_SIZESTEADINESS_DEFAULT          30            /* Size Steadiness for STB */
#define STB_FR_FRAME_DEFAULT                 5            /* Complete Frame Count for recognition in STB */
#define STB_FR_RATIO_DEFAULT                60            /* Account Ratio for recognition in STB */
#define STB_FR_ANGLEUDMIN_DEFAULT          -15            /* Up/Down face angle minimum value for recognition in STB */
#define STB_FR_ANGLEUDMAX_DEFAULT           20            /* Up/Down face angle maximum value for recognition in STB */
#define STB_FR_ANGLELRMIN_DEFAULT          -20            /* Left/Right face angle maximum value for recognition in STB */
#define STB_FR_ANGLELRMAX_DEFAULT           20            /* Left/Right face angle minimum value for recognition in STB */
#define STB_FR_THRESHOLD_DEFAULT           300            /* Threshold for recognition in STB */

#define STB_OFF                              0            /* Not Use STB */
#define STB_ON                               1            /* Use STB */

#ifdef WIN32
void SaveBitmapFile(int nWidth, int nHeight, UINT8 *unImageBuffer, const char *szFileName);
void LoadAlbumData(const char *inFileName, int *outDataSize, unsigned char *outAlbumData);
#else
void SaveBitmapFile(int nWidth, int nHeight, unsigned char *unImageBuffer, const char *szFileName);
#define sprintf_s(buf, num, ...) sprintf(buf, __VA_ARGS__)
void LoadAlbumData(const char *inFileName, int *outDataSize, unsigned char *outAlbumData);
int kbhit(void);
#endif

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
    HVC_RESULT *pHVCResult = NULL;

    int nSTBFaceCount;
    STB_FACE *pSTBFaceResult;
    int nSTBBodyCount;
    STB_BODY *pSTBBodyResult;

    INT32 angleNo;
    HVC_THRESHOLD threshold;
    HVC_SIZERANGE sizeRange;
    INT32 pose;
    INT32 angle;
    INT32 timeOutTime;
    INT32 execFlag;
    INT32 imageNo;
    
    UINT8 *pAlbumData = NULL;
    INT32 albumDataSize = 0;
    
    int i;
    int ch = 0;
    int revision;
    char *pStr;                     /* String Buffer for logging output */
    int nIndex;
    int stb_use = STB_ON;

    S_STAT serialStat;             /* Serial port set value*/

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
        for ( inRate = 0; inRate<(int)(sizeof(listBaudRate)/sizeof(int)); inRate++ ) {
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

    if ( argc >= 4 ){
        /* STB_ON/STB_OFF */
        if (strcmp (argv[3] ,"STB_ON") == 0) {
            stb_use = STB_ON;
        } else if (strcmp (argv[3] ,"STB_OFF") == 0) {
            stb_use = STB_OFF;
        } else {
            PrintLog("Please Set STB_ON or STB_OFF.\n");
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
        /* Result Structure Allocation   */
        /*********************************/
        pHVCResult = (HVC_RESULT *)malloc(sizeof(HVC_RESULT));
        if ( pHVCResult == NULL ) { /* Error processing */
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08x\n", sizeof(HVC_RESULT));
            break;
        }

         /*********************************/
        /* Album data allocation         */
        /*********************************/
        pAlbumData = (UINT8*)malloc(sizeof(UINT8) * (HVC_ALBUM_SIZE_MAX + 8));
        if ( pAlbumData == NULL ) { /* Error processing */
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08x\n", sizeof(UINT8) * (HVC_ALBUM_SIZE_MAX + 8));
            break;
        }

        /*********************************/
        /* STB Initialize                */
        /*********************************/
        if (stb_use == STB_ON) {
            ret = STB_Init(STB_FUNC_DT | STB_FUNC_PT | STB_FUNC_FR);
            if ( ret != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nSTB_Init Error : %d\n", ret);
                break;
            }
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
        /* Set Threshold Values          */
        /*********************************/
        threshold.bdThreshold = BODY_THRESHOLD_DEFAULT;
        threshold.hdThreshold = HAND_THRESHOLD_DEFAULT;
        threshold.dtThreshold = FACE_THRESHOLD_DEFAULT;
        threshold.rsThreshold = REC_THRESHOLD_DEFAULT;
        ret = HVC_SetThreshold(UART_SETTING_TIMEOUT, &threshold, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetThreshold) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetThreshold Response Error : 0x%02X\n", status);
            break;
        }
        threshold.bdThreshold = 0;
        threshold.hdThreshold = 0;
        threshold.dtThreshold = 0;
        threshold.rsThreshold = 0;
        ret = HVC_GetThreshold(UART_SETTING_TIMEOUT, &threshold, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetThreshold) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetThreshold Response Error : 0x%02X\n", status);
            break;
        }
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetThreshold : Body=%4d Hand=%4d Face=%4d Recognition=%4d",
                 threshold.bdThreshold, threshold.hdThreshold, threshold.dtThreshold, threshold.rsThreshold);
        /*********************************/
        /* Set Detection Size            */
        /*********************************/
        sizeRange.bdMinSize = BODY_SIZE_RANGE_MIN_DEFAULT;
        sizeRange.bdMaxSize = BODY_SIZE_RANGE_MAX_DEFAULT;
        sizeRange.hdMinSize = HAND_SIZE_RANGE_MIN_DEFAULT;
        sizeRange.hdMaxSize = HAND_SIZE_RANGE_MAX_DEFAULT;
        sizeRange.dtMinSize = FACE_SIZE_RANGE_MIN_DEFAULT;
        sizeRange.dtMaxSize = FACE_SIZE_RANGE_MAX_DEFAULT;
        ret = HVC_SetSizeRange(UART_SETTING_TIMEOUT, &sizeRange, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetSizeRange) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetSizeRange Response Error : 0x%02X\n", status);
            break;
        }
        sizeRange.bdMinSize = 0;
        sizeRange.bdMaxSize = 0;
        sizeRange.hdMinSize = 0;
        sizeRange.hdMaxSize = 0;
        sizeRange.dtMinSize = 0;
        sizeRange.dtMaxSize = 0;
        ret = HVC_GetSizeRange(UART_SETTING_TIMEOUT, &sizeRange, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetSizeRange) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetSizeRange Response Error : 0x%02X\n", status);
            break;
        }
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetSizeRange : Body=(%4d,%4d) Hand=(%4d,%4d) Face=(%4d,%4d)",
                                                            sizeRange.bdMinSize, sizeRange.bdMaxSize,
                                                            sizeRange.hdMinSize, sizeRange.hdMaxSize,
                                                            sizeRange.dtMinSize, sizeRange.dtMaxSize);
        /*********************************/
        /* Set Face Angle                */
        /*********************************/
        pose = FACE_POSE_DEFAULT;
        angle = FACE_ANGLE_DEFAULT;
        ret = HVC_SetFaceDetectionAngle(UART_SETTING_TIMEOUT, pose, angle, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetFaceDetectionAngle) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetFaceDetectionAngle Response Error : 0x%02X\n", status);
            break;
        }
        pose = 0xff;
        angle = 0xff;
        ret = HVC_GetFaceDetectionAngle(UART_SETTING_TIMEOUT, &pose, &angle, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetFaceDetectionAngle) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetFaceDetectionAngle Response Error : 0x%02X\n", status);
            break;
        }
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetFaceDetectionAngle : Pose = 0x%02x Angle = 0x%02x", pose, angle);

                
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

        /*********************************/
        /* Set STB Parameters            */
        /*********************************/
        if (stb_use == STB_ON) {
            ret = STB_SetTrParam(STB_RETRYCOUNT_DEFAULT, STB_POSSTEADINESS_DEFAULT, STB_SIZESTEADINESS_DEFAULT);
            if ( ret != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(STB_SetTrParam) Error : %d\n", ret);
                break;
            }
            ret = STB_SetFrParam(STB_FR_THRESHOLD_DEFAULT, STB_FR_ANGLEUDMIN_DEFAULT, STB_FR_ANGLEUDMAX_DEFAULT, STB_FR_ANGLELRMIN_DEFAULT, STB_FR_ANGLELRMAX_DEFAULT, STB_FR_FRAME_DEFAULT, STB_FR_RATIO_DEFAULT);
            if ( ret != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(STB_SetFrParam) Error : %d\n", ret);
                break;
            }
        }

        do {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nPress Space Key to end: ");

            /******************/
            /* Log Output     */
            /******************/
            PrintLog(pStr);

            memset(pStr, 0, LOGBUFFERSIZE);

            /*********************************/
            /* Execute Detection             */
            /*********************************/
            timeOutTime = UART_EXECUTE_TIMEOUT;
            if (stb_use == STB_ON) {
                execFlag = HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_DIRECTION | HVC_ACTIV_FACE_RECOGNITION;
            } else {
                execFlag = HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_RECOGNITION;
            }
            imageNo = HVC_EXECUTE_IMAGE_QVGA_HALF; /* HVC_EXECUTE_IMAGE_NONE; */
            ret = HVC_ExecuteEx(timeOutTime, execFlag, imageNo, pHVCResult, &status);
            if ( ret != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_ExecuteEx) Error : %d\n", ret);
                continue;
            }
            if ( status != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_ExecuteEx Response Error : 0x%02X\n", status);
                continue;
            }
            
            if ( imageNo == HVC_EXECUTE_IMAGE_QVGA_HALF ) {
                SaveBitmapFile(pHVCResult->image.width, pHVCResult->image.height, pHVCResult->image.image, "FRSampleImage.bmp");
            }
            
            if (stb_use == STB_ON) {
                if ( STB_Exec(pHVCResult->executedFunc, pHVCResult, &nSTBFaceCount, &pSTBFaceResult, &nSTBBodyCount, &pSTBBodyResult) == 0 ) {
                    for ( i = 0; i < nSTBFaceCount; i++ )
                    {
                        if ( pHVCResult->fdResult.num <= i ) break;

                        nIndex = pSTBFaceResult[i].nDetectID;
                        pHVCResult->fdResult.fcResult[nIndex].dtResult.posX = (short)pSTBFaceResult[i].center.x;
                        pHVCResult->fdResult.fcResult[nIndex].dtResult.posY = (short)pSTBFaceResult[i].center.y;
                        pHVCResult->fdResult.fcResult[nIndex].dtResult.size = pSTBFaceResult[i].nSize;

                        if(pHVCResult->executedFunc & HVC_ACTIV_FACE_RECOGNITION){
                            pHVCResult->fdResult.fcResult[nIndex].recognitionResult.confidence += 10000; // During
                            if ( pSTBFaceResult[i].recognition.status >= STB_STATUS_COMPLETE ) {
                                pHVCResult->fdResult.fcResult[nIndex].recognitionResult.uid = pSTBFaceResult[i].recognition.value;
                                pHVCResult->fdResult.fcResult[nIndex].recognitionResult.confidence += 10000; // Complete
                            }
                        }
                    }
                }
            }

            /* Face Detection result string */
            if(pHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION){
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Face result count:%d", pHVCResult->fdResult.num);
                for(i = 0; i < pHVCResult->fdResult.num; i++){
                    /* Detection */
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \tX:%d Y:%d Size:%d Confidence:%d", i,
                                pHVCResult->fdResult.fcResult[i].dtResult.posX, pHVCResult->fdResult.fcResult[i].dtResult.posY,
                                pHVCResult->fdResult.fcResult[i].dtResult.size, pHVCResult->fdResult.fcResult[i].dtResult.confidence);
                    if(pHVCResult->executedFunc & HVC_ACTIV_FACE_RECOGNITION){
                        /* Recognition */
                        if(-128 == pHVCResult->fdResult.fcResult[i].recognitionResult.uid){
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tRecognition not possible");
                        }
                        else if(-127 == pHVCResult->fdResult.fcResult[i].recognitionResult.uid){
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tNot registered");
                        }
                        else{
                            if (stb_use == STB_ON) {
                                if ( pHVCResult->fdResult.fcResult[i].recognitionResult.confidence >= 20000 ) { // Complete
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tID:%d Confidence:%d (*)",
                                                pHVCResult->fdResult.fcResult[i].recognitionResult.uid,
                                                pHVCResult->fdResult.fcResult[i].recognitionResult.confidence - 20000);
                                } else
                                if ( pHVCResult->fdResult.fcResult[i].recognitionResult.confidence >= 10000 ) { // During
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tID:%d Confidence:%d (-)",
                                                pHVCResult->fdResult.fcResult[i].recognitionResult.uid,
                                                pHVCResult->fdResult.fcResult[i].recognitionResult.confidence - 10000);
                                } else {
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tID:%d Confidence:%d (x)",
                                                pHVCResult->fdResult.fcResult[i].recognitionResult.uid,
                                                pHVCResult->fdResult.fcResult[i].recognitionResult.confidence);
                                }
                            }
                            else {
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tID:%d Confidence:%d",
                                            pHVCResult->fdResult.fcResult[i].recognitionResult.uid,
                                            pHVCResult->fdResult.fcResult[i].recognitionResult.confidence);
                            }
                        }
                    }
                }
            }

#ifdef WIN32
            if ( _kbhit() ) {
                ch = _getch();
                ch = toupper( ch );
            }
#else
            ch = kbhit();
#endif
        } while( ch != ' ' );
    } while(0);
    /******************/
    /* Log Output     */
    /******************/
    PrintLog(pStr);

    /********************************/
    /* Free result area             */
    /********************************/
    if( pHVCResult != NULL ){
        free(pHVCResult);
    }

    /********************************/
    /* Free AlbumData area          */
    /********************************/
    if( pAlbumData != NULL ){
        free(pAlbumData);
    }

	
    /*********************************/
    /* STB Finalize                  */
    /*********************************/
    if (stb_use == STB_ON) {
        STB_Final();
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
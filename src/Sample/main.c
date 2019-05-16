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
#include <time.h>
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
#define STB_PE_FRAME_DEFAULT                10            /* Complete Frame Count for property estimation in STB */
#define STB_PE_ANGLEUDMIN_DEFAULT          -15            /* Up/Down face angle minimum value for property estimation in STB */
#define STB_PE_ANGLEUDMAX_DEFAULT           20            /* Up/Down face angle maximum value for property estimation in STB */
#define STB_PE_ANGLELRMIN_DEFAULT          -20            /* Left/Right face angle minimum value for property estimation in STB */
#define STB_PE_ANGLELRMAX_DEFAULT           20            /* Left/Right face angle maximum value for property estimation in STB */
#define STB_PE_THRESHOLD_DEFAULT           300            /* Threshold for property estimation in STB */

#define STB_OFF                              0            /* Not Use STB */
#define STB_ON                               1            /* Use STB */

#ifdef WIN32
void SaveBitmapFile(int nWidth, int nHeight, UINT8 *unImageBuffer, const char *szFileName);
#else
void SaveBitmapFile(int nWidth, int nHeight, unsigned char *unImageBuffer, const char *szFileName);
#define sprintf_s(buf, num, ...) sprintf(buf, __VA_ARGS__)
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
    /* Send Data */
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
    /* Receive Data */
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

    INT32 agleNo;
    HVC_THRESHOLD threshold;
    HVC_SIZERANGE sizeRange;
    INT32 pose;
    INT32 angle;
    INT32 timeOutTime;
    INT32 execFlag;
    INT32 imageNo;

    char *pExStr[] = {"?", "Neutral", "Happiness", "Surprise", "Anger", "Sadness"};

    int i;
    int ch = 0;
    int revision;
    char *pStr;                     /* String Buffer for logging output */
    int nIndex;
    int stb_use = STB_ON;

    S_STAT serialStat;              /* Serial port set value*/

#ifndef WIN32
    time_t start,end;
    double diffsec, realsec;
#endif

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
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08x\n", (unsigned int)sizeof(HVC_RESULT));
            break;
        }

        /*********************************/
        /* STB Initialize                */
        /*********************************/
        if (stb_use == STB_ON) {
            ret = STB_Init(STB_FUNC_BD | STB_FUNC_DT | STB_FUNC_PT | STB_FUNC_AG | STB_FUNC_GN);
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
        agleNo = SENSOR_ROLL_ANGLE_DEFAULT;
        ret = HVC_SetCameraAngle(UART_SETTING_TIMEOUT, agleNo, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetCameraAngle) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetCameraAngle Response Error : 0x%02X\n", status);
            break;
        }
        agleNo = 0xff;
        ret = HVC_GetCameraAngle(UART_SETTING_TIMEOUT, &agleNo, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetCameraAngle) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetCameraAngle Response Error : 0x%02X\n", status);
            break;
        }
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetCameraAngle : 0x%02x", agleNo);
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
        /* Set STB Parameters            */
        /*********************************/
        if (stb_use == STB_ON) {
            ret = STB_SetTrParam(STB_RETRYCOUNT_DEFAULT, STB_POSSTEADINESS_DEFAULT, STB_SIZESTEADINESS_DEFAULT);
            if ( ret != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(STB_SetTrParam) Error : %d\n", ret);
                break;
            }
            ret = STB_SetPeParam(STB_PE_THRESHOLD_DEFAULT, STB_PE_ANGLEUDMIN_DEFAULT, STB_PE_ANGLEUDMAX_DEFAULT, STB_PE_ANGLELRMIN_DEFAULT, STB_PE_ANGLELRMAX_DEFAULT, STB_PE_FRAME_DEFAULT);
            if ( ret != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(STB_SetPeParam) Error : %d\n", ret);
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
            execFlag = HVC_ACTIV_BODY_DETECTION | HVC_ACTIV_HAND_DETECTION | HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_DIRECTION |
                     HVC_ACTIV_AGE_ESTIMATION | HVC_ACTIV_GENDER_ESTIMATION | HVC_ACTIV_GAZE_ESTIMATION | HVC_ACTIV_BLINK_ESTIMATION |
                     HVC_ACTIV_EXPRESSION_ESTIMATION;
            imageNo = HVC_EXECUTE_IMAGE_QVGA_HALF; /* HVC_EXECUTE_IMAGE_NONE; */

#ifndef WIN32
            start = time(NULL);
#endif

            ret = HVC_ExecuteEx(timeOutTime, execFlag, imageNo, pHVCResult, &status);

#ifndef WIN32
            end = time(NULL);
            diffsec = 0;
            if ((start > 0) && (end > 0) && (start < end)){
                diffsec = difftime(end, start);
            }
            realsec = diffsec;
#endif

            if ( ret != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_ExecuteEx) Error : %d\n", ret);
                continue;
            }
            if ( status != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_ExecuteEx Response Error : 0x%02X\n", status);
                continue;
            }

#ifndef WIN32
            timeOutTime = (int)(realsec * 1000 * 2);
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_ExecuteEx() took [%4.2f sec].", (double)realsec);
#endif

            if ( imageNo != HVC_EXECUTE_IMAGE_NONE ) {
                SaveBitmapFile(pHVCResult->image.width, pHVCResult->image.height, pHVCResult->image.image, "SampleImage.bmp");
            }
            
            if (stb_use == STB_ON) {
                if ( STB_Exec(pHVCResult->executedFunc, pHVCResult, &nSTBFaceCount, &pSTBFaceResult, &nSTBBodyCount, &pSTBBodyResult) == 0 ) {
                    for ( i = 0; i < nSTBBodyCount; i++ )
                    {
                        if ( pHVCResult->bdResult.num <= i ) break;

                        nIndex = pSTBBodyResult[i].nDetectID;
                        pHVCResult->bdResult.bdResult[nIndex].posX = (short)pSTBBodyResult[i].center.x;
                        pHVCResult->bdResult.bdResult[nIndex].posY = (short)pSTBBodyResult[i].center.y;
                        pHVCResult->bdResult.bdResult[nIndex].size = pSTBBodyResult[i].nSize;
                    }
                    for ( i = 0; i < nSTBFaceCount; i++ )
                    {
                        if ( pHVCResult->fdResult.num <= i ) break;

                        nIndex = pSTBFaceResult[i].nDetectID;
                        pHVCResult->fdResult.fcResult[nIndex].dtResult.posX = (short)pSTBFaceResult[i].center.x;
                        pHVCResult->fdResult.fcResult[nIndex].dtResult.posY = (short)pSTBFaceResult[i].center.y;
                        pHVCResult->fdResult.fcResult[nIndex].dtResult.size = pSTBFaceResult[i].nSize;

                        if(pHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION){
                            pHVCResult->fdResult.fcResult[nIndex].ageResult.confidence += 10000; // During
                            if ( pSTBFaceResult[i].age.status >= STB_STATUS_COMPLETE ) {
                                pHVCResult->fdResult.fcResult[nIndex].ageResult.age = pSTBFaceResult[i].age.value;
                                pHVCResult->fdResult.fcResult[nIndex].ageResult.confidence += 10000; // Complete
                            }
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION){
                            pHVCResult->fdResult.fcResult[nIndex].genderResult.confidence += 10000; // During
                            if ( pSTBFaceResult[i].gender.status >= STB_STATUS_COMPLETE ) {
                                pHVCResult->fdResult.fcResult[nIndex].genderResult.gender = pSTBFaceResult[i].gender.value;
                                pHVCResult->fdResult.fcResult[nIndex].genderResult.confidence += 10000; // Complete
                            }
                        }
                    }
                }

                if(pHVCResult->executedFunc & HVC_ACTIV_BODY_DETECTION){
                    /* Body Detection result string */
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Body result count:%d", pHVCResult->bdResult.num);
                    for(i = 0; i < pHVCResult->bdResult.num; i++){
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d TR_ID:%d X:%d Y:%d Size:%d Confidence:%d", i,
                                    pSTBBodyResult[i].nTrackingID,
                                    pHVCResult->bdResult.bdResult[i].posX, pHVCResult->bdResult.bdResult[i].posY,
                                    pHVCResult->bdResult.bdResult[i].size, pHVCResult->bdResult.bdResult[i].confidence);
                    }
                }
                if(pHVCResult->executedFunc & HVC_ACTIV_HAND_DETECTION){
                    /* Hand Detection result string */
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Hand result count:%d", pHVCResult->hdResult.num);
                    for(i = 0; i < pHVCResult->hdResult.num; i++){
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                    pHVCResult->hdResult.hdResult[i].posX, pHVCResult->hdResult.hdResult[i].posY,
                                    pHVCResult->hdResult.hdResult[i].size, pHVCResult->hdResult.hdResult[i].confidence);
                    }
                }

                /* Face Detection result string */
                if(pHVCResult->executedFunc &
                        (HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_DIRECTION |
                         HVC_ACTIV_AGE_ESTIMATION | HVC_ACTIV_GENDER_ESTIMATION |
                         HVC_ACTIV_GAZE_ESTIMATION | HVC_ACTIV_BLINK_ESTIMATION |
                         HVC_ACTIV_EXPRESSION_ESTIMATION | HVC_ACTIV_FACE_RECOGNITION)){
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Face result count:%d", pHVCResult->fdResult.num);
                    for(i = 0; i < pHVCResult->fdResult.num; i++){
                        if(pHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION){
                            /* Detection */
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d TR_ID:%d X:%d Y:%d Size:%d Confidence:%d", i,
                                        pSTBFaceResult[i].nTrackingID,
                                        pHVCResult->fdResult.fcResult[i].dtResult.posX, pHVCResult->fdResult.fcResult[i].dtResult.posY,
                                        pHVCResult->fdResult.fcResult[i].dtResult.size, pHVCResult->fdResult.fcResult[i].dtResult.confidence);
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_FACE_DIRECTION){
                            /* Face Direction */
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Face Direction\tLR:%d UD:%d Roll:%d Confidence:%d",
                                        pHVCResult->fdResult.fcResult[i].dirResult.yaw, pHVCResult->fdResult.fcResult[i].dirResult.pitch,
                                        pHVCResult->fdResult.fcResult[i].dirResult.roll, pHVCResult->fdResult.fcResult[i].dirResult.confidence);
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION){
                            /* Age */
                            if(-128 == pHVCResult->fdResult.fcResult[i].ageResult.age){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tEstimation not possible");
                            } else {
                                if ( pHVCResult->fdResult.fcResult[i].ageResult.confidence >= 20000 ) { // Complete
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tAge:%d Confidence:%d (*)",
                                                pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence - 20000);
                                } else
                                if ( pHVCResult->fdResult.fcResult[i].ageResult.confidence >= 10000 ) { // During
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tAge:%d Confidence:%d (-)",
                                                pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence - 10000);
                                } else {
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tAge:%d Confidence:%d (x)",
                                                pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence);
                                }
                            }
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION){
                            /* Gender */
                            if(-128 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tEstimation not possible");
                            }
                            else{
                                if ( pHVCResult->fdResult.fcResult[i].genderResult.confidence >= 20000 ) { // Complete
                                    if(1 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (*)",
                                                    "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 20000);
                                    }
                                    else{
                                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (*)",
                                                    "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 20000);
                                    }
                                } else
                                if ( pHVCResult->fdResult.fcResult[i].genderResult.confidence >= 10000 ) { // During
                                    if(1 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (-)",
                                                    "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 10000);
                                    }
                                    else{
                                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (-)",
                                                    "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 10000);
                                    }
                                } else {
                                    if(1 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (x)",
                                                    "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence);
                                    }
                                    else{
                                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (x)",
                                                    "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence);
                                    }
                                }
                            }
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_GAZE_ESTIMATION){
                            /* Gaze */
                            if((-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR) ||
                                (-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD)){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gaze\t\tEstimation not possible");
                            }
                            else{
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gaze\t\tLR:%d UD:%d",
                                            pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR, pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD);
                            }
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_BLINK_ESTIMATION){
                            /* Blink */
                            if((-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioL) ||
                                (-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioR)){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Blink\t\tEstimation not possible");
                            }
                            else{
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Blink\t\tLeft:%d Right:%d",
                                            pHVCResult->fdResult.fcResult[i].blinkResult.ratioL, pHVCResult->fdResult.fcResult[i].blinkResult.ratioR);
                            }
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_EXPRESSION_ESTIMATION){
                            /* Expression */
                            if(-128 == pHVCResult->fdResult.fcResult[i].expressionResult.score[0]){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Expression\tEstimation not possible");
                            }
                            else{
                                if(pHVCResult->fdResult.fcResult[i].expressionResult.topExpression > EX_SADNESS){
                                    pHVCResult->fdResult.fcResult[i].expressionResult.topExpression = 0;
                                }
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Expression\tExpression:%s Score:%d, %d, %d, %d, %d Degree:%d",
                                            pExStr[pHVCResult->fdResult.fcResult[i].expressionResult.topExpression],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.score[0],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.score[1],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.score[2],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.score[3],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.score[4],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.degree);
                            }
                        }
                    }
                }
            } else {
                if(pHVCResult->executedFunc & HVC_ACTIV_BODY_DETECTION){
                    /* Body Detection result string */
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Body result count:%d", pHVCResult->bdResult.num);
                    for(i = 0; i < pHVCResult->bdResult.num; i++){
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                    pHVCResult->bdResult.bdResult[i].posX, pHVCResult->bdResult.bdResult[i].posY,
                                    pHVCResult->bdResult.bdResult[i].size, pHVCResult->bdResult.bdResult[i].confidence);
                    }
                }
                if(pHVCResult->executedFunc & HVC_ACTIV_HAND_DETECTION){
                    /* Hand Detection result string */
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Hand result count:%d", pHVCResult->hdResult.num);
                    for(i = 0; i < pHVCResult->hdResult.num; i++){
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                    pHVCResult->hdResult.hdResult[i].posX, pHVCResult->hdResult.hdResult[i].posY,
                                    pHVCResult->hdResult.hdResult[i].size, pHVCResult->hdResult.hdResult[i].confidence);
                    }
                }

                /* Face Detection result string */
                if(pHVCResult->executedFunc &
                        (HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_DIRECTION |
                         HVC_ACTIV_AGE_ESTIMATION | HVC_ACTIV_GENDER_ESTIMATION |
                         HVC_ACTIV_GAZE_ESTIMATION | HVC_ACTIV_BLINK_ESTIMATION |
                         HVC_ACTIV_EXPRESSION_ESTIMATION | HVC_ACTIV_FACE_RECOGNITION)){
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Face result count:%d", pHVCResult->fdResult.num);
                    for(i = 0; i < pHVCResult->fdResult.num; i++){
                        if(pHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION){
                            /* Detection */
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                        pHVCResult->fdResult.fcResult[i].dtResult.posX, pHVCResult->fdResult.fcResult[i].dtResult.posY,
                                        pHVCResult->fdResult.fcResult[i].dtResult.size, pHVCResult->fdResult.fcResult[i].dtResult.confidence);
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_FACE_DIRECTION){
                            /* Face Direction */
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Face Direction\tLR:%d UD:%d Roll:%d Confidence:%d",
                                        pHVCResult->fdResult.fcResult[i].dirResult.yaw, pHVCResult->fdResult.fcResult[i].dirResult.pitch,
                                        pHVCResult->fdResult.fcResult[i].dirResult.roll, pHVCResult->fdResult.fcResult[i].dirResult.confidence);
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION){
                            /* Age */
                            if(-128 == pHVCResult->fdResult.fcResult[i].ageResult.age){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tEstimation not possible");
                            } else {
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tAge:%d Confidence:%d",
                                            pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence);
                            }
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION){
                            /* Gender */
                            if(-128 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tEstimation not possible");
                            }
                            else{
                                if(1 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d",
                                                "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence);
                                }
                                else{
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d",
                                                "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence);
                                }
                            }
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_GAZE_ESTIMATION){
                            /* Gaze */
                            if((-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR) ||
                                (-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD)){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gaze\t\tEstimation not possible");
                            }
                            else{
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gaze\t\tLR:%d UD:%d",
                                            pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR, pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD);
                            }
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_BLINK_ESTIMATION){
                            /* Blink */
                            if((-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioL) ||
                                (-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioR)){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Blink\t\tEstimation not possible");
                            }
                            else{
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Blink\t\tLeft:%d Right:%d",
                                            pHVCResult->fdResult.fcResult[i].blinkResult.ratioL, pHVCResult->fdResult.fcResult[i].blinkResult.ratioR);
                            }
                        }
                        if(pHVCResult->executedFunc & HVC_ACTIV_EXPRESSION_ESTIMATION){
                            /* Expression */
                            if(-128 == pHVCResult->fdResult.fcResult[i].expressionResult.score[0]){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Expression\tEstimation not possible");
                            }
                            else{
                                if(pHVCResult->fdResult.fcResult[i].expressionResult.topExpression > EX_SADNESS){
                                    pHVCResult->fdResult.fcResult[i].expressionResult.topExpression = 0;
                                }
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Expression\tExpression:%s Score:%d, %d, %d, %d, %d Degree:%d",
                                            pExStr[pHVCResult->fdResult.fcResult[i].expressionResult.topExpression],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.score[0],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.score[1],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.score[2],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.score[3],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.score[4],
                                            pHVCResult->fdResult.fcResult[i].expressionResult.degree);
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

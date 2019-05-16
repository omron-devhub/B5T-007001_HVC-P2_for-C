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

/* 
    HVC Sample API
*/

#include <stdlib.h>
#include "HVCApi.h"
#include "HVCExtraUartFunc.h"

/*----------------------------------------------------------------------------*/
/* Command number                                                             */
/*----------------------------------------------------------------------------*/
#define HVC_COM_GET_VERSION             (UINT8)0x00
#define HVC_COM_SET_CAMERA_ANGLE        (UINT8)0x01
#define HVC_COM_GET_CAMERA_ANGLE        (UINT8)0x02
#define HVC_COM_EXECUTE                 (UINT8)0x03
#define HVC_COM_EXECUTEEX               (UINT8)0x04
#define HVC_COM_SET_THRESHOLD           (UINT8)0x05
#define HVC_COM_GET_THRESHOLD           (UINT8)0x06
#define HVC_COM_SET_SIZE_RANGE          (UINT8)0x07
#define HVC_COM_GET_SIZE_RANGE          (UINT8)0x08
#define HVC_COM_SET_DETECTION_ANGLE     (UINT8)0x09
#define HVC_COM_GET_DETECTION_ANGLE     (UINT8)0x0A
#define HVC_COM_SET_BAUDRATE            (UINT8)0x0E
#define HVC_COM_REGISTRATION            (UINT8)0x10
#define HVC_COM_DELETE_DATA             (UINT8)0x11
#define HVC_COM_DELETE_USER             (UINT8)0x12
#define HVC_COM_DELETE_ALL              (UINT8)0x13
#define HVC_COM_GET_PERSON_DATA         (UINT8)0x15
#define HVC_COM_SAVE_ALBUM              (UINT8)0x20
#define HVC_COM_LOAD_ALBUM              (UINT8)0x21
#define HVC_COM_WRITE_ALBUM             (UINT8)0x22

/*----------------------------------------------------------------------------*/
/* Header for send signal data */
typedef enum {
    SEND_HEAD_SYNCBYTE = 0,
    SEND_HEAD_COMMANDNO,
    SEND_HEAD_DATALENGTHLSB,
    SEND_HEAD_DATALENGTHMSB,
    SEND_HEAD_NUM
}SEND_HEADER;
/*----------------------------------------------------------------------------*/
/* Header for receive signal data */
typedef enum {
    RECEIVE_HEAD_SYNCBYTE = 0,
    RECEIVE_HEAD_STATUS,
    RECEIVE_HEAD_DATALENLL,
    RECEIVE_HEAD_DATALENLM,
    RECEIVE_HEAD_DATALENML,
    RECEIVE_HEAD_DATALENMM,
    RECEIVE_HEAD_NUM
}RECEIVE_HEADER;

/*----------------------------------------------------------------------------*/
/* Send command signal                                                        */
/* param    : UINT8         inCommandNo     command number                    */
/*          : INT32         inDataSize      sending signal data size          */
/*          : UINT8         *inData         sending signal data               */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -10...timeout error               */
/*----------------------------------------------------------------------------*/
static INT32 HVC_SendCommand(UINT8 inCommandNo, INT32 inDataSize, UINT8 *inData)
{
    INT32 i;
    INT32 ret = 0;
    UINT8 sendData[32];

    /* Create header */
    sendData[SEND_HEAD_SYNCBYTE]        = (UINT8)0xFE;
    sendData[SEND_HEAD_COMMANDNO]       = (UINT8)inCommandNo;
    sendData[SEND_HEAD_DATALENGTHLSB]   = (UINT8)(inDataSize&0xff);
    sendData[SEND_HEAD_DATALENGTHMSB]   = (UINT8)((inDataSize>>8)&0xff);

    for(i = 0; i < inDataSize; i++){
        sendData[SEND_HEAD_NUM + i] = inData[i];
    }

    /* Send command signal */
    ret = UART_SendData(SEND_HEAD_NUM+inDataSize, sendData);
    if(ret != SEND_HEAD_NUM+inDataSize){
        return HVC_ERROR_SEND_DATA;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* Send command signal of LoadAlbum                                           */
/* param    : UINT8         inCommandNo     command number                    */
/*          : INT32         inDataSize      sending signal data size          */
/*          : UINT8         *inData         sending signal data               */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -10...timeout error               */
/*----------------------------------------------------------------------------*/
static INT32 HVC_SendCommandOfLoadAlbum(UINT8 inCommandNo, INT32 inDataSize, UINT8 *inData)
{   
    INT32 i;
    INT32 ret = 0;
    UINT8 *pSendData = NULL;
    
    pSendData = (UINT8*)malloc(SEND_HEAD_NUM + 4 + inDataSize);

    /* Create header */
    pSendData[SEND_HEAD_SYNCBYTE]       = (UINT8)0xFE;
    pSendData[SEND_HEAD_COMMANDNO]      = (UINT8)inCommandNo;
    pSendData[SEND_HEAD_DATALENGTHLSB]  = (UINT8)4;
    pSendData[SEND_HEAD_DATALENGTHMSB]  = (UINT8)0;

    pSendData[SEND_HEAD_NUM + 0]        = (UINT8)(inDataSize & 0x000000ff);
    pSendData[SEND_HEAD_NUM + 1]        = (UINT8)((inDataSize >> 8) & 0x000000ff);
    pSendData[SEND_HEAD_NUM + 2]        = (UINT8)((inDataSize >> 16) & 0x000000ff);
    pSendData[SEND_HEAD_NUM + 3]        = (UINT8)((inDataSize >> 24) & 0x000000ff);

     for(i = 0; i < inDataSize; i++){
        pSendData[SEND_HEAD_NUM + 4 + i] = inData[i];
    }
     
    /* Send command signal */
    ret = UART_SendData(SEND_HEAD_NUM+4+inDataSize, pSendData);
    if(ret != SEND_HEAD_NUM + 4 + inDataSize){
        ret = HVC_ERROR_SEND_DATA;
    }
    else{
        ret = 0;
    }
    free(pSendData);

    return ret;
}

/*----------------------------------------------------------------------------*/
/* Receive header                                                             */
/* param    : INT32         inTimeOutTime   timeout time                      */
/*          : INT32         *outDataSize    receive signal data length        */
/*          : UINT8         *outStatus      status                            */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -20...timeout error               */
/*          :                               -21...invalid header error        */
/*----------------------------------------------------------------------------*/
static INT32 HVC_ReceiveHeader(INT32 inTimeOutTime, INT32 *outDataSize, UINT8 *outStatus)
{
    INT32 ret = 0;
    UINT8 headerData[32];

    /* Get header part */
    ret = UART_ReceiveData(inTimeOutTime, RECEIVE_HEAD_NUM, headerData);
    if(ret != RECEIVE_HEAD_NUM){
        return HVC_ERROR_HEADER_TIMEOUT;
    }
    else if((UINT8)0xFE != headerData[RECEIVE_HEAD_SYNCBYTE]){
        /* Different value indicates an invalid result */
        return HVC_ERROR_HEADER_INVALID;
    }

    /* Get data length */
    *outDataSize = headerData[RECEIVE_HEAD_DATALENLL] +
                    (headerData[RECEIVE_HEAD_DATALENLM]<<8) +
                    (headerData[RECEIVE_HEAD_DATALENML]<<16) +
                    (headerData[RECEIVE_HEAD_DATALENMM]<<24);

    /* Get command execution result */
    *outStatus  = headerData[RECEIVE_HEAD_STATUS];
    return 0;
}

/*----------------------------------------------------------------------------*/
/* Receive data                                                               */
/* param    : INT32         inTimeOutTime   timeout time                      */
/*          : INT32         inDataSize      receive signal data size          */
/*          : UINT8         *outResult      receive signal data               */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -20...timeout error               */
/*----------------------------------------------------------------------------*/
static INT32 HVC_ReceiveData(INT32 inTimeOutTime, INT32 inDataSize, UINT8 *outResult)
{
    INT32 ret = 0;

    if ( inDataSize <= 0 ) return 0;

    /* Receive data */
    ret = UART_ReceiveData(inTimeOutTime, inDataSize, outResult);
    if(ret != inDataSize){
        return HVC_ERROR_DATA_TIMEOUT;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_GetVersion                                                             */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : HVC_VERSION   *outVersion     version data                      */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_GetVersion(INT32 inTimeOutTime, HVC_VERSION *outVersion, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;

    if((NULL == outVersion) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    /* Send GetVersion command signal */
    ret = HVC_SendCommand(HVC_COM_GET_VERSION, 0, NULL);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    if ( size > (INT32)sizeof(HVC_VERSION) ) {
        size = sizeof(HVC_VERSION);
    }

    /* Receive data */
    return HVC_ReceiveData(inTimeOutTime, size, (UINT8*)outVersion);
}

/*----------------------------------------------------------------------------*/
/* HVC_SetCameraAngle                                                         */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inAngleNo       camera angle number               */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_SetCameraAngle(INT32 inTimeOutTime, INT32 inAngleNo, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[32];

    if(NULL == outStatus){
        return HVC_ERROR_PARAMETER;
    }

    sendData[0] = (UINT8)(inAngleNo&0xff);
    /* Send SetCameraAngle command signal */
    ret = HVC_SendCommand(HVC_COM_SET_CAMERA_ANGLE, sizeof(UINT8), sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_GetCameraAngle                                                         */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         *outAngleNo     camera angle number               */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_GetCameraAngle(INT32 inTimeOutTime, INT32 *outAngleNo, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 recvData[32];

    if((NULL == outAngleNo) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    /* Send GetCameraAngle command signal */
    ret = HVC_SendCommand(HVC_COM_GET_CAMERA_ANGLE, 0, NULL);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    if ( size > (INT32)sizeof(UINT8) ) {
        size = sizeof(UINT8);
    }

    /* Receive data */
    ret = HVC_ReceiveData(inTimeOutTime, size, recvData);
    *outAngleNo = recvData[0];
    return ret;
}

/*----------------------------------------------------------------------------*/
/* HVC_Execute                                                                */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inExec          executable function               */
/*          : INT32         inImage         image info                        */
/*          : HVC_RESULT    *outHVCResult   result data                       */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_Execute(INT32 inTimeOutTime, INT32 inExec, INT32 inImage, HVC_RESULT *outHVCResult, UINT8 *outStatus)
{
    int i;
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[32];
    UINT8 recvData[32];

    if((NULL == outHVCResult) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    /* Send Execute command signal */
    sendData[0] = (UINT8)(inExec&0xff);
    sendData[1] = (UINT8)((inExec>>8)&0xff);
    sendData[2] = (UINT8)(inImage&0xff);
    ret = HVC_SendCommand(HVC_COM_EXECUTE, sizeof(UINT8)*3, sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    /* Receive result data */
    if ( size >= (INT32)sizeof(UINT8)*4 ) {
        outHVCResult->executedFunc = inExec;
        ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, recvData);
        outHVCResult->bdResult.num = recvData[0];
        outHVCResult->hdResult.num = recvData[1];
        outHVCResult->fdResult.num = recvData[2];
        if ( ret != 0 ) return ret;
        size -= sizeof(UINT8)*4;
    }

    /* Get Human Body Detection result */
    for(i = 0; i < outHVCResult->bdResult.num; i++){
        if ( size >= (INT32)sizeof(UINT8)*8 ) {
            ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*8, recvData);
            outHVCResult->bdResult.bdResult[i].posX = (short)(recvData[0] + (recvData[1]<<8));
            outHVCResult->bdResult.bdResult[i].posY = (short)(recvData[2] + (recvData[3]<<8));
            outHVCResult->bdResult.bdResult[i].size = (short)(recvData[4] + (recvData[5]<<8));
            outHVCResult->bdResult.bdResult[i].confidence = (short)(recvData[6] + (recvData[7]<<8));
            if ( ret != 0 ) return ret;
            size -= sizeof(UINT8)*8;
        }
    }

    /* Get Hand Detection result */
    for(i = 0; i < outHVCResult->hdResult.num; i++){
        if ( size >= (INT32)sizeof(UINT8)*8 ) {
            ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*8, recvData);
            outHVCResult->hdResult.hdResult[i].posX = (short)(recvData[0] + (recvData[1]<<8));
            outHVCResult->hdResult.hdResult[i].posY = (short)(recvData[2] + (recvData[3]<<8));
            outHVCResult->hdResult.hdResult[i].size = (short)(recvData[4] + (recvData[5]<<8));
            outHVCResult->hdResult.hdResult[i].confidence = (short)(recvData[6] + (recvData[7]<<8));
            if ( ret != 0 ) return ret;
            size -= sizeof(UINT8)*8;
        }
    }

    /* Face-related results */
    for(i = 0; i < outHVCResult->fdResult.num; i++){
        /* Face Detection result */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION)){
            if ( size >= (INT32)sizeof(UINT8)*8 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*8, recvData);
                outHVCResult->fdResult.fcResult[i].dtResult.posX = (short)(recvData[0] + (recvData[1]<<8));
                outHVCResult->fdResult.fcResult[i].dtResult.posY = (short)(recvData[2] + (recvData[3]<<8));
                outHVCResult->fdResult.fcResult[i].dtResult.size = (short)(recvData[4] + (recvData[5]<<8));
                outHVCResult->fdResult.fcResult[i].dtResult.confidence = (short)(recvData[6] + (recvData[7]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*8;
            }
        }

        /* Face direction */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_FACE_DIRECTION)){
            if ( size >= (INT32)sizeof(UINT8)*8 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*8, recvData);
                outHVCResult->fdResult.fcResult[i].dirResult.yaw = (short)(recvData[0] + (recvData[1]<<8));
                outHVCResult->fdResult.fcResult[i].dirResult.pitch = (short)(recvData[2] + (recvData[3]<<8));
                outHVCResult->fdResult.fcResult[i].dirResult.roll = (short)(recvData[4] + (recvData[5]<<8));
                outHVCResult->fdResult.fcResult[i].dirResult.confidence = (short)(recvData[6] + (recvData[7]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*8;
            }
        }

        /* Age */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION)){
            if ( size >= (INT32)sizeof(UINT8)*3 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*3, recvData);
                outHVCResult->fdResult.fcResult[i].ageResult.age = (char)(recvData[0]);
                outHVCResult->fdResult.fcResult[i].ageResult.confidence = (short)(recvData[1] + (recvData[2]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*3;
            }
        }

        /* Gender */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION)){
            if ( size >= (INT32)sizeof(UINT8)*3 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*3, recvData);
                outHVCResult->fdResult.fcResult[i].genderResult.gender = (char)(recvData[0]);
                outHVCResult->fdResult.fcResult[i].genderResult.confidence = (short)(recvData[1] + (recvData[2]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*3;
            }
        }

        /* Gaze */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_GAZE_ESTIMATION)){
            if ( size >= (INT32)sizeof(UINT8)*2 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*2, recvData);
                outHVCResult->fdResult.fcResult[i].gazeResult.gazeLR = (char)(recvData[0]);
                outHVCResult->fdResult.fcResult[i].gazeResult.gazeUD = (char)(recvData[1]);
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*2;
            }
        }

        /* Blink */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_BLINK_ESTIMATION)){
            if ( size >= (INT32)sizeof(UINT8)*4 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, recvData);
                outHVCResult->fdResult.fcResult[i].blinkResult.ratioL = (short)(recvData[0] + (recvData[1]<<8));
                outHVCResult->fdResult.fcResult[i].blinkResult.ratioR = (short)(recvData[2] + (recvData[3]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*4;
            }
        }

        /* Expression */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_EXPRESSION_ESTIMATION)){
            if ( size >= (INT32)sizeof(UINT8)*3 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*3, recvData);
                outHVCResult->fdResult.fcResult[i].expressionResult.topExpression = (char)(recvData[0]);
                outHVCResult->fdResult.fcResult[i].expressionResult.topScore = (char)(recvData[1]);
                outHVCResult->fdResult.fcResult[i].expressionResult.degree = (char)(recvData[2]);
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*3;
            }
        }

        /* Face Recognition */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_FACE_RECOGNITION)){
            if ( size >= (INT32)sizeof(UINT8)*4 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, recvData);
                outHVCResult->fdResult.fcResult[i].recognitionResult.uid = (short)(recvData[0] + (recvData[1]<<8));
                outHVCResult->fdResult.fcResult[i].recognitionResult.confidence = (short)(recvData[2] + (recvData[3]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*4;
            }
        }
    }

    if(HVC_EXECUTE_IMAGE_NONE != inImage){
        /* Image data */
        if ( size >= (INT32)sizeof(UINT8)*4 ) {
            ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, recvData);
            outHVCResult->image.width = (short)(recvData[0] + (recvData[1]<<8));
            outHVCResult->image.height = (short)(recvData[2] + (recvData[3]<<8));
            if ( ret != 0 ) return ret;
            size -= sizeof(UINT8)*4;
        }

        if ( size >= (INT32)sizeof(UINT8)*outHVCResult->image.width*outHVCResult->image.height ) {
            ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*outHVCResult->image.width*outHVCResult->image.height, outHVCResult->image.image);
            if ( ret != 0 ) return ret;
            size -= sizeof(UINT8)*outHVCResult->image.width*outHVCResult->image.height;
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_ExecuteEx                                                              */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inExec          executable function               */
/*          : INT32         inImage         image info                        */
/*          : HVC_RESULT    *outHVCResult   result data                       */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_ExecuteEx(INT32 inTimeOutTime, INT32 inExec, INT32 inImage, HVC_RESULT *outHVCResult, UINT8 *outStatus)
{
    int i, j;
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[32];
    UINT8 recvData[32];

    if((NULL == outHVCResult) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    /* Send Execute command signal */
    sendData[0] = (UINT8)(inExec&0xff);
    sendData[1] = (UINT8)((inExec>>8)&0xff);
    sendData[2] = (UINT8)(inImage&0xff);
    ret = HVC_SendCommand(HVC_COM_EXECUTEEX, sizeof(UINT8)*3, sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    /* Receive result data */
    if ( size >= (INT32)sizeof(UINT8)*4 ) {
        outHVCResult->executedFunc = inExec;
        ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, recvData);
        outHVCResult->bdResult.num = recvData[0];
        outHVCResult->hdResult.num = recvData[1];
        outHVCResult->fdResult.num = recvData[2];
        if ( ret != 0 ) return ret;
        size -= sizeof(UINT8)*4;
    }

    /* Get Human Body Detection result */
    for(i = 0; i < outHVCResult->bdResult.num; i++){
        if ( size >= (INT32)sizeof(UINT8)*8 ) {
            ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*8, recvData);
            outHVCResult->bdResult.bdResult[i].posX = (short)(recvData[0] + (recvData[1]<<8));
            outHVCResult->bdResult.bdResult[i].posY = (short)(recvData[2] + (recvData[3]<<8));
            outHVCResult->bdResult.bdResult[i].size = (short)(recvData[4] + (recvData[5]<<8));
            outHVCResult->bdResult.bdResult[i].confidence = (short)(recvData[6] + (recvData[7]<<8));
            if ( ret != 0 ) return ret;
            size -= sizeof(UINT8)*8;
        }
    }

    /* Get Hand Detection result */
    for(i = 0; i < outHVCResult->hdResult.num; i++){
        if ( size >= (INT32)sizeof(UINT8)*8 ) {
            ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*8, recvData);
            outHVCResult->hdResult.hdResult[i].posX = (short)(recvData[0] + (recvData[1]<<8));
            outHVCResult->hdResult.hdResult[i].posY = (short)(recvData[2] + (recvData[3]<<8));
            outHVCResult->hdResult.hdResult[i].size = (short)(recvData[4] + (recvData[5]<<8));
            outHVCResult->hdResult.hdResult[i].confidence = (short)(recvData[6] + (recvData[7]<<8));
            if ( ret != 0 ) return ret;
            size -= sizeof(UINT8)*8;
        }
    }

    /* Face-related results */
    for(i = 0; i < outHVCResult->fdResult.num; i++){
        /* Face Detection result */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION)){
            if ( size >= (INT32)sizeof(UINT8)*8 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*8, recvData);
                outHVCResult->fdResult.fcResult[i].dtResult.posX = (short)(recvData[0] + (recvData[1]<<8));
                outHVCResult->fdResult.fcResult[i].dtResult.posY = (short)(recvData[2] + (recvData[3]<<8));
                outHVCResult->fdResult.fcResult[i].dtResult.size = (short)(recvData[4] + (recvData[5]<<8));
                outHVCResult->fdResult.fcResult[i].dtResult.confidence = (short)(recvData[6] + (recvData[7]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*8;
            }
        }

        /* Face direction */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_FACE_DIRECTION)){
            if ( size >= (INT32)sizeof(UINT8)*8 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*8, recvData);
                outHVCResult->fdResult.fcResult[i].dirResult.yaw = (short)(recvData[0] + (recvData[1]<<8));
                outHVCResult->fdResult.fcResult[i].dirResult.pitch = (short)(recvData[2] + (recvData[3]<<8));
                outHVCResult->fdResult.fcResult[i].dirResult.roll = (short)(recvData[4] + (recvData[5]<<8));
                outHVCResult->fdResult.fcResult[i].dirResult.confidence = (short)(recvData[6] + (recvData[7]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*8;
            }
        }

        /* Age */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION)){
            if ( size >= (INT32)sizeof(UINT8)*3 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*3, recvData);
                outHVCResult->fdResult.fcResult[i].ageResult.age = (char)(recvData[0]);
                outHVCResult->fdResult.fcResult[i].ageResult.confidence = (short)(recvData[1] + (recvData[2]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*3;
            }
        }

        /* Gender */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION)){
            if ( size >= (INT32)sizeof(UINT8)*3 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*3, recvData);
                outHVCResult->fdResult.fcResult[i].genderResult.gender = (char)(recvData[0]);
                outHVCResult->fdResult.fcResult[i].genderResult.confidence = (short)(recvData[1] + (recvData[2]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*3;
            }
        }

        /* Gaze */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_GAZE_ESTIMATION)){
            if ( size >= (INT32)sizeof(UINT8)*2 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*2, recvData);
                outHVCResult->fdResult.fcResult[i].gazeResult.gazeLR = (char)(recvData[0]);
                outHVCResult->fdResult.fcResult[i].gazeResult.gazeUD = (char)(recvData[1]);
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*2;
            }
        }

        /* Blink */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_BLINK_ESTIMATION)){
            if ( size >= (INT32)sizeof(UINT8)*4 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, recvData);
                outHVCResult->fdResult.fcResult[i].blinkResult.ratioL = (short)(recvData[0] + (recvData[1]<<8));
                outHVCResult->fdResult.fcResult[i].blinkResult.ratioR = (short)(recvData[2] + (recvData[3]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*4;
            }
        }

        /* Expression */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_EXPRESSION_ESTIMATION)){
            if ( size >= (INT32)sizeof(UINT8)*6 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*6, recvData);
                outHVCResult->fdResult.fcResult[i].expressionResult.topExpression = -128;
                outHVCResult->fdResult.fcResult[i].expressionResult.topScore = -128;
                for(j = 0; j < 5; j++){
                    outHVCResult->fdResult.fcResult[i].expressionResult.score[j] = (char)(recvData[j]);
                    if(outHVCResult->fdResult.fcResult[i].expressionResult.topScore < outHVCResult->fdResult.fcResult[i].expressionResult.score[j]){
                        outHVCResult->fdResult.fcResult[i].expressionResult.topScore = outHVCResult->fdResult.fcResult[i].expressionResult.score[j];
                        outHVCResult->fdResult.fcResult[i].expressionResult.topExpression = j + 1;
                    }
                }
                outHVCResult->fdResult.fcResult[i].expressionResult.degree = (char)(recvData[5]);
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*6;
            }
        }

        /* Face Recognition */
        if(0 != (outHVCResult->executedFunc & HVC_ACTIV_FACE_RECOGNITION)){
            if ( size >= (INT32)sizeof(UINT8)*4 ) {
                ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, recvData);
                outHVCResult->fdResult.fcResult[i].recognitionResult.uid = (short)(recvData[0] + (recvData[1]<<8));
                outHVCResult->fdResult.fcResult[i].recognitionResult.confidence = (short)(recvData[2] + (recvData[3]<<8));
                if ( ret != 0 ) return ret;
                size -= sizeof(UINT8)*4;
            }
        }
    }

    if(HVC_EXECUTE_IMAGE_NONE != inImage){
        /* Image data */
        if ( size >= (INT32)sizeof(UINT8)*4 ) {
            ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, recvData);
            outHVCResult->image.width = (short)(recvData[0] + (recvData[1]<<8));
            outHVCResult->image.height = (short)(recvData[2] + (recvData[3]<<8));
            if ( ret != 0 ) return ret;
            size -= sizeof(UINT8)*4;
        }

        if ( size >= (INT32)sizeof(UINT8)*outHVCResult->image.width*outHVCResult->image.height ) {
            ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*outHVCResult->image.width*outHVCResult->image.height, outHVCResult->image.image);
            if ( ret != 0 ) return ret;
            size -= sizeof(UINT8)*outHVCResult->image.width*outHVCResult->image.height;
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_SetThreshold                                                           */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : HVC_THRESHOLD *inThreshold    threshold values                  */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_SetThreshold(INT32 inTimeOutTime, HVC_THRESHOLD *inThreshold, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[32];

    if((NULL == inThreshold) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    sendData[0] = (UINT8)(inThreshold->bdThreshold&0xff);
    sendData[1] = (UINT8)((inThreshold->bdThreshold>>8)&0xff);
    sendData[2] = (UINT8)(inThreshold->hdThreshold&0xff);
    sendData[3] = (UINT8)((inThreshold->hdThreshold>>8)&0xff);
    sendData[4] = (UINT8)(inThreshold->dtThreshold&0xff);
    sendData[5] = (UINT8)((inThreshold->dtThreshold>>8)&0xff);
    sendData[6] = (UINT8)(inThreshold->rsThreshold&0xff);
    sendData[7] = (UINT8)((inThreshold->rsThreshold>>8)&0xff);
    /* Send SetThreshold command signal */
    ret = HVC_SendCommand(HVC_COM_SET_THRESHOLD, sizeof(UINT8)*8, sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_GetThreshold                                                           */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : HVC_THRESHOLD *outThreshold   threshold values                  */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_GetThreshold(INT32 inTimeOutTime, HVC_THRESHOLD *outThreshold, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 recvData[32];

    if((NULL == outThreshold) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    /* Send GetThreshold command signal */
    ret = HVC_SendCommand(HVC_COM_GET_THRESHOLD, 0, NULL);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    if ( size > (INT32)sizeof(UINT8)*8 ) {
        size = sizeof(UINT8)*8;
    }

    /* Receive data */
    ret = HVC_ReceiveData(inTimeOutTime, size, recvData);
    outThreshold->bdThreshold = recvData[0] + (recvData[1]<<8);
    outThreshold->hdThreshold = recvData[2] + (recvData[3]<<8);
    outThreshold->dtThreshold = recvData[4] + (recvData[5]<<8);
    outThreshold->rsThreshold = recvData[6] + (recvData[7]<<8);
    return ret;
}

/*----------------------------------------------------------------------------*/
/* HVC_SetSizeRange                                                           */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : HVC_SIZERANGE *inSizeRange    detection sizes                   */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_SetSizeRange(INT32 inTimeOutTime, HVC_SIZERANGE *inSizeRange, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[32];

    if((NULL == inSizeRange) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    sendData[0] = (UINT8)(inSizeRange->bdMinSize&0xff);
    sendData[1] = (UINT8)((inSizeRange->bdMinSize>>8)&0xff);
    sendData[2] = (UINT8)(inSizeRange->bdMaxSize&0xff);
    sendData[3] = (UINT8)((inSizeRange->bdMaxSize>>8)&0xff);
    sendData[4] = (UINT8)(inSizeRange->hdMinSize&0xff);
    sendData[5] = (UINT8)((inSizeRange->hdMinSize>>8)&0xff);
    sendData[6] = (UINT8)(inSizeRange->hdMaxSize&0xff);
    sendData[7] = (UINT8)((inSizeRange->hdMaxSize>>8)&0xff);
    sendData[8] = (UINT8)(inSizeRange->dtMinSize&0xff);
    sendData[9] = (UINT8)((inSizeRange->dtMinSize>>8)&0xff);
    sendData[10] = (UINT8)(inSizeRange->dtMaxSize&0xff);
    sendData[11] = (UINT8)((inSizeRange->dtMaxSize>>8)&0xff);
    /* Send SetSizeRange command signal */
    ret = HVC_SendCommand(HVC_COM_SET_SIZE_RANGE, sizeof(UINT8)*12, sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_GetSizeRange                                                           */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : HVC_SIZERANGE *outSizeRange   detection sizes                   */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_GetSizeRange(INT32 inTimeOutTime, HVC_SIZERANGE *outSizeRange, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 recvData[32];

    if((NULL == outSizeRange) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    /* Send GetSizeRange command signal */
    ret = HVC_SendCommand(HVC_COM_GET_SIZE_RANGE, 0, NULL);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    if ( size > (INT32)sizeof(UINT8)*12 ) {
        size = sizeof(UINT8)*12;
    }

    /* Receive data */
    ret = HVC_ReceiveData(inTimeOutTime, size, recvData);
    outSizeRange->bdMinSize = recvData[0] + (recvData[1]<<8);
    outSizeRange->bdMaxSize = recvData[2] + (recvData[3]<<8);
    outSizeRange->hdMinSize = recvData[4] + (recvData[5]<<8);
    outSizeRange->hdMaxSize = recvData[6] + (recvData[7]<<8);
    outSizeRange->dtMinSize = recvData[8] + (recvData[9]<<8);
    outSizeRange->dtMaxSize = recvData[10] + (recvData[11]<<8);
    return ret;
}

/*----------------------------------------------------------------------------*/
/* HVC_SetFaceDetectionAngle                                                  */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inPose          Yaw angle range                   */
/*          : INT32         inAngle         Roll angle range                  */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_SetFaceDetectionAngle(INT32 inTimeOutTime, INT32 inPose, INT32 inAngle, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[32];

    if(NULL == outStatus){
        return HVC_ERROR_PARAMETER;
    }

    sendData[0] = (UINT8)(inPose&0xff);
    sendData[1] = (UINT8)(inAngle&0xff);
    /* Send SetFaceDetectionAngle command signal */
    ret = HVC_SendCommand(HVC_COM_SET_DETECTION_ANGLE, sizeof(UINT8)*2, sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_GetFaceDetectionAngle                                                  */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         *outPose        Yaw angle range                   */
/*          : INT32         *outAngle       Roll angle range                  */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_GetFaceDetectionAngle(INT32 inTimeOutTime, INT32 *outPose, INT32 *outAngle, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 recvData[32];

    if((NULL == outPose) || (NULL == outAngle) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    /* Send GetFaceDetectionAngle signal command */
    ret = HVC_SendCommand(HVC_COM_GET_DETECTION_ANGLE, 0, NULL);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    if ( size > (INT32)sizeof(UINT8)*2 ) {
        size = sizeof(UINT8)*2;
    }

    /* Receive data */
    ret = HVC_ReceiveData(inTimeOutTime, size, recvData);
    *outPose = recvData[0];
    *outAngle = recvData[1];
    return ret;
}

/*----------------------------------------------------------------------------*/
/* HVC_SetBaudRate                                                            */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inRate          Baudrate                          */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_SetBaudRate(INT32 inTimeOutTime, INT32 inRate, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[32];

    if(NULL == outStatus){
        return HVC_ERROR_PARAMETER;
    }

    sendData[0] = (UINT8)(inRate&0xff);
    /* Send SetBaudRate command signal */
    ret = HVC_SendCommand(HVC_COM_SET_BAUDRATE, sizeof(UINT8), sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_Registration                                                           */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inUserID        User ID (0-499)                   */
/*          : INT32         inDataID        Data ID (0-9)                     */
/*          : HVC_IMAGE     *outImage       image info                        */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_Registration(INT32 inTimeOutTime, INT32 inUserID, INT32 inDataID, HVC_IMAGE *outImage, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[32];
    UINT8 recvData[32];

    if((NULL == outImage) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    /* Send Registration signal command */
    sendData[0] = (UINT8)(inUserID&0xff);
    sendData[1] = (UINT8)((inUserID>>8)&0xff);
    sendData[2] = (UINT8)(inDataID&0xff);
    ret = HVC_SendCommand(HVC_COM_REGISTRATION, sizeof(UINT8)*3, sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    /* Receive data */
    if ( size >= (INT32)sizeof(UINT8)*4 ) {
        ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, recvData);
        outImage->width = recvData[0] + (recvData[1]<<8);
        outImage->height = recvData[2] + (recvData[3]<<8);
        if ( ret != 0 ) return ret;
        size -= sizeof(UINT8)*4;
    }

    /* Image data */
    if ( size >= (INT32)sizeof(UINT8)*64*64 ) {
        ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*64*64, outImage->image);
        if ( ret != 0 ) return ret;
        size -= sizeof(UINT8)*64*64;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_DeleteData                                                             */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inUserID        User ID (0-499)                   */
/*          : INT32         inDataID        Data ID (0-9)                     */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_DeleteData(INT32 inTimeOutTime, INT32 inUserID, INT32 inDataID, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[32];

    if(NULL == outStatus){
        return HVC_ERROR_PARAMETER;
    }

    /* Send Delete Data signal command */
    sendData[0] = (UINT8)(inUserID&0xff);
    sendData[1] = (UINT8)((inUserID>>8)&0xff);
    sendData[2] = (UINT8)(inDataID&0xff);
    ret = HVC_SendCommand(HVC_COM_DELETE_DATA, sizeof(UINT8)*3, sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_DeleteUser                                                             */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inUserID        User ID (0-499)                   */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_DeleteUser(INT32 inTimeOutTime, INT32 inUserID, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[32];

    if(NULL == outStatus){
        return HVC_ERROR_PARAMETER;
    }

    /* Send Delete User signal command */
    sendData[0] = (UINT8)(inUserID&0xff);
    sendData[1] = (UINT8)((inUserID>>8)&0xff);
    ret = HVC_SendCommand(HVC_COM_DELETE_USER, sizeof(UINT8)*2, sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_DeleteAll                                                              */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_DeleteAll(INT32 inTimeOutTime, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;

    if(NULL == outStatus){
        return HVC_ERROR_PARAMETER;
    }

    /* Send Delete All signal command */
    ret = HVC_SendCommand(HVC_COM_DELETE_ALL, 0, NULL);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;
    return 0;
}

/*----------------------------------------------------------------------------*/
/* HVC_GetUserData                                                            */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inUserID        User ID (0-499)                   */
/*          : INT32         *outDataNo      Registration Info                 */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_GetUserData(INT32 inTimeOutTime, INT32 inUserID, INT32 *outDataNo, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
    UINT8 sendData[8];
    UINT8 recvData[8];

    if((NULL == outDataNo) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }

    /* Send Get Registration Info signal command */
    sendData[0] = (UINT8)(inUserID&0xff);
    sendData[1] = (UINT8)((inUserID>>8)&0xff);
    ret = HVC_SendCommand(HVC_COM_GET_PERSON_DATA, sizeof(UINT8)*2, sendData);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    if ( size > (INT32)sizeof(UINT8)*2 ) {
        size = sizeof(UINT8)*2;
    }

    /* Receive data */
    ret = HVC_ReceiveData(inTimeOutTime, size, recvData);
    *outDataNo = recvData[0] + (recvData[1]<<8);
    return ret;
}

/*----------------------------------------------------------------------------*/
/* HVC_SaveAlbum                                                              */
/* param    : INT32         inTimeOutTime       timeout time (ms)             */
/*          : UINT8         *outAlbumData       Album data                    */
/*          : INT32         *outAlbumDataSize   Album data size               */
/*          : UINT8         *outStatus          response code                 */
/* return   : INT32                             execution result error code   */
/*          :                                   0...normal                    */
/*          :                                   -1...parameter error          */
/*          :                                   other...signal error          */
/*----------------------------------------------------------------------------*/
INT32 HVC_SaveAlbum(INT32 inTimeOutTime, UINT8 *outAlbumData, INT32 *outAlbumDataSize, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;

    UINT8 *tmpAlbumData = NULL;;
    
    if((NULL == outAlbumData) || (NULL == outAlbumDataSize) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }
        
    /* Send Save Album signal command */
    ret = HVC_SendCommand(HVC_COM_SAVE_ALBUM, 0, NULL);
    if ( ret != 0 ) return ret;

    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    if ( size >= (INT32)sizeof(UINT8)*8 + HVC_ALBUM_SIZE_MIN ) {
        *outAlbumDataSize = size;
        tmpAlbumData = outAlbumData;

        do{
            ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, tmpAlbumData);
            if ( ret != 0 ) return ret;
            tmpAlbumData += sizeof(UINT8)*4;

            ret = HVC_ReceiveData(inTimeOutTime, sizeof(UINT8)*4, tmpAlbumData);
            if ( ret != 0 ) return ret;
            tmpAlbumData += sizeof(UINT8)*4;

            ret = HVC_ReceiveData(inTimeOutTime, size - sizeof(UINT8)*8, tmpAlbumData);
            if ( ret != 0 ) return ret;
        }while(0);
    }
    return ret;
}


/*----------------------------------------------------------------------------*/
/* HVC_LoadAlbum                                                              */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : UINT8         *inAlbumData    Album data                        */
/*          : INT32         inAlbumDataSize Album data size                   */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_LoadAlbum(INT32 inTimeOutTime, UINT8 *inAlbumData, INT32 inAlbumDataSize, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;
        
    if((NULL == inAlbumData) || (NULL == outStatus)){
        return HVC_ERROR_PARAMETER;
    }
        
    /* Send Save Album signal command */
    ret = HVC_SendCommandOfLoadAlbum(HVC_COM_LOAD_ALBUM, inAlbumDataSize, inAlbumData);
    if ( ret != 0 ) return ret;
    
    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    return ret;
}

/*----------------------------------------------------------------------------*/
/* HVC_WriteAlbum                                                             */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : UINT8         *outStatus      response code                     */
/* return   : INT32                         execution result error code       */
/*          :                               0...normal                        */
/*          :                               -1...parameter error              */
/*          :                               other...signal error              */
/*----------------------------------------------------------------------------*/
INT32 HVC_WriteAlbum(INT32 inTimeOutTime, UINT8 *outStatus)
{
    INT32 ret = 0;
    INT32 size = 0;

    if(NULL == outStatus){
        return HVC_ERROR_PARAMETER;
    }

    /* Send Write Album signal command */
    ret = HVC_SendCommand(HVC_COM_WRITE_ALBUM, 0, NULL);
    if ( ret != 0 ) return ret;

    /* Receive header */
    ret = HVC_ReceiveHeader(inTimeOutTime, &size, outStatus);
    if ( ret != 0 ) return ret;

    return ret;
}


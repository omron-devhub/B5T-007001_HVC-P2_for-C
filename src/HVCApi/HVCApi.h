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

#ifndef HVCApi_H__
#define HVCApi_H__

#ifndef UINT8
typedef     unsigned char       UINT8;      /*  8 bit Unsigned Integer  */
#endif /* UINT8 */
#ifndef INT32
typedef     int                 INT32;      /* 32 bit Signed   Integer  */
#endif /* INT32 */
#ifndef     NULL
    #define     NULL                0
#endif

#include "HVCDef.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* HVC_GetVersion                                                             */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : HVC_VERSION   *outVersion     version data                      */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_GetVersion(INT32 inTimeOutTime, HVC_VERSION *outVersion, UINT8 *outStatus);

/* HVC_SetCameraAngle                                                         */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inAngleNo       camera angle number               */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_SetCameraAngle(INT32 inTimeOutTime, INT32 inAngleNo, UINT8 *outStatus);

/* HVC_GetCameraAngle                                                         */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         *outAngleNo     camera angle number               */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_GetCameraAngle(INT32 inTimeOutTime, INT32 *outAngleNo, UINT8 *outStatus);

/* HVC_Execute                                                                */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inExec          executable function               */
/*          : INT32         inImage         image output number               */
/*          : HVC_RESULT    *outHVCResult   result data                       */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_Execute(INT32 inTimeOutTime, INT32 inExec, INT32 inImage, HVC_RESULT *outHVCResult, UINT8 *outStatus);

/* HVC_ExecuteEx                                                              */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inExec          executable function               */
/*          : INT32         inImage         image output number               */
/*          : HVC_RESULT    *outHVCResult   result data                       */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_ExecuteEx(INT32 inTimeOutTime, INT32 inExec, INT32 inImage, HVC_RESULT *outHVCResult, UINT8 *outStatus);

/* HVC_SetThreshold                                                           */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : HVC_THRESHOLD *inThreshold    threshold values                  */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_SetThreshold(INT32 inTimeOutTime, HVC_THRESHOLD *inThreshold, UINT8 *outStatus);

/* HVC_GetThreshold                                                           */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : HVC_THRESHOLD *outThreshold   threshold values                  */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_GetThreshold(INT32 inTimeOutTime, HVC_THRESHOLD *outThreshold, UINT8 *outStatus);

/* HVC_SetSizeRange                                                           */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : HVC_SIZERANGE *inSizeRange    detection sizes                   */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_SetSizeRange(INT32 inTimeOutTime, HVC_SIZERANGE *inSizeRange, UINT8 *outStatus);

/* HVC_GetSizeRange                                                           */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : HVC_SIZERANGE *outSizeRange   detection sizes                   */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_GetSizeRange(INT32 inTimeOutTime, HVC_SIZERANGE *outSizeRange, UINT8 *outStatus);

/* HVC_SetFaceDetectionAngle                                                  */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inPose          Yaw angle range                   */
/*          : INT32         inAngle         Roll angle range                  */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_SetFaceDetectionAngle(INT32 inTimeOutTime, INT32 inPose, INT32 inAngle, UINT8 *outStatus);

/* HVC_GetFaceDetectionAngle                                                  */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         *outPose        Yaw angle range                   */
/*          : INT32         *outAngle       Roll angle range                  */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_GetFaceDetectionAngle(INT32 inTimeOutTime, INT32 *outPose, INT32 *outAngle, UINT8 *outStatus);

/* HVC_SetBaudRate                                                            */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inRate          Baudrate                          */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_SetBaudRate(INT32 inTimeOutTime, INT32 inRate, UINT8 *outStatus);

/* HVC_Registration                                                           */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inUserID        User ID (0-499)                   */
/*          : INT32         inDataID        Data ID (0-9)                     */
/*          : HVC_IMAGE     *outImage       image info                        */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_Registration(INT32 inTimeOutTime, INT32 inUserID, INT32 inDataID, HVC_IMAGE *outImage, UINT8 *outStatus);

/* HVC_DeleteData                                                             */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inUserID        User ID (0-499)                   */
/*          : INT32         inDataID        Data ID (0-9)                     */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_DeleteData(INT32 inTimeOutTime, INT32 inUserID, INT32 inDataID, UINT8 *outStatus);

/* HVC_DeleteUser                                                             */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inUserID        User ID (0-499)                   */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_DeleteUser(INT32 inTimeOutTime, INT32 inUserID, UINT8 *outStatus);

/* HVC_DeleteAll                                                              */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_DeleteAll(INT32 inTimeOutTime, UINT8 *outStatus);

/* HVC_GetUserData                                                            */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : INT32         inUserID        User ID (0-499)                   */
/*          : INT32         *outDataNo      Registration Info                 */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_GetUserData(INT32 inTimeOutTime, INT32 inUserID, INT32 *outDataNo, UINT8 *outStatus);

/* HVC_SaveAlbum                                                              */
/* param    : INT32         inTimeOutTime       timeout time (ms)             */
/*          : UINT8         *outAlbumData       Album data                    */
/*          : INT32         *outAlbumDataSize   Album data size               */
/*          : UINT8         *outStatus          response code                 */
INT32 HVC_SaveAlbum(INT32 inTimeOutTime, UINT8 *outAlbumData, INT32 *outAlbumDataSize, UINT8 *outStatus);

/* HVC_LoadAlbum                                                              */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : UINT8         *inAlbumData    Album data                        */
/*          : INT32         inAlbumDataSize Album data size                   */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_LoadAlbum(INT32 inTimeOutTime, UINT8 *inAlbumData, INT32 inAlbumDataSize, UINT8 *outStatus);

/* HVC_WriteAlbum                                                             */
/* param    : INT32         inTimeOutTime   timeout time (ms)                 */
/*          : UINT8         *outStatus      response code                     */
INT32 HVC_WriteAlbum(INT32 inTimeOutTime, UINT8 *outStatus);

#ifdef  __cplusplus
}
#endif

#endif  /* HVCApi_H__ */

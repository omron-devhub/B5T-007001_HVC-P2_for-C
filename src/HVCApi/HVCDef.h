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

#ifndef HVCDef_H__
#define HVCDef_H__

/*----------------------------------------------------------------------------*/
/* Execution flag */
#define HVC_ACTIV_BODY_DETECTION        0x00000001
#define HVC_ACTIV_HAND_DETECTION        0x00000002
#define HVC_ACTIV_FACE_DETECTION        0x00000004
#define HVC_ACTIV_FACE_DIRECTION        0x00000008
#define HVC_ACTIV_AGE_ESTIMATION        0x00000010
#define HVC_ACTIV_GENDER_ESTIMATION     0x00000020
#define HVC_ACTIV_GAZE_ESTIMATION       0x00000040
#define HVC_ACTIV_BLINK_ESTIMATION      0x00000080
#define HVC_ACTIV_EXPRESSION_ESTIMATION 0x00000100
#define HVC_ACTIV_FACE_RECOGNITION      0x00000200

/* Image info of Execute command */
#define HVC_EXECUTE_IMAGE_NONE          0x00000000
#define HVC_EXECUTE_IMAGE_QVGA          0x00000001
#define HVC_EXECUTE_IMAGE_QVGA_HALF     0x00000002

/*----------------------------------------------------------------------------*/
/* Error code */

/* Parameter error */
#define HVC_ERROR_PARAMETER             -1

/* Send signal timeout error */
#define HVC_ERROR_SEND_DATA             -10

/* Receive header signal timeout error */
#define HVC_ERROR_HEADER_TIMEOUT        -20
/* Invalid header error */
#define HVC_ERROR_HEADER_INVALID        -21
/* Receive data signal timeout error */
#define HVC_ERROR_DATA_TIMEOUT          -22


/*----------------------------------------------------------------------------*/
/* Album data size */
#define HVC_ALBUM_SIZE_MIN              32
#define HVC_ALBUM_SIZE_MAX              816032

/*----------------------------------------------------------------------------*/
/* Expression */
typedef enum {
    EX_NEUTRAL = 1,
    EX_HAPPINESS,
    EX_SURPRISE,
    EX_ANGER,
    EX_SADNESS
}EXPRESSION;

/*----------------------------------------------------------------------------*/
/* Struct                                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* DeviceÅfs model and version info                                           */
/*----------------------------------------------------------------------------*/
typedef struct {
    UINT8   string[12];
    UINT8   major;
    UINT8   minor;
    UINT8   relese;
    UINT8   revision[4];
}HVC_VERSION;

/*----------------------------------------------------------------------------*/
/* Detection result                                                           */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   posX;       /* Center x-coordinate */
    INT32   posY;       /* Center y-coordinate */
    INT32   size;       /* Size */
    INT32   confidence; /* Degree of confidence */
}DETECT_RESULT;

/*----------------------------------------------------------------------------*/
/* Face direction                                                             */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   yaw;        /* Yaw angle */
    INT32   pitch;      /* Pitch angle */
    INT32   roll;       /* Roll angle */
    INT32   confidence; /* Degree of confidence */
}DIR_RESULT;

/*----------------------------------------------------------------------------*/
/* Age                                                                        */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   age;        /* Age */
    INT32   confidence; /* Degree of confidence */
}AGE_RESULT;

/*----------------------------------------------------------------------------*/
/* Gender                                                                     */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   gender;     /* Gender */
    INT32   confidence; /* Degree of confidence */
}GENDER_RESULT;

/*----------------------------------------------------------------------------*/
/* Gaze                                                                       */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   gazeLR;     /* Yaw angle */
    INT32   gazeUD;     /* Pitch angle */
}GAZE_RESULT;

/*----------------------------------------------------------------------------*/
/* Blink                                                                      */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   ratioL;     /* Left eye blink result */
    INT32   ratioR;     /* Right eye blink result */
}BLINK_RESULT;

/*----------------------------------------------------------------------------*/
/* Expression                                                                 */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   topExpression;  /* Top expression */
    INT32   topScore;       /* Top score */
    INT32   score[5];       /* Score of 5 expression */
    INT32   degree;         /* Negative-positive degree */
}EXPRESSION_RESULT;

/*----------------------------------------------------------------------------*/
/* Face Recognition                                                           */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   uid;        /* User ID */
    INT32   confidence; /* Degree of confidence */
}RECOGNITION_RESULT;

/*----------------------------------------------------------------------------*/
/* Face Detection & Estimations result                                        */
/*----------------------------------------------------------------------------*/
typedef struct{
    DETECT_RESULT       dtResult;           /* Face detection result */
    DIR_RESULT          dirResult;          /* Face direction estimation result */
    AGE_RESULT          ageResult;          /* Age Estimation result */
    GENDER_RESULT       genderResult;       /* Gender Estimation result */
    GAZE_RESULT         gazeResult;         /* Gaze Estimation result */
    BLINK_RESULT        blinkResult;        /* Blink Estimation result */
    EXPRESSION_RESULT   expressionResult;   /* Expression Estimation result */
    RECOGNITION_RESULT  recognitionResult;  /* Face Recognition result */
}FACE_RESULT;

/*----------------------------------------------------------------------------*/
/* Human Body Detection results                                               */
/*----------------------------------------------------------------------------*/
typedef struct{
    UINT8           num;            /* Number of Detection */
    DETECT_RESULT   bdResult[35];   /* Detection result */
}BD_RESULT;

/*----------------------------------------------------------------------------*/
/* Hand Detection results                                                     */
/*----------------------------------------------------------------------------*/
typedef struct{
    UINT8           num;            /* Number of Detection */
    DETECT_RESULT   hdResult[35];   /* Detection result */
}HD_RESULT;

/*----------------------------------------------------------------------------*/
/* Face Detection & Estimations results                                       */
/*----------------------------------------------------------------------------*/
typedef struct{
    UINT8           num;            /* Number of Detection */
    FACE_RESULT     fcResult[35];   /* Detection & Estimations result */
}FD_RESULT;

/*----------------------------------------------------------------------------*/
/* Image data                                                                 */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   width;
    INT32   height;
    UINT8   image[320*240];
}HVC_IMAGE;

/*----------------------------------------------------------------------------*/
/* Eesult data of Execute command                                             */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32       executedFunc;   /* Execution flag */
    BD_RESULT   bdResult;       /* Human Body Detection results */
    HD_RESULT   hdResult;       /* Hand Detection results */
    FD_RESULT   fdResult;       /* Face Detection & Estimations results */
    HVC_IMAGE   image;          /* Image data */
}HVC_RESULT;

/*----------------------------------------------------------------------------*/
/* Threshold of confidence                                                    */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   bdThreshold;        /* Threshold of confidence of Human Body Detection */
    INT32   hdThreshold;        /* Threshold of confidence of Hand Detection */
    INT32   dtThreshold;        /* Threshold of confidence of Face Detection */
    INT32   rsThreshold;        /* Threshold of confidence of Face Recognition */
}HVC_THRESHOLD;

/*----------------------------------------------------------------------------*/
/* Detection size                                                             */
/*----------------------------------------------------------------------------*/
typedef struct{
    INT32   bdMinSize;          /* Minimum detection size of Human Body Detection */
    INT32   bdMaxSize;          /* Maximum detection size of Human Body Detection */
    INT32   hdMinSize;          /* Minimum detection size of Hand Detection */
    INT32   hdMaxSize;          /* Maximum detection size of Hand Detection */
    INT32   dtMinSize;          /* Minimum detection size of Face Detection */
    INT32   dtMaxSize;          /* Maximum detection size of Face Detection */
}HVC_SIZERANGE;

#endif /* HVCDef_H__ */

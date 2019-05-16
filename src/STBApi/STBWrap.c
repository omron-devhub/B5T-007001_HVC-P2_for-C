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
    STB Sample API
*/

#include <stdlib.h>
#include "STBWrap.h"

#pragma comment(lib, "STB.lib")

#define STB_MAX_NUM 35

static HSTB m_Handle;
static int m_nFaceCount;
static STB_FACE m_Face[STB_MAX_NUM];
static int m_nBodyCount;
static STB_BODY m_Body[STB_MAX_NUM];


int STB_Init(int inFuncFlag)
{
    if(NULL != m_Handle){
        STB_DeleteHandle(m_Handle);
        m_Handle = NULL;
    }

    m_Handle = STB_CreateHandle(inFuncFlag);
    if(NULL == m_Handle){
        return STB_ERR_INITIALIZE;
    }
    return STB_NORMAL;
}

void STB_Final(void)
{
    if(NULL != m_Handle){
        STB_DeleteHandle(m_Handle);
        m_Handle = NULL;
    }
}

int STB_Exec(int inActiveFunc, const HVC_RESULT *inResult, int *pnSTBFaceCount, STB_FACE **pSTBFaceResult, int *pnSTBBodyCount, STB_BODY **pSTBBodyResult)
{
    int ret;
    STB_FRAME_RESULT frameRes;

    m_nFaceCount = 0;
    m_nBodyCount = 0;
    GetFrameResult(inActiveFunc, inResult, &frameRes);
    do{
        // Set frame information (Detection Result)
        ret = STB_SetFrameResult(m_Handle, &frameRes);
        if(STB_NORMAL != ret){
            break;
        }

        // STB Execution
        ret = STB_Execute(m_Handle);
        if(STB_NORMAL != ret){
            break;
        }

        // Get STB Result
        ret = STB_GetFaces(m_Handle, (STB_UINT32 *)&m_nFaceCount, m_Face);
        if(STB_NORMAL != ret){
            break;
        }

        ret = STB_GetBodies(m_Handle, (STB_UINT32 *)&m_nBodyCount, m_Body);
        if(STB_NORMAL != ret){
            break;
        }
    }while(0);

    *pnSTBFaceCount = m_nFaceCount;
    *pSTBFaceResult = m_Face;
    *pnSTBBodyCount = m_nBodyCount;
    *pSTBBodyResult = m_Body;
    return ret;
}

int STB_Clear(void)
{
    return STB_ClearFrameResults(m_Handle);
}

int STB_SetTrParam(int inRetryCount, int inStbPosParam, int inStbSizeParam)
{
    int ret;
    do{
        ret = STB_SetTrRetryCount(m_Handle, inRetryCount);
        if(STB_NORMAL != ret){
            break;
        }

        ret = STB_SetTrSteadinessParam(m_Handle, inStbPosParam, inStbSizeParam);
    }while(0);

    return ret;
}

int STB_SetPeParam(int inThreshold, int inUDAngleMin, int inUDAngleMax, int inLRAngleMin, int inLRAngleMax, int inCompCount)
{
    int ret;
    do{
        ret = STB_SetPeThresholdUse(m_Handle, inThreshold);
        if(STB_NORMAL != ret){
            break;
        }

        ret = STB_SetPeAngleUse(m_Handle, inUDAngleMin, inUDAngleMax, inLRAngleMin, inLRAngleMax);
        if(STB_NORMAL != ret){
            break;
        }

        ret = STB_SetPeCompleteFrameCount(m_Handle, inCompCount);
    }while(0);

    return ret;
}

int STB_SetFrParam(int inThreshold, int inUDAngleMin, int inUDAngleMax, int inLRAngleMin, int inLRAngleMax, int inCompCount, int inRatio)
{
    int ret;
    do{
        ret = STB_SetFrThresholdUse(m_Handle, inThreshold);
        if(STB_NORMAL != ret){
            break;
        }

        ret = STB_SetFrAngleUse(m_Handle, inUDAngleMin, inUDAngleMax, inLRAngleMin, inLRAngleMax);
        if(STB_NORMAL != ret){
            break;
        }

        ret = STB_SetFrCompleteFrameCount(m_Handle, inCompCount);
        if(STB_NORMAL != ret){
            break;
        }

        ret = STB_SetFrMinRatio(m_Handle, inRatio);
    }while(0);

    return ret;
}



static void GetFrameResult(int inActiveFunc, const HVC_RESULT *inResult, STB_FRAME_RESULT *outFrameResult)
{
    int i;

    // Body Detection
    outFrameResult->bodys.nCount = inResult->bdResult.num;
    if(inActiveFunc & HVC_ACTIV_BODY_DETECTION){
        for(i = 0; i <  inResult->bdResult.num; i++){
            DETECT_RESULT dtRes = inResult->bdResult.bdResult[i];
            outFrameResult->bodys.body[i].center.nX     = dtRes.posX;
            outFrameResult->bodys.body[i].center.nY     = dtRes.posY;
            outFrameResult->bodys.body[i].nSize         = dtRes.size;
            outFrameResult->bodys.body[i].nConfidence   = dtRes.confidence;
        }
    }

    outFrameResult->faces.nCount = inResult->fdResult.num;
    for(i = 0; i < inResult->fdResult.num; i++){
        // Face Detection
        if(inActiveFunc & HVC_ACTIV_FACE_DETECTION){
            DETECT_RESULT dtRes = inResult->fdResult.fcResult[i].dtResult;
            outFrameResult->faces.face[i].center.nX     = dtRes.posX;
            outFrameResult->faces.face[i].center.nY     = dtRes.posY;
            outFrameResult->faces.face[i].nSize         = dtRes.size;
            outFrameResult->faces.face[i].nConfidence   = dtRes.confidence;
        }
        
        // Face Direction
        if(inActiveFunc & HVC_ACTIV_FACE_DIRECTION){
            DIR_RESULT dirRes = inResult->fdResult.fcResult[i].dirResult;
            outFrameResult->faces.face[i].direction.nUD         = dirRes.pitch;
            outFrameResult->faces.face[i].direction.nLR         = dirRes.yaw;
            outFrameResult->faces.face[i].direction.nRoll       = dirRes.roll;
            outFrameResult->faces.face[i].direction.nConfidence = dirRes.confidence;
        } else {
            outFrameResult->faces.face[i].direction.nUD         = 0;
            outFrameResult->faces.face[i].direction.nLR         = 0;
            outFrameResult->faces.face[i].direction.nRoll       = 0;
            outFrameResult->faces.face[i].direction.nConfidence = 0;
        }

        // Age
        if(inActiveFunc & HVC_ACTIV_AGE_ESTIMATION){
            AGE_RESULT ageRes = inResult->fdResult.fcResult[i].ageResult;
            outFrameResult->faces.face[i].age.nAge              = ageRes.age;
            outFrameResult->faces.face[i].age.nConfidence       = ageRes.confidence;
        } else {
            outFrameResult->faces.face[i].age.nAge              = -128;
            outFrameResult->faces.face[i].age.nConfidence       = -128;
        }

        // Gender
        if(inActiveFunc & HVC_ACTIV_GENDER_ESTIMATION){
            GENDER_RESULT genderRes = inResult->fdResult.fcResult[i].genderResult;
            outFrameResult->faces.face[i].gender.nGender        = genderRes.gender;
            outFrameResult->faces.face[i].gender.nConfidence    = genderRes.confidence;
        } else {
            outFrameResult->faces.face[i].gender.nGender        = -128;
            outFrameResult->faces.face[i].gender.nConfidence    = -128;
        }

        // ”FØ
        if(inActiveFunc & HVC_ACTIV_FACE_RECOGNITION){
            RECOGNITION_RESULT recogRes = inResult->fdResult.fcResult[i].recognitionResult;
            outFrameResult->faces.face[i].recognition.nUID      = recogRes.uid;
            outFrameResult->faces.face[i].recognition.nScore    = recogRes.confidence;
        } else {
            outFrameResult->faces.face[i].recognition.nUID      = -128;
            outFrameResult->faces.face[i].recognition.nScore    = -128;
        }
    }
}


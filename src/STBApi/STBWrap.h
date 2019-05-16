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

#ifndef STBWrap_H__
#define STBWrap_H__

#include "STBAPI.h"
#include "STBCommonDef.h"

#include "HVCApi.h"


#ifdef  __cplusplus
extern "C" {
#endif

int STB_Init(int inFuncFlag);
void STB_Final(void);

int STB_Exec(int inActiveFunc, const HVC_RESULT *inResult, int *pnSTBFaceCount, STB_FACE **pSTBFaceResult, int *pnSTBBodyCount, STB_BODY **pSTBBodyResult);

int STB_Clear(void);

int STB_SetTrParam(int inRetryCount, int inStbPosParam, int inStbSizeParam);
int STB_SetPeParam(int inThreshold, int inUDAngleMin, int inUDAngleMax, int inLRAngleMin, int inLRAngleMax, int inCompCount);
int STB_SetFrParam(int inThreshold, int inUDAngleMin, int inUDAngleMax, int inLRAngleMin, int inLRAngleMax, int inCompCount, int inRatio);

static void GetFrameResult(int inActiveFunc, const HVC_RESULT *inResult, STB_FRAME_RESULT *outFrameResult);

#ifdef  __cplusplus
}
#endif

#endif  /* STBWrap_H__ */

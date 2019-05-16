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

#include <windows.h>

#include <stdio.h>
#include "uart.h"

static HANDLE hCom = INVALID_HANDLE_VALUE;

/* UART */
void com_close(void)
{
    if ( hCom != INVALID_HANDLE_VALUE ) {
        CloseHandle(hCom);
        hCom = INVALID_HANDLE_VALUE;
    }
}

int com_init(S_STAT *stat)
{
    DCB dcb;
    BOOL fSuccess;
    char device[16];

    com_close();

    sprintf_s(device, 16, "\\\\.\\COM%d", stat->com_num);
    hCom = CreateFile(device,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);

    if ( hCom == INVALID_HANDLE_VALUE ) {
        return(FALSE);
    }

    fSuccess = GetCommState(hCom,&dcb);
    if ( !fSuccess ) {
        com_close();
        return(FALSE);
    }

    dcb.BaudRate = stat->BaudRate;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fDsrSensitivity = FALSE;
    dcb.fOutxCtsFlow = 0;
    dcb.fTXContinueOnXoff = 0;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;

    fSuccess = SetCommState(hCom,&dcb);
    if ( !fSuccess ) {
        com_close();
        return(FALSE);
    }

    fSuccess = SetupComm(hCom, 10240, 10240);
    if ( !fSuccess ) {
        com_close();
        return(FALSE);
    }

    return TRUE;
}

int com_send(unsigned char *buf, int len)
{
    DWORD dwSize = 0;
    DWORD dwTotal = 0;

    if ( hCom != INVALID_HANDLE_VALUE ) {
		while ((DWORD)len > 0) {
			dwSize = 0;
			if (len > 63) {
		        WriteFile(hCom,&buf[dwTotal],63,&dwSize,NULL);
			}
			else {
		        WriteFile(hCom,&buf[dwTotal],len,&dwSize,NULL);
			}
			if (dwSize == 0) {
				break;
			}
			len -= dwSize;
			dwTotal += dwSize;
		}
    }
    return (int)dwTotal;
}

int com_recv(int inTimeOutTimer, unsigned char *buf, int len)
{
    DWORD ierr;
    COMSTAT stat;
    DWORD dwSize = 0;

    int ret = 0;
    int totalSize = 0;

    double finishTime = 0.0;

    LARGE_INTEGER timeFreq = {0, 0};
    LARGE_INTEGER stopTime = {0, 0};
    LARGE_INTEGER startTime = {0, 0};

    QueryPerformanceFrequency(&timeFreq);

    if ( hCom != INVALID_HANDLE_VALUE ) {
        QueryPerformanceCounter(&startTime);
        do{
            ClearCommError(hCom,&ierr,&stat);
            if ( stat.cbInQue >= 1 ) {
                ret = len - totalSize;
                if ( ret > (int)stat.cbInQue ) ret = stat.cbInQue;
                ReadFile(hCom,&buf[totalSize],ret,&dwSize,NULL);
                totalSize += (int)dwSize;
            }
            if ( totalSize >= len ) break;

            QueryPerformanceCounter(&stopTime);
            finishTime = (double)(stopTime.QuadPart - startTime.QuadPart) * 1000 / (double)timeFreq.QuadPart;
            if ( finishTime >= (double)inTimeOutTimer ) break;
        }while(1);
    }
    return totalSize;
}

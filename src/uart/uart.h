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

#ifndef UART_H__
#define UART_H__

/* Struct for the serial port */
typedef struct {
    int com_num;                /* COM number */
    unsigned long BaudRate;     /* Baud rate 9600-921600 */
} S_STAT;

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef WIN32
void com_sleep(int nSleep);
#endif

void com_close(void);
int com_init(S_STAT *stat);
int com_send(unsigned char *buf, int len);
int com_recv(int inTimeOutTimer, unsigned char *buf, int len);

#ifdef  __cplusplus
}
#endif

#endif  /* UART_H__ */

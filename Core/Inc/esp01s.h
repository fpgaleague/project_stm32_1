/*
 * esp01s.h
 *
 *  Created on: Jul 15, 2025
 *      Author: myown
 */

#ifndef _ESP01S_H_
#define _ESP01S_H_

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

void ESP01S_Init(void);

void MY_ESP01S_Init(void);//我自己的

void ESP01S_Clear(void);

void ESP01S_SendData(unsigned char *data, unsigned short len);

_Bool ESP01S_SendCmd(char *cmd, char *res);

unsigned char *ESP01S_GetIPD(unsigned short timeOut);

#endif


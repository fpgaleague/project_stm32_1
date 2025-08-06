/*
 * BH1750.c
 *
 *  Created on: 2025年7月30日
 *      Author: myown
 */
#include "BH1750.h"
#include <i2c.h>


#define  BH1750_POWER_ON     0x01  //开启BH1750命令
#define  BH1750_ADDRESS      0x23  //ADDRESS引脚接地时地址为0x46，接电源时地址为0xB8
#define  BH1750_WriADD       0x46
#define  BH1750_RedADD       0x47
       //写操作：010 0011 + 0 = 01000110 (0x46)
       //读操作：010 0011 + 1 = 01000111 (0x47)

void BH1750_WriteCmd(uint8_t cmd) {

    HAL_I2C_Master_Transmit(&hi2c1, BH1750_WriADD, &cmd, 1, HAL_MAX_DELAY);
}

//读取BH170数据,设备地址0x46
// 读取光照数据（2字节）
void BH1750_ReadData(uint8_t *data, uint8_t len) {
    HAL_I2C_Master_Receive(&hi2c1, BH1750_WriADD, data, len, HAL_MAX_DELAY);
}

void  BH1750_Init()
{
	BH1750_WriteCmd(0x01);// power on
	BH1750_WriteCmd(0x10);//H- resolution mode // 设置连续高精度模式
	HAL_Delay(180);  // 等待首次测量完成
}




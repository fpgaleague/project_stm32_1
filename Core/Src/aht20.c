/*
 * aht20.c
 *
 *  Created on: Jul 3, 2025
 *      Author: myown
 */


#include "aht20.h"

//发送7位从机地址（如AHT20的0x38） + 写位0，组成8位数据（0x70）。
#define AHT20_ADDRESS  0x70

//AHT20初始化
void  AHT20_Init() {
      uint8_t readBuffer;

      HAL_Delay(40);
      HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDRESS, &readBuffer, 1 , HAL_MAX_DELAY);  //
      if((readBuffer & 0x08) == 0x00) //检查状态寄存器的 Bit3 (0x08)，该位表示传感器是否已校准
      {  //// 只有未校准时才发送校准命令。发送 校准命令 0xBE，后跟两个参数 0x08 和 0x00（AHT20的校准命令格式）
    	  uint8_t sendBuffer[3] = { 0xBE, 0x08, 0X00};
          HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDRESS, sendBuffer, 3, HAL_MAX_DELAY);
      }
}

void  AHT20_Read(float *Temperature, float *Humidity) {
      uint8_t sendBuffer[3] = { 0xAC, 0x33, 0x00 };
      uint8_t readBuffer[6];

      HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDRESS, sendBuffer, 3, HAL_MAX_DELAY);
      HAL_Delay(75);
      HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDRESS, readBuffer, 6, HAL_MAX_DELAY);

      if((readBuffer[0] & 0x80 ) == 0x00) {
          uint32_t data = 0;
          data = ((uint32_t)readBuffer[3] >> 4) + ((uint32_t)readBuffer[2] << 4) + ((uint32_t)readBuffer[1] <<12 );
          *Humidity = data * 100.0f /(1 << 20);

          data = (((uint32_t)readBuffer[3] & 0x0F) << 16) + ((uint32_t)readBuffer[4] << 8) + (uint32_t)readBuffer[5] ;
          *Temperature = data * 200.0f / (1 << 20) - 50;
      }
}














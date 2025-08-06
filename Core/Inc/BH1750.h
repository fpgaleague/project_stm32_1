/*
 * BH1750.h
 *
 *  Created on: 2025年7月30日
 *      Author: myown
 */
#include "i2c.h"


#ifndef INC_BH1750_H_
#define INC_BH1750_H_

void BH1750_WriteCmd(uint8_t reg_add);

void BH1750_ReadData(uint8_t *data, uint8_t len);

void BH1750_Init();


#endif /* INC_BH1750_H_ */

/*
 * onenet.c
 *
 *  Created on: Jul 16, 2025
 *      Author: myown
 */



//单片机头文件
#include "main.h"

//网络设备
#include "esp01s.h"

//协议文件
#include "onenet.h"
#include "MQTT.h"

//硬件驱动
#include "usart.h"
//C库
#include <string.h>
#include <stdio.h>

#include "aht20.h"

#define PROID		"4z54XBmy4z"//产品ID

#define AUTH_INFO	"version=2018-10-31&res=products%2F4z54XBmy4z%2Fdevices%2Fmytest&et=2684126974&method=md5&sign=b27jc0SK3kgrLULGvz2qww%3D%3D"//鉴权信息

#define DEVID		"mytest"//设备ID


extern unsigned char ESP01S_buf[128];


//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNet_DevLink(void)
{
    MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
    unsigned char *dataPtr;
    _Bool status = 1;
    char debugMsg[128];

    // 替换原来的UsartPrintf
    sprintf(debugMsg, "OneNet_DevLink\r\nPROID: %s, AUIF: %s, DEVID:%s\r\n", PROID, AUTH_INFO, DEVID);
    HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), HAL_MAX_DELAY);

    if(MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
    {
        ESP01S_SendData(mqttPacket._data, mqttPacket._len);
        dataPtr = ESP01S_GetIPD(250);

        if(dataPtr != NULL)
        {
            if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
            {
                switch(MQTT_UnPacketConnectAck(dataPtr))
                {
                    case 0:
                        HAL_UART_Transmit(&huart2, (uint8_t*)"Tips: 连接成功\r\n", 16, HAL_MAX_DELAY);
                        status = 0;
                        break;
                    case 1:
                        HAL_UART_Transmit(&huart2, (uint8_t*)"WARN: 连接失败：协议错误\r\n", 26, HAL_MAX_DELAY);
                        break;
                    case 2:
                        HAL_UART_Transmit(&huart2, (uint8_t*)"WARN: 连接失败：非法的clientid\r\n", 30, HAL_MAX_DELAY);
                        break;
                    case 3:
                        HAL_UART_Transmit(&huart2, (uint8_t*)"WARN: 连接失败：服务器失败\r\n", 26, HAL_MAX_DELAY);
                        break;
                    case 4:
                        HAL_UART_Transmit(&huart2, (uint8_t*)"WARN: 连接失败：用户名或密码错误\r\n", 34, HAL_MAX_DELAY);
                        break;
                    case 5:
                        HAL_UART_Transmit(&huart2, (uint8_t*)"WARN: 连接失败：非法链接(比如token非法)\r\n", 42, HAL_MAX_DELAY);
                        break;
                    default:
                        HAL_UART_Transmit(&huart2, (uint8_t*)"ERR: 连接失败：未知错误\r\n", 24, HAL_MAX_DELAY);
                        break;
                }
            }
        }
        MQTT_DeleteBuffer(&mqttPacket);
    }
    else
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"WARN: MQTT_PacketConnect Failed\r\n", 31, HAL_MAX_DELAY);
    }
    return status;
}

//修改的原本的
/*unsigned char OneNet_FillBuf(char *buf)
{

	char text[32];
	float Temp;
    float Humidity;
	memset(text, 0, sizeof(text));

	strcpy(buf, ",;");
	Temp = data[2]+(float)data[3]/10;
    Humidity=data[0]+(float)data[1]/10;
	memset(text, 0, sizeof(text));
	sprintf(text, "Temp,%.2f;Humidity,%.2f;",Temp,Humidity);
	strcat(buf, text);


	return strlen(buf);

}*/

unsigned char OneNet_FillBuf(char *buf)
{
    float temperature, humidity;
    char text[64];

    // 读取AHT20数据
    AHT20_Read(&temperature, &humidity);

    // 格式化数据到buf
    sprintf(buf, ",;CurrentTemperature,%.2f;CurrentHumidity,%.2f;", temperature, humidity);

    // 调试输出（直接替换UsartPrintf）
    char debugMsg[64];
    sprintf(debugMsg, "AHT20 Data: Temp=%.2fC, Humi=%.2f%%\r\n", temperature, humidity);
    HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), HAL_MAX_DELAY);

    return strlen(buf);
}

//==========================================================
//	函数名称：	OneNet_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//
//	返回参数：	无
//
//	说明：
//==========================================================
void OneNet_SendData(void)
{
    MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
    char buf[128];
    short body_len = 0, i = 0;
    char debugMsg[64];

    // 替换原来的UsartPrintf
    HAL_UART_Transmit(&huart2, (uint8_t*)"Tips: OneNet_SendData-MQTT\r\n", 26, HAL_MAX_DELAY);

    memset(buf, 0, sizeof(buf));
    body_len = OneNet_FillBuf(buf);

    if(body_len)
    {
        if(MQTT_PacketSaveData(DEVID, body_len, NULL, 5, &mqttPacket) == 0)
        {
            for(; i < body_len; i++)
                mqttPacket._data[mqttPacket._len++] = buf[i];

            ESP01S_SendData(mqttPacket._data, mqttPacket._len);

            // 发送字节数调试信息
            sprintf(debugMsg, "Send %d Bytes\r\n", mqttPacket._len);
            HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), HAL_MAX_DELAY);

            MQTT_DeleteBuffer(&mqttPacket);
        }
        else
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)"WARN: EDP_NewBuffer Failed\r\n", 27, HAL_MAX_DELAY);
        }
    }
}

//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
    MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
    char *req_payload = NULL;
    char *cmdid_topic = NULL;
    unsigned short req_len = 0;
    unsigned char type = 0;
    short result = 0;
    char *dataPtr = NULL;
    char numBuf[10];
    int num = 0;
    char debugMsg[64];

    type = MQTT_UnPacketRecv(cmd);
    switch(type)
    {
        case MQTT_PKT_CMD:
            result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);
            if(result == 0)
            {
                sprintf(debugMsg, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
                HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), HAL_MAX_DELAY);

                if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)
                {
                    HAL_UART_Transmit(&huart2, (uint8_t*)"Tips: Send CmdResp\r\n", 20, HAL_MAX_DELAY);
                    ESP01S_SendData(mqttPacket._data, mqttPacket._len);
                    MQTT_DeleteBuffer(&mqttPacket);
                }
            }
            break;

        case MQTT_PKT_PUBACK:
            if(MQTT_UnPacketPublishAck(cmd) == 0)
                HAL_UART_Transmit(&huart2, (uint8_t*)"Tips: MQTT Publish Send OK\r\n", 27, HAL_MAX_DELAY);
            break;

        default:
            result = -1;
            break;
    }

    ESP01S_Clear();

    if(result == -1)
        return;

    dataPtr = strchr(req_payload, '}');
    if(dataPtr != NULL && result != -1)
    {
        dataPtr++;
        while(*dataPtr >= '0' && *dataPtr <= '9')
            numBuf[num++] = *dataPtr++;
        numBuf[num] = 0;
        num = atoi((const char *)numBuf);
    }

    if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
    {
        MQTT_FreeBuffer(cmdid_topic);
        MQTT_FreeBuffer(req_payload);
    }
}




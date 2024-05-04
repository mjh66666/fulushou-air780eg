/**
    说明：780EG驱动，适用于EC618&EC716&EC718系列 AT指令内置MQTT协议，不用额外的协议文件
    简单MQTT发送json数据功能
    日期：24/4/16
    版本：v1.0
    广州软件学院萝卜丁团队出品
**/
#include "stm32f10x.h" // Device header

// C库
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "780eg.h"

// 硬件驱动
#include "delay.h"
#include "usart.h"

unsigned char Air780EG_buf[256]; // 串口缓冲区

unsigned short Air780EG_cnt = 0, Air780EG_lastcnt = 0; // 接收数据计数值,上一个计数值

/**
 * @brief 清除缓存
 * @param	无
 * @retval 无
 */

void Air780EG_Clear(void)
{
    memset(Air780EG_buf, 0, sizeof(Air780EG_buf)); // 将缓存区的数据全部清空
    Air780EG_cnt = 0;
}

/**
  * @brief	等待接收完成
  * @param	无
  * @retval	REV_OK 接收完成标志 REV_WAIT	1接收未完成标志

  */
_Bool Air780EG_WaitRecive(void)
{
    //size_t length;
    if (Air780EG_cnt == 0)
        return REV_WAIT; // 完全没在接收

    if (Air780EG_cnt == Air780EG_lastcnt) // 如果上一次的值和这次相同，则说明接收完毕
    {
        // length = strlen(Air780EG_buf);
        // if (length < 256 - 1) {
        //     Air780EG_buf[length] = '\0'; //非必要，为字符串尾部增加'\0',以防下面strcpy出错
        // } else {
        //     Air780EG_buf[256 - 1] = '\0';
        // }
        Air780EG_cnt = 0; // 清零接收计数值
        return REV_OK;
    }

    Air780EG_lastcnt = Air780EG_cnt; // 如果不是上面两种情况，说明还在继续接收数据，对last进行赋值，下次继续比对
    return REV_WAIT;
}

/**
 * @brief	发送命令
 * @param	cmd:命令
 * @param   ret:需要检查的返回指令，如“+xxx”，“ok”
 * @param   respond:保存返回指令，读取参数
 * @retval  0 为成功，1为失败
 */
_Bool Air780EG_Sendcmd(char *cmd, char *ret, char *respond)
{
    uint8_t timewait = 200; // 等待次数 0~255
    Usart_SendString(USART2, (unsigned char *)cmd, strlen((const char *)cmd));

    while (timewait--) {

        if (Air780EG_WaitRecive() == REV_OK) // 如果收到数据
        {
            if (strstr((const char *)Air780EG_buf, ret) != NULL) // 如果检索到关键词
            {
                if (respond != NULL) { // 判断是否有填返回值参数
                    strcpy(respond, (const char *)Air780EG_buf);
                }

                Air780EG_Clear(); // 清空缓存
                return 0;
            }
        }
        Delay_ms(10);
    }
    return 1;
}

/**
 * @brief	Air780EG设备初始化
 * @param	无
 * @retval  无
 */
void Air780EG_Init()
{
    char test[100];
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure); // reset低电平有效

    GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
    Delay_ms(2000);
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
    Delay_ms(2000);

    UsartPrintf(USART1, "Reset!");

    Air780EG_Clear();

    while (Air780EG_Sendcmd("AT\r\n", "OK\r\n", NULL)) // 查询AT收发是否正常
        Delay_ms(100);
    while (Air780EG_Sendcmd("ATE0\r\n", "OK\r\n", NULL)) // 关闭回显
        Delay_ms(100);
    UsartPrintf(USART1, "AT OK!");
    while (Air780EG_Sendcmd("AT+CPIN?\r\n", "READY", NULL)) // 查询PIN码锁状态
        Delay_ms(100);

    UsartPrintf(USART1, "SIM OK!");
    while (Air780EG_Sendcmd("AT+CGATT?\r\n", "+CGATT: 1", test)) // 查询SIM卡是否附着
        Delay_ms(100);

    // 连接到emqx平台
    while (Air780EG_Sendcmd(AT_EMQX_IP, "CONNECT OK", NULL))
        Delay_ms(100);
    while (Air780EG_Sendcmd(AT_EMQX_Client, "OK", NULL))
        Delay_ms(100);
    while (Air780EG_Sendcmd(AT_EMQX_CONNECT, "CONNACK OK", NULL))
        Delay_ms(100);
    UsartPrintf(USART1, "EMQX OK!");

    UsartPrintf(USART1, "EMQX ready\r\n");
}

/**
 * @brief	将单个数据封包为json格式
 * @param	msg:消息键值名
 * @param   value：数据值
 * @param	Type:数据类型，可以为int和float
 * @retval	字符串长度
 */

unsigned char save_jsonData(char *json_output, char msg[], void *value, enum DataType Type)
{
    unsigned short json_len;
    char json_buf[150]; // buf:数据存储区
    switch (Type) {
        case Int:
            sprintf(json_buf, "{\\22%s\\22:%d}", msg, *(int *)value);
            break; // 将数据格式化成json
        case Float:
            sprintf(json_buf, "{\\22%s\\22:%1f}", msg, *(float *)value);
            break;
    }
    json_len = strlen(json_buf) / sizeof(char); // 字符串长度
    memcpy(json_output, json_buf, json_len);    // copy json数据到json_output
    return json_len;
}

/**
 * @brief	将数据封包为json格式 心率、血氧、温度专用
 * @param	json_output:输出，value1、2、3：数据值，
 * @retval	字符串长度
 */
unsigned char save_therejsonData(char *json_output, float *SPO2_value, int *heart_value, int *temp_value)
{
    unsigned short json_len;
    char json_buf[150];
    sprintf(json_buf, "{\\22%s\\22:%1f,\\22%s\\22:%d,\\22%s\\22:%d}", "SPO2", *SPO2_value, "heart", *heart_value, "temp", *temp_value); // 将数据格式化成json
    json_len = strlen(json_buf) / sizeof(char);                                                                                         // 字符串长度
    memcpy(json_output, json_buf, json_len);                                                                                            // copy json数据到
    return json_len;
}

/**
  * @brief	通过AT+MPUB发布命令发送json格式的单个数据
  * @param	Type:数据类型,,,
    @param	qos：服务质量
    @param	retain：保留标志
    @param	char topic[]：消息主题
    @param  data：消息内容
    典型:AT+MPUB="test2",0,0,"{\22msg\22:\22say\22}"
  * @retval
  */
void Air780EG_Sendmqttdata(enum DataType Type, char topic[], int qos, int retain, void *data, char msg[])
{
    char buf[150];     // 存储要json字符
    char sendbuf[150]; // 存储要发送的总字符

    Air780EG_Clear(); // 清缓存
    memset(sendbuf, 0, sizeof(sendbuf));
    memset(buf, 0, sizeof(buf));

    save_jsonData(buf, msg, data, Type); // 格式化数据,消息名，数据
    UsartPrintf(USART1, "pack ready\r\n");
    sprintf(sendbuf, "AT+MPUB=\"%s\",%d,%d,\"%s\"\r\n", topic, qos, retain, buf); // '\'转义 ，" \" "代表”双引号

    Air780EG_Sendcmd(sendbuf, "OK\r\n", NULL);

    memset(sendbuf, 0, sizeof(sendbuf));
    memset(buf, 0, sizeof(buf));
}

/**
  * @brief	通过AT+MPUB发布命令发送json格式的三个数据（心率血氧温度专用）
            qos：服务质量为0,retain：保留标志为0,
  * @param	char topic[]：主题,data：消息内容
            典型:AT+MPUB="test2",0,0,"{\22msg\22:\22say\22}"
  * @retval
  */

void Air780EG_Sendtheremqttdata(char topic[], float *SPO2_data, int *heart_data, int *temp_data)
{
    char buf[150];     // 存储要json字符
    char sendbuf[150]; // 存储要发送的总字符

    Air780EG_Clear(); // 清缓存
    memset(sendbuf, 0, sizeof(sendbuf));
    memset(buf, 0, sizeof(buf));

    save_therejsonData(buf, SPO2_data, heart_data, temp_data); // 格式化数据,数据
    UsartPrintf(USART1, "pack ready\r\n");
    sprintf(sendbuf, "AT+MPUB=\"%s\",%d,%d,\"%s\"\r\n", topic, 0, 0, buf); // '\'转义 ，" \" "代表”双引号

    Air780EG_Sendcmd(sendbuf, "OK\r\n", NULL);

    memset(sendbuf, 0, sizeof(sendbuf));
    memset(buf, 0, sizeof(buf));
}

/**
 * @brief 串口中断	将DR寄存器里的串口数据读到Air780EG缓存区
 * @param
 * @retval
 */
void USART2_IRQHandler(void)
{

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) // 接收中断
    {
        if (Air780EG_cnt >= sizeof(Air780EG_buf)) Air780EG_cnt = 0; // 防止串口被刷爆
        Air780EG_buf[Air780EG_cnt++] = USART2->DR;

        USART_ClearFlag(USART2, USART_FLAG_RXNE);
    }
}

/**************************以下为GNSS模块功能部分********************************************/

/**
 * @brief GNSS初始化
 *
 */
void Air780EG_GNSSInit()
{
    while (Air780EG_Sendcmd("AT+CGNSPWR=1\r\n", "OK\r\n", NULL)) // 打开GPS
        Delay_ms(500);
    while (Air780EG_Sendcmd("AT+CGNSAID=31,1,1,1\r\n", "OK\r\n", NULL)) // 使能位置辅助定位
        Delay_ms(500);
    UsartPrintf(USART1, "GNSS ok!");
}
/**
 * @brief 读取GNSS信息后发生数据到EMQX
 * @param 无
 * @retval 0为成功，1为失败
 */
_Bool Air780EG_sendGNSSdata()
{
    char GNSS_respond[256];
    char *GNSS_position;
    int count, fix_status = 0; // 计数值和定位状态
    float data[2];             // 经纬度存储

    /*
    while (Air780EG_Sendcmd("AT+CGNSPWR?", "OK\r\n", GNSS_respond)) // 查询GNSS状态
        Delay_ms(500);
    if (strstr((const char *)GNSS_respond, "1") != NULL) // 如果检索到关键词'1'即为打开
    {
        memset(GNSS_respond, 0, sizeof(GNSS_respond));
    } else // 否则就继续打开
    {
        memset(GNSS_respond, 0, sizeof(GNSS_respond));
        Air780EG_GNSSInit();
    }
    */

    while (Air780EG_Sendcmd("AT+CGNSINF", "+CGNSINF", GNSS_respond))

        GNSS_position = strtok(GNSS_respond, ","); // 分割字符串
    while (GNSS_position != NULL) {

        // 是否成功定位
        if (count == 1) {
            fix_status = atoi(GNSS_position);
            if (fix_status == 0) {
                Air780EG_Sendmqttdata(Int, "warning", 0, 0, &fix_status, "fix_status");
                return 1;
            }
        }

        // 返回1则代表定位成功
        if (fix_status == 1) {
            if (count > 3 && count < 6) {
                data[count - 4] = atof(GNSS_position); // 将相应的信息转换成浮点型之后存到数组里面
            }
            count++;
        }
        GNSS_position = strtok(NULL, ",");
    }
    // 发送数据
    return 0;
}
/*
int main()
{
    char GNSS_respond[256] = "+gnss=1,0,4.0,7.8,9,89";
    char *GNSS_position;
    int count = 0; // 初始化 count 变量为 0
    float data[6];

    GNSS_position = strtok(GNSS_respond, ",");
    while (GNSS_position != NULL) {
        if (count > 2) {
            data[count - 3] = atof(GNSS_position);                  // 将字符串转换为浮点数，并存储到 data 数组中
            printf("data[%d]: %f\r\n", count - 3, data[count - 3]); // 打印转换后的浮点数
        }
        count++; // 增加 count 变量
        GNSS_position = strtok(NULL, ",");
    }
    printf("data[0]: %f\r\n", data[0]); // 打印第一个浮点数
    printf("data[1]: %f\r\n", data[1]); // 打印第二个浮点数

    return 0;
}

*/

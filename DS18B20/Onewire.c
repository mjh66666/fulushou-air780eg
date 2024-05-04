#include "stm32f10x.h" // Device header
#include "Delay.h"

/**
 * @brief 单总线读输入
 * @param	无
 * @retval	BitValue
 */
uint8_t Onewire_read(void)
{
    uint8_t BitValue;
    BitValue = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
    return BitValue;
}

/**
 * @brief 单总线写输出
 * @param	BitValue
 * @retval	无
 */
void Onewire_write(uint8_t Bitvalue)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, (BitAction)Bitvalue);
}

/**
 * @brief  单总线初始化
 * @param	无
 * @retval	AckBit 设备应答返回值为0
 */
uint8_t Onewire_Init(void)
{
    uint8_t AckBit;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // RCC时钟使能

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    Onewire_write(1);
    Delay_us(1);
    Onewire_write(0);
    Delay_us(750);
    Onewire_write(1);
    Delay_us(500);
    AckBit = Onewire_read();
    Onewire_write(1);
    return AckBit;
}
/**
 * @brief  单总线发送一个Bit
 * @param	Bit
 * @retval	无
 */
void Onewire_SendBit(unsigned char Bit)
{
    Onewire_write(0);
    Delay_us(15);
    Onewire_write(Bit);
    Delay_us(45);
    Onewire_write(1);
}
/**
 * @brief  单总线接收一个Bit
 * @param	无
 * @retval	Bit
 */
unsigned char Onewire_ReceiveBit(void)
{
    unsigned char Bit;
    Onewire_write(1);
    Delay_us(2);
    Onewire_write(0);
    Delay_us(3);
    Onewire_write(1);
    Delay_us(5);
    Bit = Onewire_read();
    Delay_us(60);
    return Bit;
}

/**
 * @brief  单总线发送一个字节
 * @param	Byte
 * @retval	无
 */
void Onewire_SendByte(unsigned char Byte)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        Onewire_SendBit(Byte & (0x01 << i));
    }
}
/**
 * @brief  单总线接收一个字节
 * @param	无
 * @retval	Byte
 */
unsigned char Onewire_ReceiveByte(void)
{
    unsigned char i;
    unsigned char Byte = 0x00;
    for (i = 0; i < 8; i++) {
        if (Onewire_ReceiveBit()) { Byte |= (0x01 << i); }
    }

    return Byte;
}

#include "stm32f10x.h" // Device header
#include "Onewire.h"
#include "OLED.h"

#define DS18B20_SKIP_ROM        0xCC // 忽略ROM指令
#define DS18B20_CONVERT_T       0x44 // 温度转换指令
#define DS18B20_READ_SCRATCHPAD 0xBE // 读暂存器指令

/**
 * @brief	进行温度转换
 * @param	无
 * @retval无
 */
void DS18B20_ConvertT(void)
{
    Onewire_Init();
    Onewire_SendByte(DS18B20_SKIP_ROM);
    Onewire_SendByte(DS18B20_CONVERT_T);
}

/**
 * @brief	读寄存器数据
 * @param	无
 * @retval温度值
 */
float DS18B20_ReadT(void)
{
    unsigned char TLSB, TMSB;
    float T;
    Onewire_Init();
    Onewire_SendByte(DS18B20_SKIP_ROM);
    Onewire_SendByte(DS18B20_READ_SCRATCHPAD);
    TLSB = Onewire_ReceiveByte();
    TMSB = Onewire_ReceiveByte();
    if (TMSB > 0x7f) {
        TLSB = ~TLSB;
        TMSB = ~TMSB + 1;
    }

    T = ((TMSB << 4) | (TLSB >> 4)) + (float)(TLSB & 0x0f) * 0.0625; // 计算温度值
    return T;
}
#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

uint8_t Onewire_read(void);
void Onewire_write(uint8_t Bitvalue);
uint8_t Onewire_Init(void);
void Onewire_SendBit(unsigned char Bit);
unsigned char Onewire_ReceiveBit(void);
void Onewire_SendByte(unsigned char Byte);
unsigned char Onewire_ReceiveByte(void);
#endif
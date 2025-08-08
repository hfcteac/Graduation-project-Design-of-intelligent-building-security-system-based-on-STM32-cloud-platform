#ifndef __DELAY_H
#define __DELAY_H

#include "sys.h"

void Delay_us(unsigned int us);
void Delay_ms(unsigned int ms);
void Delay_s(unsigned int s);

void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);
#endif

#ifndef __SYS_H
#define __SYS_H
#include "ch32f10x.h"

void WFI_SET(void);
void INTX_DISABLE(void);
void INTX_ENABLE(void);
void MSR_MSP(u32 addr);

#endif

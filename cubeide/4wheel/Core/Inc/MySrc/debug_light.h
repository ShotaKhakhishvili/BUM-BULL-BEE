#ifndef MYSRC_DEBUG_LIGHT_H
#define MYSRC_DEBUG_LIGHT_H

#include <stdint.h>
#include <stdbool.h>

void DebugLight_Init(void);
void DebugLight_SetColor(uint8_t r, uint8_t g, uint8_t b);
void DebugLight_Send(void);
bool DebugLight_IsBusy(void);

#endif /* MYSRC_DEBUG_LIGHT_H */

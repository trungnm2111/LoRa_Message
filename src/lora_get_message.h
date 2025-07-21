#ifndef LORA_GET_MESSAGE_H
#define LORA_GET_MESSAGE_H

#include "lora.h"
/*
 * FSM get Frame data
 */
typedef enum
{
  LORA_FRAME_START   = 0,
  LORA_FRAME_TYPE    = 1,
  LORA_FRAME_LENGTH  = 2,
  LORA_FRAME_DATA    = 3,
  LORA_FRAME_CRC     = 4,
  LORA_FRAME_END     = 5,
}LORA_Fsm_Frame_Data_t;

typedef struct {
    uint8_t Buffer[LORA_PACKET_MAX_SIZE];
    uint16_t Index;
    uint16_t Length;
} LORA_Data_t;

extern LORA_Data_t gLoraData;

extern LORA_Fsm_Frame_Data_t lora_frame_status;

void lora_init_frame(void);
void lora_get_frame(uint8_t temp_char);


#endif // LORA_GET_MESSAGE_H

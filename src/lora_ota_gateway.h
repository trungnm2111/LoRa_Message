#ifndef LORA_OTA_GATEWAY_H
#define LORA_OTA_GATEWAY_H

#include "lora.h"


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

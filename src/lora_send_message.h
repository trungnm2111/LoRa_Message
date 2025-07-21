
#ifndef LORA_SEND_MESSAGE_H
#define LORA_SEND_MESSAGE_H

#include <stdint.h>
#include "lora.h"


extern uint8_t DATA_BUF[LORA_DATA_MAX_SIZE];


void lora_join_network_request();
#endif // LORA_SEND_MESSAGE_H

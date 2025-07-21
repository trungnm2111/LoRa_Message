#include "lora.h"
// #include "rtvars.h"
// #include "hld_uart2.h"

uint8_t mock_data[LORA_PACKET_MAX_SIZE];
uint16_t mock_length;

void lora_send_frame(uint8_t *data, uint16_t length)
{
    memcpy(mock_data, data, length);
    mock_length = length;
}

// void lora_send_frame(uint8_t *data, uint16_t length)
// {
//     // uart2_send_data(data, length);
// }
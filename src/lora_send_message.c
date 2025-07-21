#include "lora_send_message.h"
#include "lora.h"
#include "string.h"
// #include "rtvars.h"
// #include "lora_send_message.h"

uint8_t DATA_BUF[LORA_DATA_MAX_SIZE] = {0}; // Buffer for sending data

void lora_join_network_request()
{
    LORA_frame_t *frame = (LORA_frame_t *)DATA_BUF;
    frame->sof = LORA_SOF;
    frame->packet_type = LORA_PACKET_TYPE_JOIN;
    frame->data_len = 8; // Length of the MAc address
    //
    uint16_t len = LORA_FRAME_HEADER_SIZE;
    //
    // memcpy(&DATA_BUF[len], gSysParam.join_info.mac, sizeof(gSysParam.join_info.mac));
    // len += sizeof(gSysParam.join_info.mac);
    memcpy(&DATA_BUF[len], gSysParam.mac, sizeof(gSysParam.mac));
    len += sizeof(gSysParam.mac);
    //
    // uint32_t crc = help_calculate_crc(DATA_BUF, len);
    crc_t crc = crc_fast(DATA_BUF, len);

    memcpy(&DATA_BUF[len], &crc, sizeof(crc));
    len += sizeof(crc); 
    //
    DATA_BUF[len++] = LORA_EOF; // End of Frame
    //
    // Send the frame
    lora_send_frame(DATA_BUF, len);
}

uint8_t loraJoinNetworkAccept(void)
{
    
}

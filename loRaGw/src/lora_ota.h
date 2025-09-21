#ifndef LORA_OTA_H
#define LORA_OTA_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "lora.h"

#define OTA_ACK                0x00u
#define OTA_NACK               0x01u
#define OTA_START_LENGTH            1

typedef enum {
    OTA_SERVER_IDLE = 0,
    OTA_SERVER_START_SENT,
    OTA_SERVER_HEADER_SENT,
    OTA_SERVER_DATA_SENDING,
    OTA_SERVER_END_SENT,
    OTA_SERVER_SIGNAL_UPDATE_SENT,
    OTA_SERVER_DONE
} ota_server_state_t;


typedef enum {
    OTA_TYPE_START        = 0x00,
    OTA_TYPE_END             ,
    OTA_TYPE_SIGNAL_UPDATE   ,
    OTA_TYPE_RESPONSE        ,
    OTA_TYPE_HEADER          ,
    OTA_TYPE_DATA 
} ota_message_type_t;


/* OTA_HEADER: exactly 16 bytes */
typedef struct {
    uint16_t session_id;
    uint16_t chunk_size;
    uint16_t total_chunks;
    uint16_t reserved0;
    uint32_t total_size;
    uint32_t image_crc32;
} ota_header_t;

typedef struct {
    ota_server_state_t state;
    const uint8_t *fw_image;
    uint32_t fw_size;
    uint16_t chunk_size;
    uint16_t total_chunks;
    uint16_t current_seq;

    ota_header_t header;
} ota_server_ctx_t;

/*
 * LoRa OTA Status 
 */
typedef enum
{    
    LORA_OTA_STATUS_INVALID_SESSION = -4,
    LORA_OTA_STATUS_ENCODE_FAILED = -3,
    LORA_OTA_STATUS_NULL_POINTER = -2,
    LORA_OTA_STATUS_PROCESS_SIGNAL_UPDATE_SUCCESS = -1 ,
    LORA_OTA_STATUS_PROCESS_UPDATE_DONE = 0,
    // Trạng thái thành công (dương, từ 1 trở lên để tránh nhầm lẫn với 0)
    LORA_OTA_STATUS_CREATE_START_SUCCESS            = 1,
    LORA_OTA_STATUS_CREATE_HEADER_SUCCESS           = 2,
    LORA_OTA_STATUS_CREATE_DATA_SUCCESS             = 3,
    LORA_OTA_STATUS_CREATE_RESPONSE_SUCCESS         = 4,
    LORA_OTA_STATUS_CREATE_SIGNAL_UPDATE_SUCCESS    = 5,
    LORA_OTA_STATUS_CREATE_END_SUCCESS              = 6     
} LORA_OtaStatus_t;

extern ota_server_ctx_t ota_srv;
LORA_OtaStatus_t otaServerNextFrame(uint8_t *frame_buf, uint16_t *frame_len);
void otaServerPrepare(const uint8_t *fw_image, uint32_t fw_size, uint16_t chunk_size, uint32_t image_crc32, uint16_t session_id);

#endif // LORA_OTA_H

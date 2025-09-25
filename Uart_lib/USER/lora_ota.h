/* lora_ota.h
 * OTA-over-LoRa protocol header
 * Author: TrungNM
 * Created: *********
*/

#ifndef LORA_OTA_H
#define LORA_OTA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "lora.h"
#include "crc.h"

#define OTA_RESP_ACK                0x00u
#define OTA_RESP_NACK               0x01u
#define OTA_START_LENGTH                1

#define OTA_MAX_FIRMWARE_SIZE    (10 * 1024)  // 64KB maximum
#define OTA_MAX_CHUNK_SIZE       150          // Maximum chunk size
#define OTA_FIRMWARE_BUFFER_SIZE (OTA_MAX_FIRMWARE_SIZE + 256) // Buffer với padding



typedef enum {
    OTA_TYPE_START           = 0x00,    /* payload = 0x00 */
    OTA_TYPE_HEADER					 = 0x01,		/* payload = 16 bytes (ota_header_t) */
		OTA_TYPE_DATA						 = 0x02,		/* payload = seq(2B) + data(nB) */
		OTA_TYPE_RESPONSE        = 0x03,    /* payload = 0x00(ACK)/0x01(NACK) */
		OTA_TYPE_END		         = 0x04,
		OTA_TYPE_SIGNAL_UPDATE   = 0x05    /* payload = 0x02 */  
} ota_message_type_t;




/*
 * LoRa OTA Status 
 */
typedef enum
{ 
		LORA_OTA_STATUS_INVALID_FIRMWARE_SIZE						= -14,
		LORA_OTA_STATUS_INVALID_CHUNK_SIZE 							= -13,
		LORA_OTA_STATUS_INVALID_TOTAL_CHUNKS						= -12,
		LORA_OTA_STATUS_INCOMPLETE_DATA									= -11,
		LORA_OTA_STATUS_INVALID_DATA_HEADER							= -10,
		LORA_OTA_STATUS_INVALID_INPUT										= -9,
    LORA_OTA_STATUS_INVALID_CHECK_STATE             = -8,
    LORA_OTA_STATUS_CHECK_CRC_FIRMWARE_ERROR        = -7,
    LORA_OTA_STATUS_SEQUENCE_ERROR                  = -6,
    LORA_OTA_STATUS_DECODE_FAILED                   = -5, 
    LORA_OTA_STATUS_INVALID_SESSION                 = -4, 
    LORA_OTA_STATUS_ENCODE_FAILED                   = -3,
    LORA_OTA_STATUS_NULL_POINTER                    = -2,
    LORA_OTA_STATUS_BUFFER_OVERFLOW                 = -1,
		LORA_OTA_STATUS_INVALID_DATA_FIRMWARE						= 0,
    // Trạng thái thành công (dương, từ 1 trở lên để tránh nhầm lẫn với 0)
    LORA_OTA_STATUS_CREATE_START_SUCCESS            = 1,
    LORA_OTA_STATUS_CREATE_HEADER_SUCCESS           = 2,
    LORA_OTA_STATUS_CREATE_DATA_SUCCESS             = 3,
    LORA_OTA_STATUS_CREATE_RESPONSE_SUCCESS         = 4,
    LORA_OTA_STATUS_CREATE_SIGNAL_UPDATE_SUCCESS    = 5,
    LORA_OTA_STATUS_CREATE_END_SUCCESS              = 6,
    LORA_OTA_STATUS_PROCESS_START_SUCCESS           = 7,
    LORA_OTA_STATUS_PROCESS_HEADER_SUCCESS          = 8,
    LORA_OTA_STATUS_PROCESS_END_SUCCESS             = 9,
    LORA_OTA_STATUS_PROCESS_SIGNAL_UPDATE_SUCCESS   = 10,
    LORA_OTA_STATUS_PROCESS_DATA_SUCCESS            = 11
 
} LORA_OtaStatus_t;

typedef enum {
    OTA_NODE_IDLE = 0,
    OTA_NODE_WAIT_START,
    OTA_NODE_WAIT_HEADER,
    OTA_NODE_RECEIVING_DATA,
    OTA_NODE_WAIT_END,
    OTA_NODE_WAIT_SIGNAL_UPDATE,
    OTA_NODE_DONE,
    OTA_NODE_ERROR
} OTA_Node_State_t;

/* OTA_HEADER: exactly 16 bytes */
typedef struct {
    uint16_t session_id;        /* GW-defined session, increments per OTA */
    uint16_t fw_size;           /* total firmware size in bytes            */
    uint16_t chunk_size;        /* bytes per data chunk (e.g., 128/192)    */
    uint16_t total_chunks;      /* total number of chunks                  */
    uint32_t fw_crc;            /* CRC32 of the entire image (post-write)  */
    // uint16_t reserved0;         /* reserved (keep 0), for future use       */
} ota_header_t;

typedef struct {
    uint16_t expected_session_id;        /* GW-defined session, increments per OTA */
    uint16_t expected_fw_size;
    OTA_Node_State_t expected_state;
    uint8_t *expected_fw_buffer;
    uint16_t expected_chunk_size;
    uint16_t expected_total_chunks;
    uint16_t expected_seq;
    uint32_t expected_crc;
    uint16_t received_bytes;
} OTA_NodeContext_t;

extern OTA_NodeContext_t ota_node;

LORA_OtaStatus_t otaNodeProcessFrame(const LORA_frame_t *frame);
LORA_OtaStatus_t loRaOtaReceiveHandler(uint8_t message_type, uint8_t *payload, uint16_t payload_len);
void otaNodeInit(); 


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // LORA_OTA_H

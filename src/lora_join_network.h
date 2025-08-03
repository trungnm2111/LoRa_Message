
#ifndef LORA_JOIN_NETWORK_H
#define LORA_JOIN_NETWORK_H

#include <stdint.h>
#include "lora.h"
// #include "lora_node_storage.h"

/*
 * Lora Join Network status 
 */
typedef enum
{
    JOIN_STATUS_CHECK_NULL = -6,
    LORA_JOIN_STATUS_INVALID_MAC,
    LORA_JOIN_STATUS_INVALID_KEY,           // NEW
    LORA_JOIN_STATUS_MAC_MISMATCH,
    LORA_JOIN_REQUEST_STATUS_ENCODE_FAILED,
    LORA_JOIN_ACCEPT_STATUS_ENCODE_FAILED,
    LORA_JOIN_STATUS_DECODE_FAILED,
    LORA_JOIN_STATUS_ADD_NODE_FAILED,
    LORA_JOIN_STATUS_INVALID_PAYLOAD_LENGTH,
    LORA_JOIN_STATUS_CREATE_JOIN_REQUEST_SUCCESS,
    LORA_JOIN_STATUS_DECODE_JOIN_REQUEST_SUCCESS,
    LORA_JOIN_STATUS_GET_JOIN_REQUEST_SUCCESS,
    LORA_JOIN_ACCEPT_STATUS_DECODE_SUCCESS,
    LORA_JOIN_STATUS_CREATE_JOIN_ACCEPT_SUCCESS,
	LORA_JOIN_STATUS_SUCCESS
} LORA_Join_Network_Status_t;


/*
 * LoRa Join Network Status - Cải tiến với giá trị rõ ràng và debug-friendly
 */
typedef enum
{
    // Lỗi nghiêm trọng (âm, từ -10 trở xuống để dễ phân biệt)
    LORA_JOIN_STATUS_NULL_POINTER = -10,
    LORA_JOIN_STATUS_INVALID_MAC = -9,
    LORA_JOIN_STATUS_INVALID_KEY = -8,
    LORA_JOIN_STATUS_MAC_MISMATCH = -7,
    LORA_JOIN_STATUS_INVALID_PAYLOAD_LENGTH = -6,
    LORA_JOIN_STATUS_ENCODE_FAILED = -5,
    LORA_JOIN_STATUS_DECODE_FAILED = -4,
    LORA_JOIN_STATUS_ADD_NODE_FAILED = -3,
    
    // Trạng thái thành công (dương, từ 1 trở lên để tránh nhầm lẫn với 0)
    LORA_JOIN_STATUS_SUCCESS = 1,
    LORA_JOIN_STATUS_CREATE_REQUEST_SUCCESS = 2,
    LORA_JOIN_STATUS_DECODE_REQUEST_SUCCESS = 3,
    LORA_JOIN_STATUS_GET_REQUEST_SUCCESS = 4,
    LORA_JOIN_STATUS_CREATE_ACCEPT_SUCCESS = 5,
    LORA_JOIN_STATUS_DECODE_ACCEPT_SUCCESS = 6
    
} LORA_Join_Network_Status_t;

typedef enum
{
	LORA_JOIN_TYPE_REQUEST = 1,
	LORA_JOIN_TYPE_ACCEPT,
	LORA_JOIN_TYPE_CONFIRM,
	LORA_JOIN_TYPE_COMPLETED,
} LORA_Join_Type_t;

// 1. Join Request: chỉ MAC Address (6 bytes)
typedef struct {
    uint8_t mac_address[6];
} __attribute__((packed)) LORA_join_request_t;


// 1. Join Accept: chỉ MAC Address (6 bytes)
typedef struct {
    uint8_t mac_address[6];
    uint8_t net_key[16];
} __attribute__((packed)) LORA_join_accept_t;


// typedef struct
// {
// 	uint32_t mac[2]; // MAC address
// 	uint32_t netkey[4];
// 	uint32_t devkey[4];
// 	uint8_t unicast;
// 	uint8_t is_joined; // true if joined to the network
// } LORA_join_info_t;


// Main join message handler
LORA_Join_Network_Status_t loRaJoinMessageHandler(uint8_t message_type, uint8_t *payload, uint16_t payload_len);

// === JOIN REQUEST FUNCTIONS ===

// Node side: Create và send join request
LORA_Join_Network_Status_t loRaCreateJoinRequest(const uint8_t *mac_address, uint8_t *frame_buffer, uint16_t *frame_length);

// Gateway side: Decode join request
LORA_Join_Network_Status_t loRaProcessJoinRequest(const uint8_t *payload_data, uint16_t payload_len, uint8_t *response_mac);

LORA_Join_Network_Status_t loRaJoinNetworkGetRequest(uint8_t *payload_data, uint16_t payload_len);

#endif // LORA_SEND_MESSAGE_H

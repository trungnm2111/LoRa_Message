#ifndef LORA_H
#define LORA_H

#include "stdio.h"
#include "stdint.h"
#include <stdlib.h>
#include "string.h"
#include "crc.h"

#define LORA_SOF 0xAA // Start of Frame
#define LORA_EOF 0xBB // End of Frame

#define LORA_DATA_OVERHEAD_SIZE       10 // 1B SOF + 2B Type + 2B Length + 4B CRC + 1B EOF
#define LORA_DATA_MAX_SIZE            128
#define LORA_PACKET_MAX_SIZE          (LORA_DATA_MAX_SIZE + LORA_DATA_OVERHEAD_SIZE)
#define LORA_FRAME_HEADER_SIZE        5 // 1B SOF + 2B Type + 2B Length
// /*
//  * Lora type message
//  */
// typedef enum
// {
// 	LORA_PACKET_TYPE_JOIN = 0,
// 	LORA_PACKET_TYPE_OTA,
// 	LORA_PACKET_TYPE_DATA_SENSOR,
// 	LORA_PACKET_TYPE_ACK
// } LORA_Packet_Type_t;

/*
 * Lora Frame status 
 */
typedef enum
{
  LORA_STATUS_DECODE_FRAME_INVALID_EOF = -4,
  LORA_STATUS_DECODE_FRAME_INVALID_CRC,
	LORA_STATUS_ENCODE_FRAME_OVERLOAD_DATA,
  LORA_STATUS_DECODE_FRAME_ERROR ,
	LORA_STATUS_ENCODE_FRAME_ERROR ,
  LORA_STATUS_DECODE_FRAME_SUCCESS,
	LORA_STATUS_ENCODE_FRAME_SUCCESS,
	LORA_STATUS_GET_FSM_DONE,
	LORA_STATUS_GET_FSM_IN_PROGRESS,	
} LORA_Frame_Status_t;

// /*
//  * Lora packet type join
//  */
// typedef enum
// {
// 	LORA_JOIN_TYPE_REQUEST = 0,
// 	LORA_JOIN_TYPE_ACCEPT,
// 	LORA_JOIN_TYPE_CONFIRM,
// 	LORA_JOIN_TYPE_COMPLETED,
// } LORA_Join_Type_t;

// /*
//  * Lora packet type OTA
//  */
// typedef enum
// {
// 	LORA_OTA_TYPE_START = 0,
// 	LORA_OTA_TYPE_HEADER,
// 	LORA_OTA_TYPE_DATA,
// 	LORA_OTA_TYPE_END,
// 	LORA_OTA_TYPE_RESPONSE,
// } LORA_Ota_Type_t;

// /*
//  * Lora packet type data sensor
//  */
// typedef enum
// {
//     LORA_SENSOR_TYPE_TEMPERATURE,
//     LORA_SENSOR_TYPE_HUMIDITY,
//     LORA_SENSOR_TYPE_PRESSURE,
//     LORA_SENSOR_TYPE_GPS,
//     LORA_SENSOR_TYPE_ACK
// } LORA_Sensor_Type_t;

/**
 * @brief Trạng thái của FSM khi nhận frame LoRa
 */
typedef enum
{
    LORA_FSM_STATE_START = 0,
    LORA_FSM_STATE_TYPE,
    LORA_FSM_STATE_LENGTH,
    LORA_FSM_STATE_DATA,
    LORA_FSM_STATE_CRC,
    LORA_FSM_STATE_END
} LoraFsmState_t;



/**
 * @brief Bộ FSM xử lý nhận frame LoRa
 */
typedef struct
{
    LoraFsmState_t state;                  		  /**< Trạng thái hiện tại của FSM */
    uint8_t buffer[LORA_PACKET_MAX_SIZE];      /**< Bộ đệm lưu frame nhận được */
    uint16_t index;                            /**< Chỉ số hiện tại trong buffer */
    uint16_t expected_data_length;             /**< Độ dài payload được khai báo trong header */
    volatile int is_done;                      /**< Cờ cho biết FSM đã hoàn thành việc nhận frame */
} LoraFsm_t;

typedef struct
{
	uint32_t mac[2]; // MAC address
	uint32_t netkey[4];
	uint32_t devkey[4];
	uint8_t unicast;
	uint8_t is_joined; // true if joined to the network
} LORA_join_info_t;

extern LORA_join_info_t gSysParam;

typedef struct
{
	uint32_t mac[2]; // MAC address
	uint32_t netkey[4];
	uint32_t devkey[4];
	uint8_t unicast; // Unicast address
} LORA_node_info_t;

/*
 * ________________________________________________
 * |     | Packet  Type   |     |     |     |     |
 * | SOF | + Message Type | Len | DATA| CRC | EOF |
 * |_____|________________|_____|_____|_____|_____|
 *   1B          2B			    2B     NB    4B    1B
*/

typedef struct
{
  uint8_t   		sof;
  uint8_t  			packet_type;
  uint8_t  			message_type;
  uint16_t  		data_len;
  const uint8_t *   data_payload;
  uint32_t  		crc;
  uint8_t   		eof;
}__attribute__((packed)) LORA_frame_t;



extern LoraFsm_t lora_fsm_frame;

// extern uint8_t DATA_BUF[LORA_DATA_MAX_SIZE];
extern uint8_t mock_data[LORA_PACKET_MAX_SIZE];
extern uint16_t mock_length;

void loRaInitFsmFrame(void);
void lora_send_frame(uint8_t *data, uint16_t length);
LORA_Frame_Status_t loRaEncodeFrame(LORA_frame_t *input, uint8_t * frame_buffer);
void loRaGetFsmFrame(uint8_t byte);
LORA_Frame_Status_t loRaDecodeFrame(uint8_t *input_frame_buffer, uint16_t length, LORA_frame_t *output);

#endif // LORA_H

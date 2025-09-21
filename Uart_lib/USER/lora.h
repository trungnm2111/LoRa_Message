	#ifndef LORA_H
#define LORA_H

#include "stdio.h"
#include "stdint.h"
#include <stdlib.h>
#include "string.h"
#include "lora_join_network.h"
#include "crc.h"
#include "uart.h"

#define LORA_SOF 0xAA // Start of Frame
#define LORA_EOF 0xBB // End of Frame

#define LORA_DATA_OVERHEAD_SIZE       10 // 1B SOF + 2B Type + 2B Length + 4B CRC + 1B EOF
#define LORA_DATA_MAX_SIZE            140
#define LORA_PACKET_MAX_SIZE          150
#define LORA_FRAME_HEADER_SIZE        5 // 1B SOF + 2B Type + 2B Length
#define LORA_FRAME_CRC_SIZE           4

/*
 * Lora type message
 */
typedef enum
{
	LORA_PACKET_TYPE_JOIN = 1,
	LORA_PACKET_TYPE_OTA,
	LORA_PACKET_TYPE_DATA_SENSOR,
	LORA_PACKET_TYPE_ACK
} LORA_Packet_Type_t;


/*
 * Lora Frame status 
 */
typedef enum
{
  LORA_STATUS_DECODE_FRAME_INVALID_LENGTH_INPUT2  = -10,
  LORA_STATUS_DECODE_FRAME_INVALID_SOF            = -9,
  LORA_STATUS_DECODE_FRAME_INVALID_LENGTH_INPUT1  = -8,
  LORA_STATUS_DECODE_FRAME_INVALID_TYPE_INPUT     = -7, 
  LORA_STATUS_DECODE_FRAME_JOIN_REQUEST_ERROR     = -6,
  LORA_STATUS_DECODE_FRAME_INVALID_EOF            = -5,
  LORA_STATUS_ENCODE_FRAME_JOIN_REQUEST_ERROR ,
  LORA_STATUS_DECODE_FRAME_INVALID_CRC ,
	LORA_STATUS_ENCODE_FRAME_OVERLOAD_DATA,
  LORA_STATUS_DECODE_FRAME_ERROR ,
	LORA_STATUS_ENCODE_FRAME_ERROR ,
  LORA_STATUS_DECODE_FRAME_SUCCESS,
	LORA_STATUS_ENCODE_FRAME_SUCCESS,
	LORA_STATUS_GET_FSM_DONE,
	LORA_STATUS_GET_FSM_IN_PROGRESS,	
} LORA_Frame_Status_t;


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
    LORA_FSM_STATE_START 	= 0,
    LORA_FSM_STATE_TYPE		= 1,
    LORA_FSM_STATE_LENGTH	= 2,
    LORA_FSM_STATE_DATA		= 3,
    LORA_FSM_STATE_CRC 		= 4,
    LORA_FSM_STATE_END		= 5
} LoraFsmState_t;


/**
 * @brief Bộ FSM xử lý nhận frame LoRa
 */
typedef struct
{
		LoraFsmState_t state ; /**< Trạng thái hiện tại của FSM */
    uint16_t frame_length;
    uint16_t index;                            /**< Chỉ số hiện tại trong buffer */
		uint16_t save_length;            		 				/**< Độ dài payload được khai báo trong header */
	  uint8_t buffer[150];      									/**< Bộ đệm lưu frame nhận được */
} LoraFsm_t;


extern LoraFsm_t lora_fsm_frame;

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
  uint8_t *     data_payload;
  uint32_t  		crc;
  uint8_t   		eof;
}__attribute__((packed))LORA_frame_t;



// extern uint8_t DATA_BUF[LORA_DATA_MAX_SIZE];

void loRaInitFsmFrame(LoraFsm_t *fsm);
void lora_send_frame(uint8_t *data, uint16_t length);
LORA_Frame_Status_t loRaEncryptedFrame(LORA_frame_t *input, uint8_t * frame_buffer);
uint8_t loRaGetFsmFrame(LoraFsm_t *fsm, uint8_t byte);
LORA_Frame_Status_t loRaDecodeFrame(uint8_t *input_frame_buffer, uint16_t length, LORA_frame_t *output);
uint16_t swap_endian16(uint16_t num);
uint32_t swap_endian32(uint32_t val);
#endif // LORA_H

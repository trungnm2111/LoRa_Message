#ifndef LORA_H
#define LORA_H

#include <stdint.h>
#include "crc.h"

#define LORA_SOF 0xAA // Start of Frame
#define LORA_EOF 0xBB // End of Frame

#define LORA_DATA_OVERHEAD_SIZE       10 // 1B SOF + 2B Type + 2B Length + 4B CRC + 1B EOF
#define LORA_DATA_MAX_SIZE            128
#define LORA_PACKET_MAX_SIZE          (LORA_DATA_MAX_SIZE + LORA_DATA_OVERHEAD_SIZE)
#define LORA_FRAME_HEADER_SIZE        5 // 1B SOF + 2B Type + 2B Length
/*
 * Lora type message
 */
typedef enum
{
	LORA_PACKET_TYPE_JOIN,
	LORA_PACKET_TYPE_OTA,
	LORA_PACKET_TYPE_DATA_SENSOR,
	LORA_PACKET_TYPE_ACK
} LORA_Packet_Type_t;


/*
 * Lora packet type join
 */
typedef enum
{
	LORA_JOIN_TYPE_REQUEST,
	LORA_JOIN_TYPE_ACCEPT,
	LORA_JOIN_TYPE_CONFIRM,
	LORA_JOIN_TYPE_COMPLETED,
} LORA_Join_Type_t;

/*
 * Lora packet type OTA
 */
typedef enum
{
	LORA_OTA_TYPE_START,
	LORA_OTA_TYPE_HEADER,
	LORA_OTA_TYPE_DATA,
	LORA_OTA_TYPE_END,
	LORA_OTA_TYPE_RESPONSE,
} LORA_Ota_Type_t;

/*
 * Lora packet type data sensor
 */
typedef enum
{
    LORA_SENSOR_TYPE_TEMPERATURE,
    LORA_SENSOR_TYPE_HUMIDITY,
    LORA_SENSOR_TYPE_PRESSURE,
    LORA_SENSOR_TYPE_GPS,
    LORA_SENSOR_TYPE_ACK
} LORA_Sensor_Type_t;

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
 * ________________________________________
 * |     | Packet |     |     |     |     |
 * | SOF | Type   | Len | DATA| CRC | EOF |
 * |_____|________|_____|_____|_____|_____|
 *   1B      2B     2B    NB     4B    1B
*/

typedef struct
{
  uint8_t   sof;
  uint16_t  packet_type;
  uint16_t  data_len;
  uint8_t   data;
  uint32_t  crc;
  uint8_t   eof;
}__attribute__((packed)) LORA_frame_t;



extern uint8_t mock_data[LORA_PACKET_MAX_SIZE];
extern uint16_t mock_length;

void lora_send_frame(uint8_t *data, uint16_t length);

#endif // LORA_H

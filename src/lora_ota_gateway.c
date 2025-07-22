#include "lora_ota_gateway.h"
#include <string.h>
// #include "rtvars.h"
// #include "lora.h"
// #include "help_convert_datatype.h"
// #include "hld_crc.h"
// #include "ota.h"


LORA_Fsm_Frame_Data_t lora_frame_status = LORA_FRAME_START;

// static void lora_init_frame(void);
static void lora_check_frame(void);


/*FSM Message*/
void lora_get_frame(uint8_t temp_char)
{
    switch (lora_frame_status)
    {
			/*---------------------------------(Start (1byte))---------------------------------*/
			case LORA_FRAME_START:
					lora_init_frame();
					if(temp_char == LORA_SOF)
					{
						// Start of Frame
						gLoraData.Buffer[gLoraData.Index++] = temp_char;
						lora_frame_status = LORA_FRAME_TYPE;
					}
					else
					{
						lora_init_frame();
#if DEBUG_ERROR
						PRINTF("Start of Frame mismatch in get frame\n");
#endif
					}
			break;
			/*---------------------------------(Type Message (2byte))---------------------------------*/
			case LORA_FRAME_TYPE:
					gLoraData.Buffer[gLoraData.Index++] = temp_char;
					if(gLoraData.Index >= 3)
					{
						lora_frame_status = LORA_FRAME_LENGTH;
					}
			break;
			/*---------------------------------(Length (2byte))---------------------------------*/
			case LORA_FRAME_LENGTH:
					gLoraData.Buffer[gLoraData.Index++] = temp_char;
					if (gLoraData.Index >= 5)
					{
							uint16_t tempData = help_convert_byte_to_uint16(gLoraData.Buffer[3], gLoraData.Buffer[4]);
							gLoraData.Length = tempData;
							lora_frame_status = LORA_FRAME_DATA;
					}
			break;
			/*---------------------------------(Data (Nbyte))---------------------------------*/
			case LORA_FRAME_DATA:
					gLoraData.Buffer[gLoraData.Index++] = temp_char;
					if ((gLoraData.Index - 5) >= gLoraData.Length)
					{
							lora_frame_status = LORA_FRAME_CRC;
					}
			break;
			/*---------------------------------(Check Sum(4byte))---------------------------------*/
			case LORA_FRAME_CRC:
					gLoraData.Buffer[gLoraData.Index++] = temp_char;
					if ((gLoraData.Index - 5 - gLoraData.Length) >= 4)
					{	
							uint32_t tempData = help_convert_bytes_to_uint32(gLoraData.Buffer[gLoraData.Index - 4], gLoraData.Buffer[gLoraData.Index - 3], gLoraData.Buffer[gLoraData.Index - 2], gLoraData.Buffer[gLoraData.Index - 1]);
							//Calculate the received data's CRC
							uint32_t cal_data_crc = help_calculate_crc_bytes(gLoraData.Buffer, gLoraData.Index - 4);
							if(tempData == cal_data_crc) 
							{
								lora_frame_status = LORA_FRAME_END;
							}
							else 
							{
								lora_init_frame();
#if DEBUG_ERROR
								PRINTF("Chunk's CRC mismatch in get frame\n");
								PRINTF("cal_data_crc: %x\n", cal_data_crc);
								PRINTF("received data_crc: %x\n", tempData);
#endif
							}
					}
			break;
			/*---------------------------------(End(1byte))---------------------------------*/
			case LORA_FRAME_END:
					if(temp_char == LORA_EOF)
					{
						// gSysParam.frame_ready_flag = true;
						gSysParam.is_joined = 1;
						PRINTF("end fsm ");
						gLoraData.Buffer[gLoraData.Index] = temp_char;
						lora_frame_status = LORA_FRAME_START;
						lora_check_frame();
					}
			break;
			default:
				break;
    }
}

void lora_init_frame(void)
{
	gLoraData.Index = 0;
	gLoraData.Length = 0;
	lora_frame_status = LORA_FRAME_START;
	// Clear the buffer
	memset(gLoraData.Buffer, 0, LORA_PACKET_MAX_SIZE);
}

static void lora_check_frame()
{
	// PRINTF("LORA Frame: ");
	// Check if the frame is valid
	for (uint16_t i = 0; i <= gLoraData.Index; i++)
	{
		// PRINTF("%02X ", gLoraData.Buffer[i]);
	}
	// PRINTF("\r\n");
	if(gLoraData.Buffer[0] == LORA_SOF && gLoraData.Buffer[gLoraData.Index] == LORA_EOF)
	{
		if(gLoraData.Buffer[1] == LORA_PACKET_TYPE_JOIN)
		{
			switch (gLoraData.Buffer[2])
			{
			case LORA_JOIN_TYPE_ACCEPT:
				/* code */
				break;
			case LORA_JOIN_TYPE_COMPLETED:
				/* code */
				break;
			
			default:
				break;
			}
		}
		else if(gLoraData.Buffer[1] == LORA_PACKET_TYPE_OTA)
		{
			// ota_decode_frame();
		}
		else if(gLoraData.Buffer[1] == LORA_PACKET_TYPE_DATA_SENSOR)
		{
			switch (gLoraData.Buffer[2])
			{
    
			case LORA_SENSOR_TYPE_ACK:
				/* code */
				break;
			default:
				break;
			}
		}
	}
}



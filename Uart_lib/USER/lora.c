#include "lora.h"
#include <stddef.h>
#include <string.h>

LoraFsmState_t state = LORA_FSM_STATE_START;
LoraFsm_t lora_fsm_frame;

uint16_t help_convert_byte_to_uint16(uint8_t low_byte, uint8_t high_byte);
uint32_t help_convert_bytes_to_uint32(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);


/**
 * @brief Mã hóa một khung dữ liệu LoRa từ cấu trúc `LORA_frame_t` vào buffer truyền.
 *
 * Hàm này thực hiện việc đóng gói một khung dữ liệu LoRa bao gồm:
 *  - Start-of-Frame (SOF)
 *  - Loại gói và loại bản tin
 *  - Độ dài dữ liệu (2 byte little-endian)
 *  - Payload dữ liệu
 *  - CRC kiểm tra toàn bộ frame (ngoại trừ EOF)
 *  - End-of-Frame (EOF)
 *
 * Hàm kiểm tra tính hợp lệ của các tham số đầu vào, đồng thời giới hạn kích thước
 * dữ liệu payload không vượt quá `LORA_DATA_MAX_SIZE`.
 *
 * @param[in] input         Con trỏ tới cấu trúc `LORA_frame_t` chứa thông tin frame cần mã hóa.
 * @param[out] frame_buffer Bộ đệm đích (cần đủ lớn) để chứa frame đã mã hóa.
 *
 * @return Một giá trị thuộc kiểu `LORA_Frame_State_t`, cụ thể:
 *         - `LORA_STATUS_ENCODE_FRAME_SUCCESS` nếu mã hóa thành công.
 *         - `LORA_STATUS_ENCODE_FRAME_ERROR` nếu con trỏ đầu vào NULL hoặc payload NULL.
 *         - `LORA_STATUS_ENCODE_FRAME_OVERLOAD_DATA` nếu `data_len` vượt quá `LORA_DATA_MAX_SIZE`.
 */
LORA_Frame_Status_t loRaEncryptedFrame(LORA_frame_t *input, uint8_t * frame_buffer)
{
    if (input == NULL || frame_buffer == NULL || input->data_payload == NULL)
    {
        return LORA_STATUS_ENCODE_FRAME_ERROR;
    }

    if (input->data_len > LORA_DATA_MAX_SIZE)
    {
        return LORA_STATUS_ENCODE_FRAME_OVERLOAD_DATA;
    }
    input->sof = LORA_SOF;
    input->eof = LORA_EOF;
    
    // Ghi các trường header vào buffer
    frame_buffer[0] = input->sof;
    frame_buffer[1] = input->packet_type;
    frame_buffer[2] = input->message_type;

    // Ghi data_len (big-endian)
    frame_buffer[3] = (uint8_t)((input->data_len >> 8) & 0xFF);
    frame_buffer[4] = (uint8_t)(input->data_len & 0xFF);

    // Ghi payload vào sau header
    uint16_t offset = LORA_FRAME_HEADER_SIZE;
    memcpy(&frame_buffer[offset], input->data_payload, input->data_len);
    offset += input->data_len;

     // Tính CRC từ đầu buffer tới trước CRC
    crc_t crc = crc_slow(frame_buffer, offset);
    memcpy(&frame_buffer[offset], &crc, sizeof(crc));
    offset += sizeof(crc);

     // Thêm EOF
    frame_buffer[offset++] = input->eof;

    return LORA_STATUS_ENCODE_FRAME_SUCCESS;
}


LORA_Frame_Status_t loRaDecodeFrame(uint8_t *input_frame_buffer, uint16_t length, LORA_frame_t *output) 
{
    //|| (fsm->is_done != 1)
    if (!input_frame_buffer || !output || length == 0) {
        return LORA_STATUS_DECODE_FRAME_INVALID_TYPE_INPUT;
    }

    // Kiểm tra độ dài tối thiểu: SOF + packet_type + message_type + data_len(2 byte) + CRC(2 byte) + EOF
    if (length < LORA_FRAME_HEADER_SIZE + sizeof(crc_t) + 1) 
		{
        return LORA_STATUS_DECODE_FRAME_INVALID_LENGTH_INPUT1;
    }

    uint16_t index = 0;

    // Kiểm tra SOF với bounds check
    if (index >= length || input_frame_buffer[index++] != LORA_SOF) 
		{
        return LORA_STATUS_DECODE_FRAME_INVALID_SOF;
    }
	
    output->sof = LORA_SOF;
		
		if (index + 3 >= length) { // Cần ít nhất 4 bytes cho packet_type + message_type + data_len
        return LORA_STATUS_DECODE_FRAME_INVALID_LENGTH_INPUT1;
    }
		
    // Đọc header
    output->packet_type = input_frame_buffer[index++];
    output->message_type = input_frame_buffer[index++];
    output->data_len = ((uint16_t)input_frame_buffer[index++] << 8);  // Byte cao
    output->data_len |= input_frame_buffer[index++];    

    // Kiểm tra nếu length không đủ để chứa payload + CRC + EOF
    if (length < LORA_FRAME_HEADER_SIZE + output->data_len + sizeof(crc_t) + 1) {
        return LORA_STATUS_DECODE_FRAME_INVALID_LENGTH_INPUT2;
    }

		if (index + output->data_len > length) {
        return LORA_STATUS_DECODE_FRAME_INVALID_LENGTH_INPUT2;
    }
		
    // Gán con trỏ đến payload trong buffer (không copy)
    output->data_payload = &input_frame_buffer[index];
    index += output->data_len;

    // Đọc CRC theo Big Endian (giống data_len)
    crc_t received_crc = 0;
    received_crc = ((uint32_t)input_frame_buffer[index++] << 24);  // Byte cao nhất
    received_crc |= ((uint32_t)input_frame_buffer[index++] << 16); // Byte cao
    received_crc |= ((uint32_t)input_frame_buffer[index++] << 8);  // Byte thấp
    received_crc |= input_frame_buffer[index++];                  // Byte thấp nhất

    // Tính CRC lại từ đầu đến trước CRC
    crc_t calculated_crc = crc_slow(input_frame_buffer, index - sizeof(crc_t));
    output->crc = calculated_crc;
		
    if (received_crc != calculated_crc) {
        return LORA_STATUS_DECODE_FRAME_INVALID_CRC;
    }
    // Kiểm tra EOF
    if (index >= length || input_frame_buffer[index++] != LORA_EOF) {
        return LORA_STATUS_DECODE_FRAME_INVALID_EOF;
    }	
		
    output->eof = LORA_EOF;
		
		if (index != length) {
        return LORA_STATUS_DECODE_FRAME_INVALID_LENGTH_INPUT2;
    }
		
    switch (output->packet_type)
    {
        case LORA_PACKET_TYPE_JOIN:
            // loRaJoinPacketHandler(output->message_type, output->data_payload, output->data_len);
            break;
        
        case LORA_PACKET_TYPE_OTA:
        default:
            break;
    }
    return LORA_STATUS_DECODE_FRAME_SUCCESS;
}


/*FSM Message*/
uint8_t loRaGetFsmFrame(LoraFsm_t *fsm, uint8_t byte)
{
		// Safety check
    if(fsm->index >= LORA_PACKET_MAX_SIZE) 
		{ 
				Usart_Send_Char('e');
				Usart_SendNumber(fsm->index);
				loRaInitFsmFrame(fsm);
        return 0;
    }
		
    switch (fsm->state)
    {
			/*---------------------------------(Start (1byte))---------------------------------*/
			case LORA_FSM_STATE_START:		
//					Usart_Send_Char('S');
					if(byte == LORA_SOF)				// Start of Frame
					{
						for(int i = 0; i < LORA_PACKET_MAX_SIZE; i++) 
						{
							fsm->buffer[i] = 0;
						}
						fsm->index = 0; 
						fsm->buffer[fsm->index++] = byte;
						fsm->state = LORA_FSM_STATE_TYPE;				
					}		
			break;

			/*---------------------------------(Type Message (2byte))---------------------------------*/
			case LORA_FSM_STATE_TYPE:
//					Usart_Send_Char('T');
					fsm->buffer[fsm->index++] = byte;
			
					if(fsm->index == 3)
					{
						fsm->state = LORA_FSM_STATE_LENGTH;
					}
			break;
					
			/*---------------------------------(Length (2byte))---------------------------------*/
			case LORA_FSM_STATE_LENGTH:
//					Usart_Send_Char('L');
					fsm->buffer[fsm->index++] = byte;
			
					if (fsm->index == 5)
					{
							fsm->frame_length = help_convert_byte_to_uint16(fsm->buffer[3], fsm->buffer[4]);

							if(fsm->frame_length < 0 || fsm->frame_length > 140)
							{
								loRaInitFsmFrame(fsm);
							}
							else 
							{
								fsm->state = LORA_FSM_STATE_DATA;
							}
					}
					
			break;
					
			/*---------------------------------(Data (Nbyte))---------------------------------*/
			case LORA_FSM_STATE_DATA:
//					Usart_Send_Char('D');
			
					if(fsm->index < LORA_PACKET_MAX_SIZE) { // Check bounds
						fsm->buffer[fsm->index++] = byte;
					}
					
					if (fsm->index == fsm->frame_length + 5)
					{
						fsm->state = LORA_FSM_STATE_CRC;  
					}
					
			break;
					
			/*---------------------------------(Check Sum(4byte))---------------------------------*/
			case LORA_FSM_STATE_CRC:
////					Usart_Send_Char('C');
					fsm->buffer[fsm->index++] = byte;
			
					if ((fsm->index - 5 - fsm->frame_length) == sizeof(crc_t))
					{	
						crc_t received_crc = help_convert_bytes_to_uint32(		
						fsm->buffer[fsm->index - 4],	
						fsm->buffer[fsm->index - 3],
						fsm->buffer[fsm->index - 2],
						fsm->buffer[fsm->index - 1]);							
				 															
						crc_t calculated_crc  = crc_slow(fsm->buffer, fsm->index - sizeof(crc_t));			//Calculate the received data's CRC
						
						if (received_crc == calculated_crc)
						{
							fsm->state = LORA_FSM_STATE_END;
						}
						else 
						{
							loRaInitFsmFrame(fsm); 
						}
					}
			break;
					
			/*---------------------------------(End(1byte))---------------------------------*/
			case LORA_FSM_STATE_END:
//					Usart_Send_Char('N');
					if(byte == LORA_EOF)
					{
						fsm->buffer[fsm->index++] = byte;
						fsm->save_length = fsm->index;
						return 1;
					}
					else 
					{
						loRaInitFsmFrame(fsm);
					}
			break;
					
			default:
				Usart_Send_Char('x');
				loRaInitFsmFrame(fsm); 
			break;
    }
	return 0;
}


void loRaInitFsmFrame(LoraFsm_t *fsm)
{
	fsm->state = LORA_FSM_STATE_START;
	fsm->index = 0;
	fsm->frame_length = 0;
}

uint16_t help_convert_byte_to_uint16(uint8_t high_byte, uint8_t low_byte )
{
    return ((uint16_t)high_byte << 8) | low_byte;
}

uint32_t help_convert_bytes_to_uint32(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3)
{
    return ((uint32_t)byte0 << 24) | ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | byte3;
}

uint16_t swap_endian16(uint16_t num) 
{
    return ((num >> 8)  & 0x00FF) |   // Byte 0 -> Byte 1
           ((num << 8)  & 0xFF00);    			// Byte 1 -> Byte 0
}

uint32_t swap_endian32(uint32_t val) {
    return ((val >> 24) & 0x000000FF) |
           ((val >> 8)  & 0x0000FF00) |
           ((val << 8)  & 0x00FF0000) |
           ((val << 24) & 0xFF000000);
}

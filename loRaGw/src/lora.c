#include "lora.h"
#include <stddef.h>
#include <string.h>

uint16_t help_convert_byte_to_uint16(uint8_t low_byte, uint8_t high_byte);
uint32_t help_convert_bytes_to_uint32(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);
void loRaInitFsmFrame(void);


uint8_t mock_data[LORA_PACKET_MAX_SIZE];
uint16_t mock_length;

// void lora_send_frame(uint8_t *data, uint16_t length)
// {
//     memcpy(mock_data, data, length);
//     mock_length = length;
// }

// void lora_send_frame(uint8_t *data, uint16_t length)
// {
//     // uart2_send_data(data, length);
// }

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

    // // Ghi data_len (little-endian)
    // frame_buffer[3] = (uint8_t)(input->data_len & 0xFF);
    // frame_buffer[4] = (uint8_t)((input->data_len >> 8) & 0xFF);
    // Ghi data theo Big endian
    output->data_len = ((uint16_t)input_frame_buffer[index++] << 8);  // Byte cao
    output->data_len |= input_frame_buffer[index++];                 // Byte thấp

    // Ghi payload vào sau header
    uint16_t offset = LORA_FRAME_HEADER_SIZE;
    memcpy(&frame_buffer[offset], input->data_payload, input->data_len);
    offset += input->data_len;

     // Tính CRC từ đầu buffer tới trước CRC
    crc_t crc = crc_fast(frame_buffer, offset);
    memcpy(&frame_buffer[offset], &crc, sizeof(crc));
    offset += sizeof(crc);

     // Thêm EOF
    frame_buffer[offset++] = input->eof;

    return LORA_STATUS_ENCODE_FRAME_SUCCESS;
}

LORA_Frame_Status_t loRaDecodeFrame(uint8_t *input_frame_buffer, uint16_t length, LORA_frame_t *output) 
{
    if (!input_frame_buffer || !output) {
        return LORA_STATUS_DECODE_FRAME_ERROR;
    }

    // Kiểm tra độ dài tối thiểu: SOF + packet_type + message_type + data_len(2 byte) + CRC(2 byte) + EOF
    if (length < LORA_FRAME_HEADER_SIZE + sizeof(crc_t) + 1) {
        return LORA_STATUS_DECODE_FRAME_ERROR;
    }

    uint16_t index = 0;

    // Kiểm tra SOF
    if (input_frame_buffer[index++] != LORA_SOF) {
        return LORA_STATUS_DECODE_FRAME_ERROR;
    }

    output->sof = LORA_SOF;

    // Đọc header
    output->packet_type = input_frame_buffer[index++];
    output->message_type = input_frame_buffer[index++];
    output->data_len = input_frame_buffer[index++];
    output->data_len |= ((uint16_t)input_frame_buffer[index++] << 8);

    // Kiểm tra nếu length không đủ để chứa payload + CRC + EOF
    if (length < LORA_FRAME_HEADER_SIZE + output->data_len + sizeof(crc_t) + 1) {
        return LORA_STATUS_DECODE_FRAME_ERROR;
    }

    // Gán con trỏ đến payload trong buffer (không copy)
    output->data_payload = &input_frame_buffer[index];

    index += output->data_len;

    // Đọc CRC
    crc_t received_crc = 0;
    memcpy(&received_crc, &input_frame_buffer[index], sizeof(crc_t));
    index += sizeof(crc_t);

    // Tính CRC lại từ đầu đến trước CRC
    crc_t calculated_crc = crc_fast(input_frame_buffer, index - sizeof(crc_t));
    if (received_crc != calculated_crc) {
        return LORA_STATUS_DECODE_FRAME_INVALID_CRC;
    }

    // Kiểm tra EOF
    if (input_frame_buffer[index++] != LORA_EOF) {
        return LORA_STATUS_DECODE_FRAME_INVALID_EOF;
    }
    output->eof = LORA_EOF;

    if (output < 0) {
        // Handle error
    }

    switch (output->packet_type)
    {
        case LORA_PACKET_TYPE_JOIN:
            // loRaJoinPacketHandler(output->message_type, output->data_payload, output->data_len);
            break;
        
        default:
            break;
    }
    return LORA_STATUS_DECODE_FRAME_SUCCESS;
}


LoraFsm_t lora_fsm_frame;
LoraFsmState_t state = LORA_FSM_STATE_START;


/*FSM Message*/
void loRaGetFsmFrame(uint8_t byte)
{
    
    switch (lora_fsm_frame.state)
    {
			/*---------------------------------(Start (1byte))---------------------------------*/
			case LORA_FSM_STATE_START:
					loRaInitFsmFrame();
					if(byte == LORA_SOF)
					{
						// Start of Frame
						lora_fsm_frame.buffer[lora_fsm_frame.index++] = byte;
						lora_fsm_frame.state = LORA_FSM_STATE_TYPE;
					}
					else
					{
						loRaInitFsmFrame();
					}
			break;

			/*---------------------------------(Type Message (2byte))---------------------------------*/
			case LORA_FSM_STATE_TYPE:
					lora_fsm_frame.buffer[lora_fsm_frame.index++] = byte;
					if(lora_fsm_frame.index >= 3)
					{
						lora_fsm_frame.state = LORA_FSM_STATE_LENGTH;
					}
			break;
			/*---------------------------------(Length (2byte))---------------------------------*/
			case LORA_FSM_STATE_LENGTH:
					lora_fsm_frame.buffer[lora_fsm_frame.index++] = byte;
					if (lora_fsm_frame.index >= 5)
					{
							lora_fsm_frame.expected_data_length = help_convert_byte_to_uint16(lora_fsm_frame.buffer[3], lora_fsm_frame.buffer[4]);
                            lora_fsm_frame.state = LORA_FSM_STATE_DATA;
					}
			break;
			/*---------------------------------(Data (Nbyte))---------------------------------*/
			case LORA_FSM_STATE_DATA:
					lora_fsm_frame.buffer[lora_fsm_frame.index++] = byte;
					if ((lora_fsm_frame.index - 5) >= lora_fsm_frame.expected_data_length)
					{
						lora_fsm_frame.state = LORA_FSM_STATE_CRC;  
					}
			break;
			/*---------------------------------(Check Sum(4byte))---------------------------------*/
			case LORA_FSM_STATE_CRC:
					lora_fsm_frame.buffer[lora_fsm_frame.index++] = byte;
                    if ((lora_fsm_frame.index - 5 - lora_fsm_frame.expected_data_length) >= sizeof(crc_t))
					{	
							crc_t received_crc = help_convert_bytes_to_uint32(
                                lora_fsm_frame.buffer[lora_fsm_frame.index - 4],
                                lora_fsm_frame.buffer[lora_fsm_frame.index - 3],
                                lora_fsm_frame.buffer[lora_fsm_frame.index - 2],
                                lora_fsm_frame.buffer[lora_fsm_frame.index - 1]);							//Calculate the received data's CRC
                             // Tính CRC từ đầu buffer tới trước CRC
                                                    
                            crc_t calculated_crc  = crc_fast(lora_fsm_frame.buffer, lora_fsm_frame.index - sizeof(crc_t));
							
                            if (received_crc == calculated_crc)
                            {
                                lora_fsm_frame.state = LORA_FSM_STATE_END;
                            }
							else 
							{
								loRaInitFsmFrame();
							}
					}
			break;
			/*---------------------------------(End(1byte))---------------------------------*/
			case LORA_FSM_STATE_END:
					if(byte == LORA_EOF)
					{
                        lora_fsm_frame.buffer[lora_fsm_frame.index++] = byte;
                        lora_fsm_frame.state = LORA_FSM_STATE_START;
						// lora_check_frame();
						lora_fsm_frame.is_done = 1;
					}
                    else
                    {
                        loRaInitFsmFrame();  // EOF không hợp lệ
                    }
			break;

			default:
				break;
    }
}


void loRaInitFsmFrame(void)
{
    lora_fsm_frame.index = 0;
    lora_fsm_frame.expected_data_length = 0;
    lora_fsm_frame.is_done = 0;
    lora_fsm_frame.state = LORA_FSM_STATE_START;
    memset(lora_fsm_frame.buffer, 0, LORA_PACKET_MAX_SIZE);
}

uint16_t help_convert_byte_to_uint16(uint8_t low_byte, uint8_t high_byte)
{
    return ((uint16_t)high_byte << 8) | low_byte;
}

uint32_t help_convert_bytes_to_uint32(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3)
{
    return ((uint32_t)byte3 << 24) | ((uint32_t)byte2 << 16) | ((uint32_t)byte1 << 8) | byte0;
}

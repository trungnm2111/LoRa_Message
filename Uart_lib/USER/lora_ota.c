/* lora_ota.h
 * OTA-over-LoRa protocol 
 * Author: TrungNM
 * Created: *********
*/

#include "lora_ota.h"

static uint8_t firmware_receive_buffer[10*1024];


OTA_NodeContext_t ota_node;

void otaNodeInit(void) 
{
    ota_node.expected_state = OTA_NODE_WAIT_START;
    ota_node.expected_fw_buffer = firmware_receive_buffer;
    ota_node.expected_fw_size = 0;
    ota_node.expected_chunk_size = 0;
    ota_node.expected_seq = 0;
    ota_node.expected_crc = 0;
    ota_node.received_bytes = 0;
}


LORA_OtaStatus_t loRaOtaReceiveHandler(uint8_t message_type, uint8_t *payload, uint16_t payload_len)
{
    // Kiểm tra input
    if (!payload && payload_len > 0) {
        return LORA_OTA_STATUS_INVALID_INPUT;
    }
    
    switch (message_type) {
    case OTA_TYPE_START:
        if (ota_node.expected_state != OTA_NODE_WAIT_START) {
            return LORA_OTA_STATUS_INVALID_CHECK_STATE;
        }
        
        // Reset tất cả các giá trị OTA
        memset(&ota_node, 0, sizeof(ota_node));
        ota_node.expected_state = OTA_NODE_WAIT_HEADER;
        ota_node.expected_fw_buffer = firmware_receive_buffer;
        return LORA_OTA_STATUS_PROCESS_START_SUCCESS;
    
    case OTA_TYPE_HEADER:
        if (ota_node.expected_state != OTA_NODE_WAIT_HEADER) {
            return LORA_OTA_STATUS_INVALID_CHECK_STATE;
        }

        if (payload_len < sizeof(ota_header_t)) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_DECODE_FAILED;
        }

        ota_header_t *hdr = (ota_header_t *)payload;
        
        // CRITICAL: Kiểm tra firmware size hợp lệ
				hdr->fw_size = swap_endian16(hdr->fw_size);
        if (hdr->fw_size == 0 || hdr->fw_size > OTA_MAX_FIRMWARE_SIZE) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_INVALID_FIRMWARE_SIZE;
        }
        
        // Kiểm tra chunk size hợp lệ
				hdr->chunk_size = swap_endian16(hdr->chunk_size);
        if (hdr->chunk_size == 0 || hdr->chunk_size > OTA_MAX_CHUNK_SIZE) {
            ota_node.expected_state = OTA_NODE_ERROR;
						Usart_SendNumber(hdr->chunk_size);
            return LORA_OTA_STATUS_INVALID_CHUNK_SIZE;
        }
        
        // Kiểm tra total chunks logic
				hdr->total_chunks = swap_endian16(hdr->total_chunks);
        uint16_t calculated_chunks = (hdr->fw_size + hdr->chunk_size - 1) / hdr->chunk_size;
        if (hdr->total_chunks != calculated_chunks) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_INVALID_TOTAL_CHUNKS;
        }
				hdr->fw_crc = swap_endian32(hdr->fw_crc);

        // Gán các giá trị sau khi validate
        ota_node.expected_session_id = hdr->session_id;
        ota_node.expected_fw_size = hdr->fw_size;
        ota_node.expected_chunk_size = hdr->chunk_size;
        ota_node.expected_crc = hdr->fw_crc;
        ota_node.expected_total_chunks = hdr->total_chunks;
        ota_node.expected_seq = 0;
        ota_node.received_bytes = 0;
				
        // Clear firmware buffer
        if (ota_node.expected_fw_buffer) {
            memset(ota_node.expected_fw_buffer, 0, OTA_MAX_FIRMWARE_SIZE);
        }

        ota_node.expected_state = OTA_NODE_RECEIVING_DATA;
        return LORA_OTA_STATUS_PROCESS_HEADER_SUCCESS;
    
    case OTA_TYPE_DATA:
        if (ota_node.expected_state != OTA_NODE_RECEIVING_DATA) {
            return LORA_OTA_STATUS_INVALID_CHECK_STATE;
        }
        
        if (payload_len < 2) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_DECODE_FAILED;
        }
        
        // Kiểm tra buffer pointer
        if (!ota_node.expected_fw_buffer) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_NULL_POINTER;
        }
        
        uint16_t seq = ((uint16_t)payload[0] << 8) | payload[1];
        uint16_t data_len = payload_len - 2;
        
        // Validate sequence (có thể bỏ comment nếu cần strict sequence)
        if (seq != ota_node.expected_seq) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_SEQUENCE_ERROR;
        }
        
        // CRITICAL: Kiểm tra bounds trước khi memcpy
        if (ota_node.received_bytes + data_len > ota_node.expected_fw_size) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_BUFFER_OVERFLOW;
        }
        
        // CRITICAL: Double check buffer bounds
        if (ota_node.received_bytes + data_len > OTA_FIRMWARE_BUFFER_SIZE) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_BUFFER_OVERFLOW;
        }
        
        // Kiểm tra data_len hợp lệ
        if (data_len == 0 || data_len > ota_node.expected_chunk_size) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_BUFFER_OVERFLOW;
        }

        // AN TOÀN: Copy data với bounds đã kiểm tra
        memcpy(ota_node.expected_fw_buffer + ota_node.received_bytes,
               &payload[2],
               data_len);
               
        ota_node.received_bytes += data_len;
        ota_node.expected_seq++;

        // Kiểm tra điều kiện hoàn thành
        if (ota_node.received_bytes >= ota_node.expected_fw_size || 
            ota_node.expected_seq >= ota_node.expected_total_chunks) {
            ota_node.expected_state = OTA_NODE_WAIT_END;
        }
        
        return LORA_OTA_STATUS_PROCESS_DATA_SUCCESS;

    case OTA_TYPE_END:
        if (ota_node.expected_state != OTA_NODE_WAIT_END) {
            return LORA_OTA_STATUS_INVALID_CHECK_STATE;
        }
        
        // Kiểm tra dữ liệu đã nhận đủ chưa
        if (ota_node.received_bytes != ota_node.expected_fw_size) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_INCOMPLETE_DATA;
        }
        
        ota_node.expected_state = OTA_NODE_WAIT_SIGNAL_UPDATE;
        return LORA_OTA_STATUS_PROCESS_END_SUCCESS;

    case OTA_TYPE_SIGNAL_UPDATE:
        if (ota_node.expected_state != OTA_NODE_WAIT_SIGNAL_UPDATE) {
            return LORA_OTA_STATUS_INVALID_CHECK_STATE;
        }
        if (!ota_node.expected_fw_buffer || ota_node.received_bytes == 0) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_CHECK_CRC_FIRMWARE_ERROR;
        }
        // Tính CRC với bounds check
        crc_t crc_check = crc_slow(ota_node.expected_fw_buffer, ota_node.received_bytes);

        if (crc_check != ota_node.expected_crc) {
            ota_node.expected_state = OTA_NODE_ERROR;
            return LORA_OTA_STATUS_CHECK_CRC_FIRMWARE_ERROR;
        }
        
        ota_node.expected_state = OTA_NODE_DONE;
        return LORA_OTA_STATUS_PROCESS_SIGNAL_UPDATE_SUCCESS;

    default:
        return LORA_OTA_STATUS_INVALID_SESSION;
    }
}

LORA_OtaStatus_t loRaOtaCreateStart(uint8_t *frame_buffer, uint16_t *frame_length)
{
    // Tạo frame structure
    LORA_frame_t lora_frame = {
        .packet_type = LORA_PACKET_TYPE_OTA,
        .message_type = OTA_TYPE_START,
        .data_len = OTA_START_LENGTH,
        .data_payload = 0x00
    };

    LORA_Frame_Status_t result = loRaEncryptedFrame(&lora_frame, frame_buffer);
    
    if (result != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
        return LORA_OTA_STATUS_ENCODE_FAILED;
    }
    
    *frame_length = LORA_FRAME_HEADER_SIZE + LORA_JOIN_REQUEST_LENGTH + sizeof(crc_t) + 1;
    
    return LORA_OTA_STATUS_CREATE_START_SUCCESS;
}

LORA_OtaStatus_t loRaOtaCreateResponse(uint8_t ack_nack,
                                       uint8_t *frame_buffer,
                                       uint16_t *frame_length)
{
    if (!frame_buffer || !frame_length) {
        return LORA_OTA_STATUS_NULL_POINTER;
    }

    uint8_t payload = (ack_nack == OTA_RESP_ACK) ? OTA_RESP_ACK : OTA_RESP_NACK;

    LORA_frame_t lora_frame = {
        .packet_type  = LORA_PACKET_TYPE_OTA,
        .message_type = OTA_TYPE_RESPONSE,
        .data_len     = 1,
        .data_payload = &payload
    };

    LORA_Frame_Status_t result = loRaEncryptedFrame(&lora_frame, frame_buffer);
    if (result != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
        return LORA_OTA_STATUS_ENCODE_FAILED;
    }

    *frame_length = LORA_FRAME_HEADER_SIZE + 1 + sizeof(crc_t) + 1;

    return LORA_OTA_STATUS_CREATE_RESPONSE_SUCCESS;
}

LORA_OtaStatus_t loRaOtaCreateHeader(const ota_header_t *header,
                                     uint8_t *frame_buffer,
                                     uint16_t *frame_length)
{
    if (!header || !frame_buffer || !frame_length) {
        return LORA_OTA_STATUS_NULL_POINTER;
    }

    // Tạo frame OTA_HEADER
    LORA_frame_t lora_frame = {
        .packet_type  = LORA_PACKET_TYPE_OTA,
        .message_type = OTA_TYPE_HEADER,
        .data_len     = sizeof(ota_header_t),
        .data_payload = (uint8_t *)header
    };

    LORA_Frame_Status_t result = loRaEncryptedFrame(&lora_frame, frame_buffer);
    if (result != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
        return LORA_OTA_STATUS_ENCODE_FAILED;
    }

    *frame_length = LORA_FRAME_HEADER_SIZE + sizeof(ota_header_t) + sizeof(crc_t) + 1;

    return LORA_OTA_STATUS_CREATE_HEADER_SUCCESS;
}

// LORA_OtaStatus_t loRaOtaCreateData(uint16_t seq,
//                                    const uint8_t *chunk,
//                                    uint16_t chunk_len,
//                                    uint8_t *frame_buffer,
//                                    uint16_t *frame_length)
// {
//     if (!chunk || !frame_buffer || !frame_length) {
//         return LORA_OTA_STATUS_NULL_POINTER;
//     }

//     // Chuẩn bị payload: seq (2B) + data
//     static uint8_t temp_payload[OTA_MAX_CHUNK_SIZE + 2];
//     memcpy(temp_payload, &seq, sizeof(uint16_t));
//     memcpy(temp_payload + 2, chunk, chunk_len);

//     uint16_t total_len = sizeof(uint16_t) + chunk_len;

//     LORA_frame_t lora_frame = {
//         .packet_type  = LORA_PACKET_TYPE_OTA,
//         .message_type = OTA_TYPE_DATA,
//         .data_len     = total_len,
//         .data_payload = temp_payload
//     };

//     LORA_Frame_Status_t result = loRaEncryptedFrame(&lora_frame, frame_buffer);
//     if (result != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
//         return LORA_OTA_STATUS_ENCODE_FAILED;
//     }

//     *frame_length = LORA_FRAME_HEADER_SIZE + total_len + sizeof(crc_t) + 1;

//     return LORA_OTA_STATUS_CREATE_DATA_SUCCESS;
// }

LORA_OtaStatus_t loRaOtaCreateEnd(uint8_t *frame_buffer, uint16_t *frame_length)
{
    if (!frame_buffer || !frame_length) {
        return LORA_OTA_STATUS_NULL_POINTER;
    }

    LORA_frame_t lora_frame = {
        .packet_type  = LORA_PACKET_TYPE_OTA,
        .message_type = OTA_TYPE_END,
        .data_len     = 1,
        .data_payload = (uint8_t *)"\x01"   // payload đơn giản = 0x01
    };

    LORA_Frame_Status_t result = loRaEncryptedFrame(&lora_frame, frame_buffer);
    if (result != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
        return LORA_OTA_STATUS_ENCODE_FAILED;
    }

    *frame_length = LORA_FRAME_HEADER_SIZE + 1 + sizeof(crc_t) + 1;

    return LORA_OTA_STATUS_CREATE_END_SUCCESS;
}

LORA_OtaStatus_t loRaOtaCreateSignalUpdate(uint8_t *frame_buffer, uint16_t *frame_length)
{
    if (!frame_buffer || !frame_length) {
        return LORA_OTA_STATUS_NULL_POINTER;
    }

    LORA_frame_t lora_frame = {
        .packet_type  = LORA_PACKET_TYPE_OTA,
        .message_type = OTA_TYPE_SIGNAL_UPDATE,
        .data_len     = 1,
        .data_payload = (uint8_t *)"\x02"
    };

    LORA_Frame_Status_t result = loRaEncryptedFrame(&lora_frame, frame_buffer);
    if (result != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
        return LORA_OTA_STATUS_ENCODE_FAILED;
    }

    *frame_length = LORA_FRAME_HEADER_SIZE + 1 + sizeof(crc_t) + 1;

    return LORA_OTA_STATUS_CREATE_SIGNAL_UPDATE_SUCCESS;
}

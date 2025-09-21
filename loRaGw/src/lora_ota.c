/* lora_ota.h
 * OTA-over-LoRa protocol 
 * Author: TrungNM
 * Created: *********
*/

#include "lora_ota.h"

ota_server_ctx_t ota_srv;

LORA_OtaStatus_t loRaOtaCreateStart(uint8_t *frame_buffer, uint16_t *frame_length)
{
    uint8_t payload_create = 0x00;
    // Tạo frame structure
    LORA_frame_t lora_frame = {
        .packet_type = LORA_PACKET_TYPE_OTA,
        .message_type = OTA_TYPE_START,
        .data_len = OTA_START_LENGTH,
        .data_payload = &payload_create
    };

    LORA_Frame_Status_t result = loRaEncryptedFrame(&lora_frame, frame_buffer);
    
    if (result != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
        return LORA_JOIN_STATUS_ENCODE_FAILED;
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

    uint8_t payload = (ack_nack == OTA_ACK) ? OTA_ACK : OTA_NACK;

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


LORA_OtaStatus_t loRaOtaCreateData(uint16_t seq,
                                   const uint8_t *chunk,
                                   uint16_t chunk_len,
                                   uint8_t *frame_buffer,
                                   uint16_t *frame_length)
{
    if (!chunk || !frame_buffer || !frame_length) {
        return LORA_OTA_STATUS_NULL_POINTER;
    }

    // Chuẩn bị payload: seq (2B) + data
    static uint8_t temp_payload[2000 + 2];
    memcpy(temp_payload, &seq, sizeof(uint16_t));
    memcpy(temp_payload + 2, chunk, chunk_len);

    uint16_t total_len = sizeof(uint16_t) + chunk_len;

    LORA_frame_t lora_frame = {
        .packet_type  = LORA_PACKET_TYPE_OTA,
        .message_type = OTA_TYPE_DATA,
        .data_len     = total_len,
        .data_payload = temp_payload
    };

    LORA_Frame_Status_t result = loRaEncryptedFrame(&lora_frame, frame_buffer);
    if (result != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
        return LORA_OTA_STATUS_ENCODE_FAILED;
    }

    *frame_length = LORA_FRAME_HEADER_SIZE + total_len + sizeof(crc_t) + 1;

    return LORA_OTA_STATUS_CREATE_DATA_SUCCESS;
}

LORA_OtaStatus_t loRaOtaCreateEnd(uint8_t *frame_buffer, uint16_t *frame_length)
{
    if (!frame_buffer || !frame_length) {
        return LORA_OTA_STATUS_NULL_POINTER;
    }
    uint8_t payload_end = 0x01;
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

void otaServerPrepare(const uint8_t *fw_image,
                      uint32_t fw_size,
                      uint16_t chunk_size,
                      uint32_t image_crc32,
                      uint16_t session_id)
{
    ota_srv.fw_image   = fw_image;
    ota_srv.fw_size    = fw_size;
    ota_srv.chunk_size = chunk_size;
    ota_srv.total_chunks = (fw_size + chunk_size - 1) / chunk_size;
    ota_srv.current_seq = 0;

    ota_srv.header.session_id   = session_id;
    ota_srv.header.chunk_size   = chunk_size;
    ota_srv.header.total_chunks = ota_srv.total_chunks;
    ota_srv.header.reserved0    = 0;
    ota_srv.header.total_size   = fw_size;
    ota_srv.header.image_crc32  = image_crc32;

    ota_srv.state = OTA_SERVER_IDLE;
}

LORA_OtaStatus_t otaServerNextFrame(uint8_t *frame_buf, uint16_t *frame_len)
{
    switch (ota_srv.state) {
    case OTA_SERVER_IDLE:
        ota_srv.state = OTA_SERVER_START_SENT;
        return loRaOtaCreateStart(frame_buf, frame_len);

    case OTA_SERVER_START_SENT:
        ota_srv.state = OTA_SERVER_HEADER_SENT;
        return loRaOtaCreateHeader(&ota_srv.header, frame_buf, frame_len);

    case OTA_SERVER_HEADER_SENT:
    case OTA_SERVER_DATA_SENDING:
        if (ota_srv.current_seq < ota_srv.total_chunks) {
            ota_srv.state = OTA_SERVER_DATA_SENDING;
            return loRaOtaCreateData(ota_srv.current_seq++,
                                     ota_srv.fw_image + ota_srv.current_seq * ota_srv.chunk_size,
                                     (ota_srv.current_seq == ota_srv.total_chunks - 1) ?
                                         (ota_srv.fw_size - ota_srv.current_seq * ota_srv.chunk_size) :
                                         ota_srv.chunk_size,
                                     frame_buf,
                                     frame_len);
        } else {
            ota_srv.state = OTA_SERVER_END_SENT;
            return loRaOtaCreateEnd(frame_buf, frame_len);
        }

    case OTA_SERVER_END_SENT:
        ota_srv.state = OTA_SERVER_SIGNAL_UPDATE_SENT;
        return loRaOtaCreateSignalUpdate(frame_buf, frame_len);

    case OTA_SERVER_SIGNAL_UPDATE_SENT:
        ota_srv.state = OTA_SERVER_DONE;
        return LORA_OTA_STATUS_CREATE_SIGNAL_UPDATE_SUCCESS;

    case OTA_SERVER_DONE:
        ota_srv.state = OTA_SERVER_IDLE;
        return LORA_OTA_STATUS_INVALID_SESSION; 
        
    // default:
    //     return LORA_OTA_STATUS_INVALID_SESSION;
    }
}

#include "lora_join_network.h"
#include "lora.h"
#include "string.h"


LORA_Join_Network_Status_t loRaJoinMessageHandler(uint8_t message_type, uint8_t *payload, uint16_t payload_len)
{
    if (message_type == LORA_JOIN_TYPE_REQUEST)
        return loRaJoinNetworkGetRequest(payload, payload_len);

    return LORA_JOIN_STATUS_DECODE_FAILED; // Hoặc một enum khác nếu có loại message không hỗ trợ
}


// === JOIN REQUEST IMPLEMENTATION ===

// Node side: Create join request frame
LORA_Join_Network_Status_t loRaCreateJoinRequest(const uint8_t *mac_address, 
                                                uint8_t *frame_buffer, 
                                                uint16_t *frame_length)
{
    // Validation
    if (!mac_address || !frame_buffer || !frame_length) {
        return LORA_JOIN_STATUS_NULL_POINTER;
    }
    
    // Validate MAC address không phải all-zero
    uint8_t zero_mac[6] = {0};
    if (memcmp(mac_address, zero_mac, 6) == 0) {
        return LORA_JOIN_STATUS_INVALID_MAC;
    }
    
    // Tạo payload
    LORA_join_request_t join_request;
    memcpy(join_request.mac_address, mac_address, 6);
    
    // Tạo frame structure
    LORA_frame_t lora_frame = {
        .packet_type = LORA_PACKET_TYPE_JOIN,
        .message_type = LORA_JOIN_TYPE_REQUEST,
        .data_len = sizeof(LORA_join_request_t),
        .data_payload = (uint8_t*)&join_request
    };
    
    LORA_Frame_Status_t result = loRaEncryptedFrame(&lora_frame, frame_buffer);
    
    if (result != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
        return LORA_JOIN_STATUS_ENCODE_FAILED;
    }
    
    *frame_length = LORA_FRAME_HEADER_SIZE + sizeof(LORA_join_request_t) + sizeof(crc_t) + 1;
    
    return LORA_JOIN_STATUS_CREATE_REQUEST_SUCCESS;
}

LORA_Join_Network_Status_t loRaJoinNetworkGetRequest(uint8_t *payload, uint16_t payload_len)
{
    // Kiểm tra tham số đầu vào
    if (!payload) {
        return LORA_JOIN_STATUS_NULL_POINTER;
    }
    
    if (payload_len != sizeof(LORA_join_request_t)) {
        return LORA_JOIN_STATUS_INVALID_PAYLOAD_LENGTH;
    }
    
    // Lấy MAC address từ payload
    const uint8_t *mac_address = payload;
    uint8_t zero_mac[6] = {0};
    
    // Kiểm tra MAC address có hợp lệ không (không được là tất cả số 0)
    if (memcmp(mac_address, zero_mac, 6) == 0) {
        return LORA_JOIN_STATUS_INVALID_MAC;
    }
    
    // Nếu cần lưu MAC address để phản hồi, bạn có thể:
    // 1. Sử dụng biến global
    // 2. Hoặc truyền thêm tham số output
    // 3. Hoặc lưu vào một structure quản lý state
    
    // TODO: Store node info, assign network parameters, etc.
    // TODO: Prepare join response with network session keys
    
    return LORA_JOIN_STATUS_DECODE_REQUEST_SUCCESS;
}

/* ////////////////////////////////////////// JOIN ACCEPT /////////////////////////// */
/* Create Join Accept */
LORA_Join_Network_Status_t loRaJoinNetworkGetAccept(uint8_t *payload_data, uint16_t payload_len)
{
    if (!payload_data) {
        return LORA_JOIN_STATUS_NULL_POINTER;
    }

    if (payload_len != sizeof(LORA_join_accept_t)) {
        return LORA_JOIN_STATUS_INVALID_PAYLOAD_LENGTH;
    }

    const LORA_join_accept_t *join_accept = (const LORA_join_accept_t *)payload_data;

    static const uint8_t zero_mac[6] = {0};
    static const uint8_t zero_key[16] = {0};

    if (memcmp(join_accept->mac_address, zero_mac, 6) == 0) {
        return LORA_JOIN_STATUS_INVALID_MAC;
    }

    if (memcmp(join_accept->net_key, zero_key, 16) == 0) {
        return LORA_JOIN_STATUS_INVALID_KEY;
    }

    LORA_Join_Network_Status_t store_result = loRaStoreJoinAcceptData(
        join_accept->mac_address, join_accept->net_key
    );

    if (store_result != LORA_JOIN_STATUS_STORE_SUCCESS) {
        return store_result;
    }

    return LORA_JOIN_STATUS_PROCESS_SUCCESS;
}

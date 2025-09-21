#include "lora_join_network.h"
#include "lora.h"
#include "string.h"

/* ========================================= FUNCTION FOR NODE ============================================= */


LORA_Join_Network_Status_t loRaJoinPacketHandler(uint8_t message_type, uint8_t *payload, uint16_t payload_len)
{
    LORA_join_info_t join_info;
    // if (message_type == LORA_JOIN_TYPE_REQUEST)
    //     return loRaJoinNetworkGetRequest(payload, payload_len, &join_info);
    if (message_type == LORA_JOIN_TYPE_ACCEPT){
        return loRaJoinNetworkGetAccept(payload, payload_len, &join_info);
    }

    if (message_type == LORA_JOIN_TYPE_COMPLETED){
        return loRaJoinNetworkGetComplete(payload, payload_len, &join_info);
    }

    return LORA_JOIN_STATUS_DECODE_FAILED; // Hoặc một enum khác nếu có loại message không hỗ trợ
}


// === JOIN REQUEST IMPLEMENTATION ===

// Node side: Create join request frame
LORA_Join_Network_Status_t loRaCreateJoinRequest(uint8_t *mac_address, 
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
    
    // Tạo frame structure
    LORA_frame_t lora_frame = {
        .packet_type = LORA_PACKET_TYPE_JOIN,
        .message_type = LORA_JOIN_TYPE_REQUEST,
        .data_len = LORA_JOIN_REQUEST_LENGTH,
        .data_payload = mac_address
    };
    
    LORA_Frame_Status_t result = loRaEncryptedFrame(&lora_frame, frame_buffer);
    
    if (result != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
        return LORA_JOIN_STATUS_ENCODE_FAILED;
    }
    
    *frame_length = LORA_FRAME_HEADER_SIZE + LORA_JOIN_REQUEST_LENGTH + sizeof(crc_t) + 1;
    
    return LORA_JOIN_STATUS_CREATE_REQUEST_SUCCESS;
}



/* ===  Get Join Accept == */
LORA_Join_Network_Status_t loRaJoinNetworkGetAccept(uint8_t *payload_data, uint16_t payload_len, LORA_join_info_t *join_info )
{
    if (!payload_data || !join_info) {
        return LORA_JOIN_STATUS_NULL_POINTER;
    }

    if (payload_len != 6+16) {
        return LORA_JOIN_STATUS_INVALID_PAYLOAD_LENGTH;
    }

    const uint8_t zero_mac[6] = {0};
    const uint8_t zero_key[16] = {0};
    
    if (memcmp(payload_data, zero_mac, 6) == 0) {
        return LORA_JOIN_STATUS_INVALID_MAC;
    }

    if (memcmp(payload_data + 6, zero_key, 16) == 0) {
        return LORA_JOIN_STATUS_INVALID_KEY;
    }
    // Copy 6 bytes đầu vào mac_address
    memcpy(join_info->mac_address, payload_data, 6);
    
    // Copy 16 bytes tiếp theo vào net_key
    memcpy(join_info->net_key, payload_data + 6, 16);
 
    return LORA_JOIN_STATUS_GET_ACCEPT_SUCCESS;
}


/**
 * @brief Create Join Confirm
 */
LORA_Join_Network_Status_t loRaCreateJoinConfirm(const uint8_t *mac_address, const uint8_t *dev_key,
                                                 uint8_t *frame_buffer, uint16_t *frame_length)
{
    if (!mac_address || !dev_key || !frame_buffer || !frame_length) {
        return LORA_JOIN_STATUS_NULL_POINTER;
    }
   
    // Validate MAC
    static const uint8_t zero_mac[6] = {0};
    if (memcmp(mac_address, zero_mac, 6) == 0 ) {
        return LORA_JOIN_STATUS_INVALID_MAC;
    }

    // Validate dev_key
    static const uint8_t zero_dev_key[16] = {0};
    if (memcmp(dev_key, zero_dev_key, 16) == 0 ) {
        return LORA_JOIN_STATUS_INVALID_KEY;
    }

    uint8_t payload[6 + 16];
    memcpy(payload, mac_address, 6);
    memcpy(payload + 6, dev_key, 16);

    // Đóng gói frame
    LORA_frame_t lora_frame = {
        .packet_type   = LORA_PACKET_TYPE_JOIN,
        .message_type  = LORA_JOIN_TYPE_ACCEPT,
        .data_len      = 6 + 16,
        .data_payload  = payload
    };

    LORA_Frame_Status_t status = loRaEncryptedFrame(&lora_frame, frame_buffer);
    
    if (status != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
        return LORA_JOIN_STATUS_ENCODE_FAILED;
    }

    *frame_length = LORA_FRAME_HEADER_SIZE + lora_frame.data_len + sizeof(crc_t) + 1; // 1 = encrypted flag byte
    return LORA_JOIN_STATUS_CREATE_CONFIRM_SUCCESS;
}


/* Get Join Complete */
LORA_Join_Network_Status_t loRaJoinNetworkGetComplete(uint8_t *payload_data, uint16_t payload_len, LORA_join_info_t *join_info )
{
    if (!payload_data || !join_info) {
        return LORA_JOIN_STATUS_NULL_POINTER;
    }

    if (payload_len != 6+2) {
        return LORA_JOIN_STATUS_INVALID_PAYLOAD_LENGTH;
    }

    const uint8_t zero_mac[6] = {0};
    const uint8_t zero_unicast[2] = {0};
    
    if (memcmp(payload_data, zero_mac, 6) == 0) {
        return LORA_JOIN_STATUS_INVALID_MAC;
    }

    if (memcmp(payload_data + 6, zero_unicast, 2) == 0) {
        return LORA_JOIN_STATUS_INVALID_KEY;
    }
    // Copy 6 bytes đầu vào mac_address
    memcpy(join_info->mac_address, payload_data, 6);
    
    // Copy 2 bytes tiếp theo vào unicast
    memcpy(join_info->unicast, payload_data + 6, 2);
    return LORA_JOIN_STATUS_GET_COMPLETE_SUCCESS;
}
 
/* ========================================= FUNCTION FOR GATEWAY ============================================= */


// LORA_Join_Network_Status_t loRaJoinNetworkGetRequest(uint8_t *payload, uint16_t payload_len, LORA_join_info_t *join_info)
// {
//     // Kiểm tra tham số đầu vào
//     if (!payload || !join_info) {
//         return LORA_JOIN_STATUS_NULL_POINTER;
//     }
    
//     if (payload_len != LORA_JOIN_REQUEST_LENGTH) {
//         return LORA_JOIN_STATUS_INVALID_PAYLOAD_LENGTH;
//     }
    
//     // Lấy MAC address từ payload
//     const uint8_t *payload_mac  = payload;
//     uint8_t zero_mac[6] = {0};
    
//     // Check not zero_mac
//     if (memcmp(zero_mac, payload_mac , 6) == 0) {
//         return LORA_JOIN_STATUS_INVALID_MAC;
//     }
    
//     memcpy(join_info->mac_address, payload_mac, sizeof(join_info->mac_address));    

//     return LORA_JOIN_STATUS_GET_REQUEST_SUCCESS;
// }


// /* Create Join Accept */
// LORA_Join_Network_Status_t loRaCreateJoinAccept(uint8_t *mac_address,  uint8_t *net_key,
//                                                  uint8_t *frame_buffer, uint16_t *frame_length)
// {
//     if (!mac_address || !net_key || !frame_buffer || !frame_length) {
//         return LORA_JOIN_STATUS_NULL_POINTER;
//     }

//     // Validate MAC
//     static const uint8_t zero_mac[6] = {0};
//     if (memcmp(mac_address, zero_mac, 6) == 0 ) {
//         return LORA_JOIN_STATUS_INVALID_MAC;
//     }

//     // Validate net_key
//     static const uint8_t zero_key[16] = {0};
//     if (memcmp(net_key, zero_key, 16) == 0 ) {
//         return LORA_JOIN_STATUS_INVALID_KEY;
//     }

//     uint8_t payload[6 + 16];
//     memcpy(payload, mac_address, 6);
//     memcpy(payload + 6, net_key, 16);

//     // Đóng gói frame
//     LORA_frame_t lora_frame = {
//         .packet_type   = LORA_PACKET_TYPE_JOIN,
//         .message_type  = LORA_JOIN_TYPE_ACCEPT,
//         .data_len      = 6 + 16,
//         .data_payload  = payload
//     };

//     LORA_Frame_Status_t status = loRaEncryptedFrame(&lora_frame, frame_buffer);
    
//     if (status != LORA_STATUS_ENCODE_FRAME_SUCCESS) {
//         return LORA_JOIN_STATUS_ENCODE_FAILED;
//     }

//     *frame_length = LORA_FRAME_HEADER_SIZE + lora_frame.data_len + sizeof(crc_t) + 1; // 1 = encrypted flag byte
//     return LORA_JOIN_STATUS_CREATE_ACCEPT_SUCCESS;
// }


/* Get Join Confirm */
LORA_Join_Network_Status_t loRaJoinNetworkGetConfirm(uint8_t *payload_data, uint16_t payload_len, LORA_join_info_t *join_info )
{
    if (!payload_data || !join_info) {
        return LORA_JOIN_STATUS_NULL_POINTER;
    }

    if (payload_len != 6+16) {
        return LORA_JOIN_STATUS_INVALID_PAYLOAD_LENGTH;
    }

    const uint8_t zero_mac[6] = {0};
    const uint8_t zero_dev_key[16] = {0};
    
    if (memcmp(payload_data, zero_mac, 6) == 0) {
        return LORA_JOIN_STATUS_INVALID_MAC;
    }

    if (memcmp(payload_data + 6, zero_dev_key, 16) == 0) {
        return LORA_JOIN_STATUS_INVALID_KEY;
    }
    // Copy 6 bytes đầu vào mac_address
    memcpy(join_info->mac_address, payload_data, 6);
    
    // Copy 16 bytes tiếp theo vào net_key
    memcpy(join_info->dev_key, payload_data + 6, 16);
 
    return LORA_JOIN_STATUS_GET_CONFIRM_SUCCESS;
}
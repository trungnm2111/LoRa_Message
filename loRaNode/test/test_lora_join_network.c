#ifdef TEST

#include <string.h>
#include "unity.h"
#include "lora_join_network.h"
#include "lora.h"
#include "crc.h"
#include "lora_node_storage.h"

// Giả lập biến toàn cục gLoraData và gSysParam
// typedef struct {
//     uint8_t Buffer[LORA_PACKET_MAX_SIZE];
//     uint16_t Index;c
//     uint16_t Length;
// } LORA_Data_t;

static void printHexArray(const uint8_t *array, uint16_t length);


void setUp(void) {
      // Khởi tạo trạng thái ban đầu
    crc_init();
    // lora_init_frame();
    // storageClearAll();
    storageInit();
    loRaInitFsmFrame();
}

void tearDown(void) {
    storageClearAll();
    loRaInitFsmFrame();
}



// ====================== Test CREATE_JOIN_REQUEST ======================
void test_loRaCreateJoinRequest(void) {
    printf("== Start Test CreateJoinRequest ==\n");
    uint8_t valid_mac[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
    uint8_t zero_mac[6]  = {0};
    uint8_t frame_buffer[64];
    uint16_t frame_len;
    LORA_Join_Network_Status_t status;

    // Case 1: mac_address NULL
    status = loRaCreateJoinRequest(NULL, frame_buffer, &frame_len);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, status, "Case1: mac_address NULL");

    // Case 2: frame_buffer NULL
    status = loRaCreateJoinRequest(valid_mac, NULL, &frame_len);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, status, "Case2: frame_buffer NULL");

    // Case 3: frame_length NULL
    status = loRaCreateJoinRequest(valid_mac, frame_buffer, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, status, "Case3: frame_length NULL");

    // Case 4: MAC toàn 0
    status = loRaCreateJoinRequest(zero_mac, frame_buffer, &frame_len);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_INVALID_MAC, status, "Case4: MAC all zero");

    // Case 5: encode thất bại (nếu muốn mock)
    status = loRaCreateJoinRequest(valid_mac, frame_buffer, &frame_len);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(LORA_JOIN_STATUS_ENCODE_FAILED, status, "Case5: encode fail");

    // Case 6: encode thành công
    status = loRaCreateJoinRequest(valid_mac, frame_buffer, &frame_len);
    printHexArray(frame_buffer, frame_len);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_CREATE_REQUEST_SUCCESS, status, "Case6: encode success");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(
        (LORA_FRAME_HEADER_SIZE + LORA_JOIN_REQUEST_LENGTH + LORA_FRAME_CRC_SIZE + 1),
        frame_len,
        "Frame length mismatch"
    );
    printf("TEST CREATE JOIN REQUEST SUCCESS ** \n");
}

/* ==== Test Get Accept === */
void test_loRaJoinNetworkGetAccept_valid_payload(void) {
    uint8_t payload[22] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66,   // MAC
        0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08, 0x09, 0x0A    // net_key
    };
    LORA_join_info_t join_info;
    
    LORA_Join_Network_Status_t status = loRaJoinNetworkGetAccept(payload, sizeof(payload), &join_info);

    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_GET_ACCEPT_SUCCESS, status, "Valid payload should return SUCCESS");

    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(payload, join_info.mac_address, 6, "MAC mismatch");
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(payload + 6, join_info.net_key, 16, "Net key mismatch");

    printf("MAC: ");     printHexArray(join_info.mac_address, 6);
    printf("Net Key: "); printHexArray(join_info.net_key, 16);
}


void test_loRaJoinNetworkGetAccept_invalid_mac(void) {
    uint8_t payload[22] = {0}; // All zero
    LORA_join_info_t join_info;

    LORA_Join_Network_Status_t status = loRaJoinNetworkGetAccept(payload, sizeof(payload), &join_info);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_INVALID_MAC, status, "Zero MAC should be invalid");
}

void test_loRaJoinNetworkGetAccept_invalid_key(void) {
    uint8_t payload[22] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66
    }; // Rest key = 0
    LORA_join_info_t join_info;

    LORA_Join_Network_Status_t status = loRaJoinNetworkGetAccept(payload, sizeof(payload), &join_info);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_INVALID_KEY, status, "Zero key should be invalid");
}

void test_loRaJoinNetworkGetAccept_null_pointer(void) {
    uint8_t payload[22] = {1};
    LORA_Join_Network_Status_t status;

    status = loRaJoinNetworkGetAccept(NULL, sizeof(payload), NULL);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, status, "NULL args should return NULL_POINTER");
}


void test_end_to_end_create_and_get_accept(void)
{
    printf("\n \n Start test End to End create and Get Join Accept \n \n ");
    uint8_t mac_address[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    uint8_t net_key[16]    = {
        0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
        0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99
    };
    uint8_t frame_buffer[64];
    uint16_t frame_len = 0;
    LORA_join_info_t decoded_info;

    // 1. Tạo Join Accept frame
    LORA_Join_Network_Status_t status_create = loRaCreateJoinAccept(
        mac_address,
        net_key,
        frame_buffer,
        &frame_len
    );

    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_CREATE_ACCEPT_SUCCESS, status_create,
                              "Create Join Accept failed!");

    printf("Generated Join Accept frame: ");
    printHexArray(frame_buffer, frame_len);

    LORA_frame_t OTA_FSM_Data; // Đã khai báo trong module chính
    memset(&OTA_FSM_Data, 0, sizeof(OTA_FSM_Data));

    // === Giả lập giải mã (bạn cần implement hoặc đã có hàm này)
    LORA_Frame_Status_t decode_status = loRaDecodeFrame(frame_buffer, frame_len, &OTA_FSM_Data);
    TEST_ASSERT_EQUAL(LORA_STATUS_DECODE_FRAME_SUCCESS, decode_status);

    // 2. Giải mã lại bằng Get Accept
    LORA_Join_Network_Status_t status_get = loRaJoinNetworkGetAccept(
        OTA_FSM_Data.data_payload,   // payload_data
        OTA_FSM_Data.data_len,      // payload_len
        &decoded_info
    );

    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_GET_ACCEPT_SUCCESS, status_get,
                              "Get Join Accept failed!");

    // 3. So sánh MAC
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(mac_address, decoded_info.mac_address, 6,
                                          "Decoded MAC address does not match!");

    // 4. So sánh NetKey
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(net_key, decoded_info.net_key, 16,
                                          "Decoded NetKey does not match!");
}

/* ======================================== Join Confirm ============================================================*/

void test_loRaCreateJoinConfirm_valid_data(void) {
    uint8_t mac[6] = {0x01,0x02,0x03,0x04,0x05,0x06};
    uint8_t dev_key[16] = {
        0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,
        0x90,0xA0,0xB0,0xC0,0xD0,0xE0,0xF0,0xFF
    };
    uint8_t frame_buffer[128] = {0};
    uint16_t frame_length = 0;

    LORA_Join_Network_Status_t status = loRaCreateJoinConfirm(
        mac, dev_key, frame_buffer, &frame_length
    );

    TEST_ASSERT_EQUAL_MESSAGE(
        LORA_JOIN_STATUS_CREATE_CONFIRM_SUCCESS,
        status,
        "loRaCreateJoinConfirm should succeed with valid inputs"
    );

    TEST_ASSERT_TRUE_MESSAGE(frame_length > 0, "Frame length should be > 0");

    printf("Join Confirm Frame:\n");
    printHexArray(frame_buffer, frame_length);
}

void test_loRaCreateJoinConfirm_null_pointer(void) {
    uint8_t mac[6] = {1};
    uint8_t dev_key[16] = {1};
    uint8_t buffer[64];
    uint16_t len;

    TEST_ASSERT_EQUAL_MESSAGE(
        LORA_JOIN_STATUS_NULL_POINTER,
        loRaCreateJoinConfirm(NULL, dev_key, buffer, &len),
        "Should return NULL_POINTER if mac is NULL"
    );
    TEST_ASSERT_EQUAL_MESSAGE(
        LORA_JOIN_STATUS_NULL_POINTER,
        loRaCreateJoinConfirm(mac, NULL, buffer, &len),
        "Should return NULL_POINTER if dev_key is NULL"
    );
    TEST_ASSERT_EQUAL_MESSAGE(
        LORA_JOIN_STATUS_NULL_POINTER,
        loRaCreateJoinConfirm(mac, dev_key, NULL, &len),
        "Should return NULL_POINTER if buffer is NULL"
    );
    TEST_ASSERT_EQUAL_MESSAGE(
        LORA_JOIN_STATUS_NULL_POINTER,
        loRaCreateJoinConfirm(mac, dev_key, buffer, NULL),
        "Should return NULL_POINTER if length pointer is NULL"
    );
}

void test_loRaCreateJoinConfirm_invalid_mac(void) {
    uint8_t zero_mac[6] = {0};
    uint8_t dev_key[16] = {1};
    uint8_t buffer[64];
    uint16_t len;

    TEST_ASSERT_EQUAL_MESSAGE(
        LORA_JOIN_STATUS_INVALID_MAC,
        loRaCreateJoinConfirm(zero_mac, dev_key, buffer, &len),
        "Should return INVALID_MAC if mac is all zeros"
    );
}

void test_loRaCreateJoinConfirm_invalid_key(void) {
    uint8_t mac[6] = {1};
    uint8_t zero_key[16] = {0};
    uint8_t buffer[64];
    uint16_t len;

    TEST_ASSERT_EQUAL_MESSAGE(
        LORA_JOIN_STATUS_INVALID_KEY,
        loRaCreateJoinConfirm(mac, zero_key, buffer, &len),
        "Should return INVALID_KEY if dev_key is all zeros"
    );
}

/* Test create join confirm và get join confirm */

void test_loRaCreateJoinConfirm_and_GetConfirm_success(void) {
    uint8_t mac[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    uint8_t dev_key[16] = {
        0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x01, 0x02,
        0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A
    };

    uint8_t frame_buffer[LORA_PACKET_MAX_SIZE];
    uint16_t frame_len = 0;

    /* B1: Tạo Join Confirm */
    LORA_Join_Network_Status_t status = loRaCreateJoinConfirm(
        mac, dev_key, frame_buffer, &frame_len
    );

    TEST_ASSERT_EQUAL_MESSAGE(
        LORA_JOIN_STATUS_CREATE_CONFIRM_SUCCESS,
        status,
        "Create Join Confirm failed!"
    );

    printf("Join Confirm Frame: ");
    printHexArray(frame_buffer, frame_len);

    /* B2: Giải mã frame để lấy payload thô */
    LORA_frame_t OTA_FSM_Data; // Đã khai báo trong module chính
    memset(&OTA_FSM_Data, 0, sizeof(OTA_FSM_Data));

    LORA_Frame_Status_t decode_status = loRaDecodeFrame(frame_buffer, frame_len, &OTA_FSM_Data);
    TEST_ASSERT_EQUAL(LORA_STATUS_DECODE_FRAME_SUCCESS, decode_status);

    /* B3: Gọi hàm GetConfirm */
    LORA_join_info_t join_info;
    memset(&join_info, 0, sizeof(join_info));

    LORA_Join_Network_Status_t get_status = loRaJoinNetworkGetConfirm(
        OTA_FSM_Data.data_payload,   // payload_data
        OTA_FSM_Data.data_len,
        &join_info
    );

    TEST_ASSERT_EQUAL_MESSAGE(
        LORA_JOIN_STATUS_GET_CONFIRM_SUCCESS,
        get_status,
        "Get Confirm failed!"
    );

    /* B4: Kiểm tra giá trị */
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
        mac,
        join_info.mac_address,
        6,
        "MAC address mismatch!"
    );

    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
        dev_key,
        join_info.dev_key,
        16,
        "Dev key mismatch!"
    );
}


//Test Join complete

void test_loRaJoinNetworkGetComplete_full(void) {
    printf("\n==== TEST loRaJoinNetworkGetComplete ====\n");

    uint8_t payload[8] = {0x11,0x22,0x33,0x44,0x55,0x66, 0xAB,0xCD};
    LORA_join_info_t join_info;
    memset(&join_info, 0, sizeof(join_info));

    /* Case 1: Thành công */
    LORA_Join_Network_Status_t status = loRaJoinNetworkGetComplete(payload, sizeof(payload), &join_info);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_GET_COMPLETE_SUCCESS, status,
                              "Expected SUCCESS for valid Join Complete");
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, join_info.mac_address, 6);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload+6, join_info.dev_key, 2);  // hiện tại copy vào dev_key

    printf("Parsed MAC: ");
    printHexArray(join_info.mac_address, 6);
    printf("Parsed Unicast: ");
    printHexArray(join_info.dev_key, 2);

    /* Case 2: Null pointer */
    status = loRaJoinNetworkGetComplete(NULL, sizeof(payload), &join_info);
    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_NULL_POINTER, status);

    status = loRaJoinNetworkGetComplete(payload, sizeof(payload), NULL);
    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_NULL_POINTER, status);

    /* Case 3: Sai payload length */
    status = loRaJoinNetworkGetComplete(payload, 5, &join_info);
    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_INVALID_PAYLOAD_LENGTH, status);

    /* Case 4: MAC toàn zero */
    uint8_t zero_mac_payload[8] = {0};
    status = loRaJoinNetworkGetComplete(zero_mac_payload, 8, &join_info);
    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_INVALID_MAC, status);

    /* Case 5: Unicast toàn zero */
    uint8_t zero_unicast_payload[8] = {0x11,0x22,0x33,0x44,0x55,0x66, 0x00,0x00};
    status = loRaJoinNetworkGetComplete(zero_unicast_payload, 8, &join_info);
    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_INVALID_KEY, status);

    printf("==== END TEST loRaJoinNetworkGetComplete ====\n");
}


static void printHexArray(const uint8_t *array, uint16_t length) 
{
    printf("Array Output (%d bytes):\n", length);
    for (uint16_t i = 0; i < length; i++) {
        printf("%02X ", array[i]);
    }
    printf("\n");
}


#endif // TEST

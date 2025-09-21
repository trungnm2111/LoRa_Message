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



// ====================== Test GET_REQUEST ======================
void test_loRaJoinNetworkGetRequest(void) {
    uint8_t valid_mac[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
    uint8_t zero_mac[6]  = {0};
    LORA_join_info_t join_info;
    LORA_Join_Network_Status_t status;

    printf("Bắt đầu tiến hành test cho GET_REQUEST \n");

    // Case 1: payload NULL
    status = loRaJoinNetworkGetRequest(NULL, LORA_JOIN_REQUEST_LENGTH, &join_info);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, status, "Case1: payload NULL");

    // Case 2: join_info NULL
    status = loRaJoinNetworkGetRequest(valid_mac, LORA_JOIN_REQUEST_LENGTH, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, status, "Case2: join_info NULL");

    // Case 3: sai length
    status = loRaJoinNetworkGetRequest(valid_mac, 5, &join_info);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_INVALID_PAYLOAD_LENGTH, status, "Case3: invalid length");

    // Case 4: MAC toàn 0
    status = loRaJoinNetworkGetRequest(zero_mac, LORA_JOIN_REQUEST_LENGTH, &join_info);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_INVALID_MAC, status, "Case4: MAC all zero");

    // Case 5: MAC hợp lệ
    status = loRaJoinNetworkGetRequest(valid_mac, LORA_JOIN_REQUEST_LENGTH, &join_info);
    printHexArray(join_info.mac_address, 6);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_GET_REQUEST_SUCCESS, status, "Case5: valid MAC");
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(valid_mac, join_info.mac_address, 6, "Extracted MAC mismatch");
    printf("TEST GET JOIN REQUEST SUCCESS ** \n");
}

// ====================== Test CREATE_JOIN_ACCEPT ======================

void test_loRaCreateJoinAccept_null_ptr(void) {
    printf("== Start test join accept null ptr \n");
    uint8_t mac[6] = {1,2,3,4,5,6};
    uint8_t key[16]= {1};
    uint8_t buffer[256];
    uint16_t len;

    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, loRaCreateJoinAccept(NULL, key, buffer, &len), "MAC NULL");
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, loRaCreateJoinAccept(mac, NULL, buffer, &len), "Key NULL");
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, loRaCreateJoinAccept(mac, key, NULL, &len), "Buffer NULL");
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, loRaCreateJoinAccept(mac, key, buffer, NULL), "Len NULL");
    printf("TEST CREATE JOIN ACCEPT NULL PTR SUCCESS ** \n");
}

void test_loRaCreateJoinAccept_invalid_mac_key(void) {
    printf("== Start test join accept invalid mac key \n");
    uint8_t zero_mac[6] = {0};
    uint8_t zero_key[16] = {0};
    uint8_t buffer[256];
    uint16_t len;

    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_INVALID_MAC,
        loRaCreateJoinAccept(zero_mac, (uint8_t[16]){1}, buffer, &len),
        "MAC all zero");

    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_INVALID_KEY,
        loRaCreateJoinAccept((uint8_t[6]){1}, zero_key, buffer, &len),
        "Key all zero");
    printf("TEST CREATE JOIN ACCEPT INVALID MAC SUCCESS ** \n");
}

void test_loRaCreateJoinAccept_success(void) {
    printf("== Start Test CreateJoinAccept ==\n");
    uint8_t mac[6]     = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    uint8_t net_key[16]= {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
                          0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0xFF};
    uint8_t frame_buffer[256];
    uint16_t frame_length = 0;

    LORA_Join_Network_Status_t status = loRaCreateJoinAccept(mac, net_key, frame_buffer, &frame_length);

    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_CREATE_ACCEPT_SUCCESS, status, "JoinAccept success");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(
        LORA_FRAME_HEADER_SIZE + 22 + LORA_FRAME_CRC_SIZE + 1,
        frame_length,
        "JoinAccept frame length mismatch"
    );
    printf("TEST CREATE JOIN ACCEPT SUCCESS ** \n");
}

/* --------------------------    TEST GET REQUEST  + CREATE ACCEPT ------------------------------------------------ */
void test_join_request_and_accept(void)
{
    uint8_t join_req_frame[LORA_PACKET_MAX_SIZE];
    uint16_t join_req_len = 0;

    // ===== 1. Giả lập Join Request =====
    uint8_t mac[6]     = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    LORA_Join_Network_Status_t status;

    status = loRaCreateJoinRequest(mac, join_req_frame, &join_req_len);
    printHexArray(join_req_frame, join_req_len);
    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_CREATE_REQUEST_SUCCESS, status);

    // ===== 2. Gọi hàm Get Request =====
    uint8_t parsed_mac[6];
    LORA_join_info_t join_info;
    
    LORA_Join_Network_Status_t get_status = loRaJoinNetworkGetRequest( mac, 6, &join_info);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_GET_REQUEST_SUCCESS, get_status, "get request success");
    printHexArray(join_info.mac_address, 6);
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(mac, join_info.mac_address, 6, "Extracted MAC mismatch");
    memcpy(parsed_mac, join_info.mac_address, 6);

    printf("Join Request parsed OK\n");

    // ===== 3. Tạo Join Accept =====
    uint8_t net_key[16]= {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x01, 0x02,
                        0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    uint8_t join_accept_frame[LORA_PACKET_MAX_SIZE];
    uint16_t join_accept_len = 0; 

    LORA_Join_Network_Status_t acc_status = loRaCreateJoinAccept( parsed_mac, net_key, join_accept_frame, &join_accept_len );

    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_CREATE_ACCEPT_SUCCESS, acc_status, "Failed to create Join Accept");
    printf("Join Accept created OK, length = %u\n", join_accept_len);

    // ===== 4. In ra frame để kiểm tra =====
    printf("Join Accept Frame: ");
    printHexArray(join_accept_frame, join_accept_len);
}

/* ======================================== Join Confirm ============================================================*/


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

/* Test Create Join Complete*/

void test_loRaCreateJoinComplete_full(void) {
    uint8_t mac[6]      = {0x11,0x22,0x33,0x44,0x55,0x66};
    uint8_t unicast[2]  = {0xAB,0xCD};
    uint16_t frame_len  = 0;
    uint8_t frame_buffer[LORA_PACKET_MAX_SIZE];

    printf("\n==== TEST loRaCreateJoinComplete ====\n");

    /* Case 1: Thành công */
    LORA_Join_Network_Status_t status = loRaCreateJoinComplete(mac, unicast, &frame_buffer , &frame_len);
    TEST_ASSERT_EQUAL_MESSAGE(
        LORA_JOIN_STATUS_CREATE_COMPLETE_SUCCESS,
        status,
        "Expected SUCCESS when creating Join Complete"
    );
    TEST_ASSERT_TRUE_MESSAGE(frame_len > LORA_FRAME_HEADER_SIZE, "Frame length should be larger than header size");

    printf("Generated Join Complete frame (len=%d): ", frame_len);
    printHexArray(frame_buffer, frame_len);

    /* Kiểm tra MAC nằm trong payload */
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(mac, frame_buffer + LORA_FRAME_HEADER_SIZE, 6,
                                          "MAC in payload does not match");

    /* Kiểm tra Unicast nằm trong payload */
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(unicast, frame_buffer + LORA_FRAME_HEADER_SIZE + 6, 2,
                                          "Unicast in payload does not match");

    /* Case 2: Null pointer */
    status = loRaCreateJoinComplete(NULL, unicast, &frame_buffer, &frame_len);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, status,
                              "Expected NULL_POINTER when mac=NULL");

    status = loRaCreateJoinComplete(mac, NULL,  &frame_buffer, &frame_len);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_NULL_POINTER, status,
                              "Expected NULL_POINTER when unicast=NULL");

    /* Case 3: Invalid MAC (all zero) */
    uint8_t zero_mac[6] = {0};
    status = loRaCreateJoinComplete(zero_mac, unicast, &frame_buffer, &frame_len);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_INVALID_MAC, status,
                              "Expected INVALID_MAC when mac=all zero");

    /* Case 4: Invalid Unicast (all zero) */
    uint8_t zero_unicast[2] = {0};
    status = loRaCreateJoinComplete(mac, zero_unicast, &frame_buffer, &frame_len);
    TEST_ASSERT_EQUAL_MESSAGE(LORA_JOIN_STATUS_INVALID_KEY, status,
                              "Expected INVALID_KEY when unicast=all zero");

    printf("==== END TEST loRaCreateJoinComplete ====\n");
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

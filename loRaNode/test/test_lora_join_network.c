#ifdef TEST
#include <string.h>
#include "unity.h"
#include "lora_join_network.h"
#include "lora.h"
#include "crc.h"

// Giả lập biến toàn cục gLoraData và gSysParam
// typedef struct {
//     uint8_t Buffer[LORA_PACKET_MAX_SIZE];
//     uint16_t Index;
//     uint16_t Length;
// } LORA_Data_t;

static void printHexArray(const uint8_t *array, uint16_t length);



void setUp(void)
{
    // Khởi tạo trạng thái ban đầu
    crc_init();
    // lora_init_frame();
}

void tearDown(void)
{
    // Không cần dọn dẹp
}


// void test_loraJoinNetworkRequest_shouldReturnEncodedJoinFrame(void)
// {
//     // Tạo địa chỉ MAC giả định (2 x uint32_t = 8 byte)
//     uint32_t test_mac[2] = {0x11223344, 0x55667788};

//     // Buffer để chứa frame đã mã hóa
//     uint8_t encoded_buffer_expected[LORA_PACKET_MAX_SIZE] = {0};
//     uint8_t encoded_buffer_actual[LORA_PACKET_MAX_SIZE] = {0};

//     // Tạo frame thủ công bằng hàm encode gốc
//     LORA_frame_t expected_frame = {
//         .sof = LORA_SOF,
//         .packet_type = LORA_PACKET_TYPE_JOIN,
//         .message_type = LORA_JOIN_TYPE_REQUEST,
//         .data_len = 8,
//         .data_payload = (uint8_t *)test_mac,
//         .eof = LORA_EOF
//     };
//     LORA_Frame_Status_t encode_status = loRaEncodeFrame(&expected_frame, encoded_buffer_expected);
//     TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_SUCCESS, encode_status);

//     // Gọi hàm cần test
//     uint8_t result = loraJoinNetworkRequest(test_mac, encoded_buffer_actual);

    
//     // In kết quả buffer ra màn hình
//     uint16_t encoded_len = LORA_FRAME_HEADER_SIZE + 8 + sizeof(crc_t) + 1;
//     printHexArray(encoded_buffer_actual, encoded_len);

//     // Xác minh trả về thành công
//     TEST_ASSERT_EQUAL_UINT8(1, result);

//     // // Xác minh dữ liệu giống với kết quả mong đợi
//     // uint16_t encoded_len = LORA_FRAME_HEADER_SIZE + 8 + sizeof(crc_t) + 1; // header + MAC + CRC + EOF
//     TEST_ASSERT_EQUAL_UINT8_ARRAY(encoded_buffer_expected, encoded_buffer_actual, encoded_len);
// }

void test_loRaCreateJoinRequest_valid_input_should_succeed(void)
{
    uint8_t mac[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01 };
    uint8_t frame_buf[64] = {0};
    uint16_t frame_len = 0;

    LORA_Join_Network_Status_t status = loRaCreateJoinRequest(mac, frame_buf, &frame_len);

    printHexArray(frame_buf,frame_len );
    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_CREATE_JOIN_REQUEST_SUCCESS, status);
    TEST_ASSERT_GREATER_THAN(0, frame_len); // Frame được tạo
}

void test_loRaCreateJoinRequest_null_params_should_fail(void)
{
    uint8_t frame_buf[64];
    uint16_t frame_len;

    LORA_Join_Network_Status_t status = loRaCreateJoinRequest(NULL, frame_buf, &frame_len);
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_ERROR, status);

    status = loRaCreateJoinRequest((uint8_t*)"123456", NULL, &frame_len);
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_ERROR, status);

    status = loRaCreateJoinRequest((uint8_t*)"123456", frame_buf, NULL);
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_ERROR, status);
}


void test_loRaProcessJoinRequest_valid_mac_should_succeed(void)
{
    uint8_t valid_mac[6] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
    uint8_t response_mac[6] = {0};

    LORA_Join_Network_Status_t status = loRaProcessJoinRequest(valid_mac, 6, response_mac);

    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_DECODE_JOIN_REQUEST_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(valid_mac, response_mac, 6);
}

void test_loRaProcessJoinRequest_invalid_mac_should_fail(void)
{
    uint8_t zero_mac[6] = { 0 };
    LORA_Join_Network_Status_t status = loRaProcessJoinRequest(zero_mac, 6, NULL);
    TEST_ASSERT_EQUAL(LORA_JOIN_REQUEST_STATUS_ENCODE_FAILED, status);
}


void test_loRaJoinMessageHandler_with_join_type_should_delegate_correctly(void)
{
    uint8_t payload[6] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    LORA_Join_Network_Status_t status = loRaJoinMessageHandler(LORA_JOIN_TYPE_REQUEST, payload, 6);
    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_GET_JOIN_REQUEST_SUCCESS, status);
}

void test_loRaJoinMessageHandler_with_invalid_type_should_fail(void)
{
    uint8_t payload[6] = {0};
    LORA_Join_Network_Status_t status = loRaJoinMessageHandler(0xFF, payload, 6);
    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_INVALID_MAC, status);
}




void test_full_join_request_flow_should_succeed(void)
{
    // === Step 1: Tạo MAC của node (giả lập)
    uint8_t mac[6] = { 0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33 };
    
    // === Step 2: Buffer chứa frame Lora đã mã hóa
    uint8_t tx_frame[64] = {0};
    uint16_t tx_frame_len = 0;

    // === Step 3: Gọi hàm tạo JOIN request
    LORA_Join_Network_Status_t create_status = loRaCreateJoinRequest(mac, tx_frame, &tx_frame_len);
    printHexArray(tx_frame, tx_frame_len );
    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_CREATE_JOIN_REQUEST_SUCCESS, create_status);
    TEST_ASSERT_GREATER_THAN(0, tx_frame_len);

    // === Step 4: Giải mã lại frame để lấy payload (payload sẽ được xử lý trong FSM handler)
    // Giả lập: LORA_FSM sẽ nhận được `data_payload` (sau khi decode)
    // Giả định rằng `loRaDecodeFrame()` đã trả ra `OTA_FSM_Data.Buffer` và `OTA_FSM_Data.Length`
    // → Ta phải gọi loRaDecodeFrame hoặc mô phỏng nó

    LORA_frame_t OTA_FSM_Data; // Đã khai báo trong module chính
    memset(&OTA_FSM_Data, 0, sizeof(OTA_FSM_Data));

    // === Giả lập giải mã (bạn cần implement hoặc đã có hàm này)
    LORA_Frame_Status_t decode_status = loRaDecodeFrame(tx_frame, tx_frame_len, &OTA_FSM_Data);
    TEST_ASSERT_EQUAL(LORA_STATUS_DECODE_FRAME_SUCCESS, decode_status);

    // === Step 5: Gọi FSM handler
    LORA_Join_Network_Status_t handler_status = loRaJoinMessageHandler(
        OTA_FSM_Data.message_type,
        OTA_FSM_Data.data_payload,
        OTA_FSM_Data.data_len
    );

    TEST_ASSERT_EQUAL(LORA_JOIN_STATUS_GET_JOIN_REQUEST_SUCCESS, handler_status);
}


void test_full_fsm_and_decode() {
    uint8_t mac[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    uint8_t encoded_frame[LORA_PACKET_MAX_SIZE];
    uint16_t frame_len = 0;

    printf("Creating join request frame...\n");

    LORA_Join_Network_Status_t status = loRaCreateJoinRequest(mac, encoded_frame, &frame_len);
    if (status != LORA_JOIN_STATUS_CREATE_JOIN_REQUEST_SUCCESS) {
        printf("Failed to create join request\n");
        return;
    }

    printf("Join request frame created (%d bytes):\n", frame_len);
    for (int i = 0; i < frame_len; i++) {
        printf("%02X ", encoded_frame[i]);
    }
    printf("\n");

    // Reset FSM
    loRaInitFsmFrame();

    printf("Feeding FSM byte-by-byte...\n");
    for (int i = 0; i < frame_len; i++) {
        loRaGetFsmFrame(encoded_frame[i]);

        if (lora_fsm_frame.is_done == 1) {
            printf("FSM done. Attempting to decode...\n");
            LORA_frame_t decoded_frame;

            LORA_Frame_Status_t decode_status = loRaDecodeFrame(lora_fsm_frame.buffer, lora_fsm_frame.index, &decoded_frame);
            if (decode_status == LORA_STATUS_DECODE_FRAME_SUCCESS) {
                printf("Decode Success!\n");
                printf("Packet Type: %02X\n", decoded_frame.packet_type);
                printf("Message Type: %02X\n", decoded_frame.message_type);
                printf("Data Length: %d\n", decoded_frame.data_len);
                printf("Payload: ");
                for (int j = 0; j < decoded_frame.data_len; j++) {
                    printf("%02X ", decoded_frame.data_payload[j]);
                }
                printf("\n");
            } else {
                printf("Decode failed with status %d\n", decode_status);
            }

            // reset FSM cho lần test kế tiếp nếu cần
            loRaInitFsmFrame();
        }
    }
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

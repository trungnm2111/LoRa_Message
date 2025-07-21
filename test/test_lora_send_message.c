#ifdef TEST

#include "unity.h"
#include "lora_send_message.h"
#include "lora.h"
#include "crc.h"
#include "string.h"

// Giả lập biến gSysParam
LORA_join_info_t gSysParam = {
    .mac = {0x12345678, 0x9ABCDEF0}, // MAC address mẫu
    .netkey = {0, 0, 0, 0},
    .devkey = {0, 0, 0, 0},
    .unicast = 0,
    .is_joined = 0
};

// Giả lập hàm lora_send_frame để kiểm tra dữ liệu gửi đi

void setUp(void)
{
    // Khởi tạo bảng CRC và xóa buffer
    crc_init();
    memset(DATA_BUF, 0, LORA_DATA_MAX_SIZE);
    memset(mock_data, 0, LORA_PACKET_MAX_SIZE);
    mock_length = 0;
}

void tearDown(void)
{
}

void test_lora_join_network_request_frame_structure(void)
{
    // Gọi hàm
    lora_join_network_request();

    // Kiểm tra cấu trúc khung trong DATA_BUF
    TEST_ASSERT_EQUAL_HEX8(LORA_SOF, DATA_BUF[0]); // SOF = 0xAA
    TEST_ASSERT_EQUAL_HEX16(LORA_PACKET_TYPE_JOIN, *(uint16_t*)&DATA_BUF[1]); // Packet Type = 0
    TEST_ASSERT_EQUAL_HEX16(8, *(uint16_t*)&DATA_BUF[3]); // Data Len = 8
    TEST_ASSERT_EQUAL_HEX32(0x12345678, *(uint32_t*)&DATA_BUF[5]); // MAC[0]
    TEST_ASSERT_EQUAL_HEX32(0x9ABCDEF0, *(uint32_t*)&DATA_BUF[9]); // MAC[1]
    TEST_ASSERT_EQUAL_HEX8(LORA_EOF, DATA_BUF[17]); // EOF = 0xBB
}

void test_lora_join_network_request_crc(void)
{
    // Gọi hàm
    lora_join_network_request();

    // Tính CRC cho phần dữ liệu từ SOF đến Data (5 + 8 = 13 byte)
    uint8_t crc_data[13];
    memcpy(crc_data, DATA_BUF, 13);
    crc_t expected_crc = crc_fast(crc_data, 13);

    // Kiểm tra CRC trong DATA_BUF (vị trí 13-16)
    TEST_ASSERT_EQUAL_HEX32(expected_crc, *(uint32_t*)&DATA_BUF[13]);
}

void test_lora_join_network_request_send_frame(void)
{
    // Gọi hàm
    lora_join_network_request();

    // Kiểm tra dữ liệu được gửi qua lora_send_frame
    TEST_ASSERT_EQUAL(18, mock_length); // Độ dài khung: 1 + 2 + 2 + 8 + 4 + 1 = 18
    TEST_ASSERT_EQUAL_HEX8_ARRAY(DATA_BUF, mock_data, 18); // So sánh toàn bộ khung
}

void test_lora_join_network_request_empty_mac(void)
{
    // Thiết lập MAC rỗng
    // gSysParam.join_info.mac[0] = 0;
    // gSysParam.join_info.mac[1] = 0;

    gSysParam.mac[0] = 0;
    gSysParam.mac[1] = 0;

    // Gọi hàm
    lora_join_network_request();

    // Kiểm tra Data trong khung
    TEST_ASSERT_EQUAL_HEX32(0x00000000, *(uint32_t*)&DATA_BUF[5]); // MAC[0]
    TEST_ASSERT_EQUAL_HEX32(0x00000000, *(uint32_t*)&DATA_BUF[9]); // MAC[1]

    // Kiểm tra CRC
    uint8_t crc_data[13];
    memcpy(crc_data, DATA_BUF, 13);
    crc_t expected_crc = crc_fast(crc_data, 13);
    TEST_ASSERT_EQUAL_HEX32(expected_crc, *(uint32_t*)&DATA_BUF[13]);
}

// void test_lora_send_message_NeedToImplement(void)
// {
//     TEST_IGNORE_MESSAGE("Need to Implement lora_send_message");
// }

#endif // TEST

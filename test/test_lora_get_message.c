#ifdef TEST
#include <string.h>
#include "unity.h"
#include "lora_get_message.h"
#include "lora.h"
#include "crc.h"

// Giả lập biến toàn cục gLoraData và gSysParam
// typedef struct {
//     uint8_t Buffer[LORA_PACKET_MAX_SIZE];
//     uint16_t Index;
//     uint16_t Length;
// } LORA_Data_t;

LORA_Data_t gLoraData;
LORA_join_info_t gSysParam;

// Giả lập các hàm phụ thuộc
uint16_t help_convert_byte_to_uint16(uint8_t high, uint8_t low)
{
    return (uint16_t)((high << 8) | low);
}

uint32_t help_convert_bytes_to_uint32(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
    return (uint32_t)((b1 << 24) | (b2 << 16) | (b3 << 8) | b4);
}

uint32_t help_calculate_crc_bytes(uint8_t *data, uint16_t len)
{
    return crc_fast(data, len);
}

void ota_decode_frame(void)
{
    // Hàm giả lập, không làm gì
}

// Giả lập PRINTF cho debug
#define PRINTF(...)

// Hàm để gửi khung LoRa hoàn chỉnh
void send_lora_frame(uint8_t *frame, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        lora_get_frame(frame[i]);
    }
}

void setUp(void)
{
    // Khởi tạo trạng thái ban đầu
    crc_init();
    memset(&gLoraData, 0, sizeof(gLoraData));
    memset(&gSysParam, 0, sizeof(gSysParam));
    lora_init_frame();
}

void tearDown(void)
{
    // Không cần dọn dẹp
}

void test_lora_get_frame_start_valid_sof(void)
{
    // Gửi byte SOF hợp lệ
    lora_get_frame(LORA_SOF);
    
    TEST_ASSERT_EQUAL(LORA_FRAME_TYPE, lora_frame_status);
    TEST_ASSERT_EQUAL(1, gLoraData.Index);
    TEST_ASSERT_EQUAL_HEX8(LORA_SOF, gLoraData.Buffer[0]);
}

void test_lora_get_frame_start_invalid_sof(void)
{
    // Gửi byte SOF không hợp lệ
    lora_get_frame(0xFF);
    
    TEST_ASSERT_EQUAL(LORA_FRAME_START, lora_frame_status);
    TEST_ASSERT_EQUAL(0, gLoraData.Index);
    TEST_ASSERT_EQUAL_HEX8(0, gLoraData.Buffer[0]);
}

void test_lora_get_frame_type(void)
{
    // Gửi SOF và 2 byte Packet Type
    lora_get_frame(LORA_SOF);
    lora_get_frame(0x00); // LORA_PACKET_TYPE_JOIN (byte cao)
    lora_get_frame(0x00); // Byte thấp
    
    TEST_ASSERT_EQUAL(LORA_FRAME_LENGTH, lora_frame_status);
    TEST_ASSERT_EQUAL(3, gLoraData.Index);
    TEST_ASSERT_EQUAL_HEX8(LORA_SOF, gLoraData.Buffer[0]);
    TEST_ASSERT_EQUAL_HEX16(LORA_PACKET_TYPE_JOIN, *(uint16_t*)&gLoraData.Buffer[1]);
}

void test_lora_get_frame_length(void)
{
    // Gửi SOF, Packet Type, và 2 byte Length (8)
    lora_get_frame(LORA_SOF);
    lora_get_frame(0x00); // Packet Type
    lora_get_frame(0x00);
    lora_get_frame(0x00); // Length = 8 (byte cao)
    lora_get_frame(0x08); // Byte thấp
    
    TEST_ASSERT_EQUAL(LORA_FRAME_DATA, lora_frame_status);
    TEST_ASSERT_EQUAL(5, gLoraData.Index);
    TEST_ASSERT_EQUAL(8, gLoraData.Length);
}

void test_lora_get_frame_data(void)
{
    // Gửi SOF, Packet Type, Length, và 8 byte dữ liệu
    uint8_t frame[] = {
        LORA_SOF,           // SOF
        0x00, 0x00,        // Packet Type (JOIN)
        0x00, 0x08,        // Length = 8
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 // Data (MAC address)
    };
    send_lora_frame(frame, 13);
    
    TEST_ASSERT_EQUAL(LORA_FRAME_CRC, lora_frame_status);
    TEST_ASSERT_EQUAL(13, gLoraData.Index);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(frame, gLoraData.Buffer, 13);
}

void test_lora_get_frame_valid_crc_and_end(void)
{
    // Tạo khung hợp lệ với CRC
    uint8_t frame[] = {
        LORA_SOF,           // SOF
        0x00, 0x00,        // Packet Type (JOIN)
        0x00, 0x08,        // Length = 8
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, // Data (MAC address)
        0x00, 0x00, 0x00, 0x00, // CRC (sẽ thay thế sau)
        LORA_EOF           // EOF
    };
    // Tính CRC cho phần dữ liệu (13 byte: SOF + Type + Length + Data)
    uint32_t crc = crc_fast(frame, 13);
    frame[13] = (crc >> 24) & 0xFF;
    frame[14] = (crc >> 16) & 0xFF;
    frame[15] = (crc >> 8) & 0xFF;
    frame[16] = crc & 0xFF;
    
    // Gửi khung
    send_lora_frame(frame, 18);
    
    TEST_ASSERT_EQUAL(LORA_FRAME_START, lora_frame_status);
    TEST_ASSERT_EQUAL(17, gLoraData.Index);
    TEST_ASSERT_TRUE(gSysParam.is_joined);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(frame, gLoraData.Buffer, 18);
}

void test_lora_get_frame_invalid_crc(void)
{
    // Tạo khung với CRC sai
    uint8_t frame[] = {
        LORA_SOF,           // SOF
        0x00, 0x00,        // Packet Type (JOIN)
        0x00, 0x08,        // Length = 8
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, // Data
        0xFF, 0xFF, 0xFF, 0xFF, // CRC sai
        LORA_EOF           // EOF
    };
    
    // Gửi khung
    send_lora_frame(frame, 18);
    
    TEST_ASSERT_EQUAL(LORA_FRAME_START, lora_frame_status);
    TEST_ASSERT_EQUAL(0, gLoraData.Index);
    TEST_ASSERT_FALSE(gSysParam.is_joined);
}
#endif // TEST


#ifdef TEST

#include "unity.h"
#include "lora.h"
#include "lora_ota.h"
#include "crc.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static void printHexArray(const uint8_t *array, uint16_t length) 
{
    printf("Array Output (%d bytes):\n", length);
    for (uint16_t i = 0; i < length; i++) {
        printf("%02X ", array[i]);
    }
    printf("\n");
}

void test_ota_server_send_full_flow(void) {
    // Firmware giả lập: 300 bytes
    uint8_t fw_image[300];
    for (int i = 0; i < sizeof(fw_image); i++) {
        fw_image[i] = (uint8_t)(i & 0xFF);
    }

    // CRC giả lập
    uint32_t fw_crc = 0x12345678;
    uint16_t chunk_size = 64;
    uint16_t session_id = 0xBEEF;

    // Init context
    otaServerPrepare(fw_image, sizeof(fw_image), chunk_size, fw_crc, session_id);

    uint8_t frame_buf[512];
    uint16_t frame_len = 1;
    LORA_OtaStatus_t status;

    // 1. START frame
    status = otaServerNextFrame(frame_buf, &frame_len);
    TEST_ASSERT_EQUAL(LORA_OTA_STATUS_CREATE_START_SUCCESS, status);
    TEST_ASSERT_GREATER_THAN(1, frame_len);

    // 2. HEADER frame
    status = otaServerNextFrame(frame_buf, &frame_len);
    TEST_ASSERT_EQUAL(LORA_OTA_STATUS_CREATE_HEADER_SUCCESS, status);
    TEST_ASSERT_GREATER_THAN(0, frame_len);

    // 3. DATA frames (loop qua tất cả chunk)
    int total_chunks = (sizeof(fw_image) + chunk_size - 1) / chunk_size;
    for (int i = 0; i < total_chunks; i++) {
        status = otaServerNextFrame(frame_buf, &frame_len);
        TEST_ASSERT_EQUAL(LORA_OTA_STATUS_CREATE_DATA_SUCCESS, status);
        TEST_ASSERT_GREATER_THAN(0, frame_len);
    }

    // 4. END frame
    status = otaServerNextFrame(frame_buf, &frame_len);
    TEST_ASSERT_EQUAL(LORA_OTA_STATUS_CREATE_END_SUCCESS, status);
    TEST_ASSERT_GREATER_THAN(0, frame_len);

    // 5. SIGNAL_UPDATE frame
    status = otaServerNextFrame(frame_buf, &frame_len);
    TEST_ASSERT_EQUAL(LORA_OTA_STATUS_CREATE_SIGNAL_UPDATE_SUCCESS, status);
    TEST_ASSERT_GREATER_THAN(0, frame_len);
}


#endif // TEST

#ifdef TEST

#include "crc.h"
#include "unity.h"

void setUp(void)
{
    crc_init();
}

void tearDown(void)
{
}

void test_reflect(void)
{
    TEST_ASSERT_EQUAL_HEX32(0x80, reflect(0x01, 8)); // 00000001 -> 10000000
    TEST_ASSERT_EQUAL_HEX32(0xC0, reflect(0x03, 8)); // 00000011 -> 11000000
}

/* Kiểm tra bảng tra cứu sau khi khởi tạo crc_init();*/
void test_crc_init_should_create_valid_table(void)
{
    // Kiểm tra một số giá trị trong bảng tra cứu
    TEST_ASSERT_NOT_NULL(g_crc_table);
    TEST_ASSERT_EQUAL_HEX32(0x00000000, g_crc_table[0]); // Giá trị đầu tiên
    TEST_ASSERT_EQUAL_HEX32(0x04C11DB7, g_crc_table[1]); // Giá trị mẫu cho CRC_32
}

/* Kiểm tra CRC của mảng rỗng crc_slow */
void test_crc_slow_empty_data(void) 
{ 
    // Kiểm tra CRC của mảng rỗng 
    uint8_t data[] = {};
    crc_t expected = INITIAL_REMAINDER ^ FINAL_XOR_VALUE;
    
    // Với CRC_32, expected = 0xFFFFFFFF ^ 0xFFFFFFFF = 0x00000000 
    crc_t result = crc_slow(data, 0);
    
    TEST_ASSERT_EQUAL_HEX32(expected, result); 
}

/* Kiểm tra CRC của dữ liệu "12345" (0x31, 0x32, 0x33, 0x34, 0x35) */
void test_crc_slow_known_data(void) 
{ 
    uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35}; 
    int data_length = sizeof(data) / sizeof(data[0]); 
    crc_t expected = 0xCBF53A1C; // Giá trị CRC-32 đã biết cho "12345" 
    
    crc_t result = crc_slow(data, data_length); 
    
    TEST_ASSERT_EQUAL_HEX32(expected, result); 
}

/* Kiểm tra CRC của mảng rỗng */
void test_crc_fast_empty_data(void) 
{ 
    uint8_t data[] = {}; 
    crc_t expected = INITIAL_REMAINDER ^ FINAL_XOR_VALUE; 
    // Với CRC_32, expected = 0x00000000 
    crc_t result = crc_fast(data, 0); 
    
    TEST_ASSERT_EQUAL_HEX32(expected, result); 
}

/* Kiểm tra CRC của dữ liệu "12345" (0x31, 0x32, 0x33, 0x34, 0x35) */
void test_crc_fast_known_data(void) 
{ 
    uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35}; 
    int data_length = sizeof(data) / sizeof(data[0]); 
    
    crc_t expected = 0xCBF53A1C; // Giá trị CRC-32 đã biết cho "12345" 
    crc_t result = crc_fast(data, data_length); 
    
    TEST_ASSERT_EQUAL_HEX32(expected, result); 
}

/* Kiểm tra crc_fast và crc_slow cho cùng dữ liệu */
void test_crc_fast_matches_crc_slow(void) 
{ 
    uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35}; 
    int data_length = sizeof(data) / sizeof(data[0]); 

    crc_t result_slow = crc_slow(data, data_length); 
    crc_t result_fast = crc_fast(data, data_length); 
    
    TEST_ASSERT_EQUAL_HEX32(result_slow, result_fast); 
}

#endif // TEST


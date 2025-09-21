#ifdef TEST

#include "unity.h"
#include "lora.h"
#include "crc.h"
#include <stdio.h>
#include <string.h>
void print_fsm_state(LoraFsmState_t state);

void setUp(void)
{
    // setvbuf(stdout, NULL, _IONBF, 0);
}

void tearDown(void)
{
}

// /* ////////////////////////////////////////// TEST ENCODER STRUCT INPUT TO ARRAY FRAME /////////////////////////////////// */

#define TEST_SOF         0xAA
#define TEST_PACKET_TYPE 0x01
#define TEST_MESSAGE_TYPE 0x02
#define TEST_DATA_LEN     4
#define TEST_EOF         0xBB


void test_loRaEncryptedFrame_nullInput_shouldReturnError(void)
{
    // printf("Start test_MessageCreateFrameRound3x3\n");
    // fflush(stdout);

    uint8_t buffer[64] = {0};
    LORA_Frame_Status_t status = loRaEncryptedFrame(NULL, buffer);
    

    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_ERROR, status);

}

void test_loRaEncryptedFrame_nullBuffer_shouldReturnError(void)
{


    uint8_t test_payload[4] = {0x01, 0x02, 0x03, 0x04};
    LORA_frame_t input_frame = {
        .sof = TEST_SOF,
        .packet_type = TEST_PACKET_TYPE,
        .message_type = TEST_MESSAGE_TYPE,
        .data_len = 4,
        .data_payload = test_payload,
        .eof = TEST_EOF
    };


    LORA_Frame_Status_t status = loRaEncryptedFrame(&input_frame, NULL);
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_ERROR, status);
}

void test_loRaEncryptedFrame_dataLenTooLarge_shouldReturnError(void)
{

    uint8_t dummy_payload[300]; // > LORA_MAX_DATA_LEN nếu được định nghĩa nhỏ hơn 300
    memset(dummy_payload, 0xAA, sizeof(dummy_payload));
    
    LORA_frame_t input_frame = {
        .sof = TEST_SOF,
        .packet_type = TEST_PACKET_TYPE,
        .message_type = TEST_MESSAGE_TYPE,
        .data_len = sizeof(dummy_payload),
        .data_payload = dummy_payload,
        .eof = TEST_EOF
    };
    uint8_t buffer[64] = {0};
    LORA_Frame_Status_t status = loRaEncryptedFrame(&input_frame, buffer);

    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_OVERLOAD_DATA, status); // nếu bạn có error này
}


void test_loRaEncryptedFrame_nullPayloadWithNonzeroLength_shouldReturnError(void)
{

    LORA_frame_t input_frame = {
        .sof = TEST_SOF,
        .packet_type = TEST_PACKET_TYPE,
        .message_type = TEST_MESSAGE_TYPE,
        .data_len = 5,
        .data_payload = NULL, // lỗi
        .eof = TEST_EOF
    };
    
    uint8_t buffer[64] = {0};

    LORA_Frame_Status_t status = loRaEncryptedFrame(&input_frame, buffer);
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_ERROR, status);
    

}


void test_loRaEncryptedFrame_validInput_shouldEncodeCorrectly(void)
{
    crc_init();
    // Prepare input frame
    uint8_t test_payload[TEST_DATA_LEN] = {0x11, 0x22, 0x33, 0x44};
    LORA_frame_t input_frame = {
        .sof = TEST_SOF,
        .packet_type = TEST_PACKET_TYPE,
        .message_type = TEST_MESSAGE_TYPE,
        .data_len = TEST_DATA_LEN,
        .data_payload = test_payload,
        .eof = TEST_EOF
    };

    // Output buffer
    uint8_t encoded_buffer[64] = {0};
    LORA_Frame_Status_t status = loRaEncryptedFrame(&input_frame, encoded_buffer);

    // Kiểm tra status
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_SUCCESS, status);

    // So sánh nội dung buffer
    TEST_ASSERT_EQUAL_HEX8(TEST_SOF,         encoded_buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(TEST_PACKET_TYPE, encoded_buffer[1]);
    TEST_ASSERT_EQUAL_HEX8(TEST_MESSAGE_TYPE,encoded_buffer[2]);
    TEST_ASSERT_EQUAL_HEX8(TEST_DATA_LEN & 0xFF,  encoded_buffer[3]);
    TEST_ASSERT_EQUAL_HEX8((TEST_DATA_LEN >> 8),  encoded_buffer[4]);

    // Payload
    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_payload, &encoded_buffer[5], TEST_DATA_LEN);

    // CRC
    uint16_t offset_after_payload = 5 + TEST_DATA_LEN;
    crc_t expected_crc = crc_fast(encoded_buffer, offset_after_payload);
    crc_t actual_crc;
    memcpy(&actual_crc, &encoded_buffer[offset_after_payload], sizeof(crc_t));
    TEST_ASSERT_EQUAL_UINT32(expected_crc, actual_crc);

    // EOF
    TEST_ASSERT_EQUAL_HEX8(TEST_EOF, encoded_buffer[offset_after_payload + sizeof(crc_t)]);
}

void test_loRaEncryptedFrame_dataLenZero_shouldEncodeCorrectly(void)
{
    crc_init();
    uint8_t test_payload[1] = {0}; // không dùng tới
    LORA_frame_t input_frame = {
        .sof = TEST_SOF,
        .packet_type = TEST_PACKET_TYPE,
        .message_type = TEST_MESSAGE_TYPE,
        .data_len = 0,
        .data_payload = test_payload, // vẫn cần không NULL
        .eof = TEST_EOF
    };

    uint8_t encoded_buffer[64] = {0};
    LORA_Frame_Status_t status = loRaEncryptedFrame(&input_frame, encoded_buffer);
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_SUCCESS, status);

    TEST_ASSERT_EQUAL_HEX8(TEST_SOF, encoded_buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(TEST_PACKET_TYPE, encoded_buffer[1]);
    TEST_ASSERT_EQUAL_HEX8(TEST_MESSAGE_TYPE, encoded_buffer[2]);
    TEST_ASSERT_EQUAL_HEX8(0, encoded_buffer[3]);
    TEST_ASSERT_EQUAL_HEX8(0, encoded_buffer[4]);

    uint16_t offset = 5; // Không có payload
    crc_t expected_crc = crc_fast(encoded_buffer, offset);
    crc_t actual_crc;
    memcpy(&actual_crc, &encoded_buffer[offset], sizeof(crc_t));
    TEST_ASSERT_EQUAL_UINT32(expected_crc, actual_crc);

    TEST_ASSERT_EQUAL_HEX8(TEST_EOF, encoded_buffer[offset + sizeof(crc_t)]);
}

void test_loRaEncryptedFrame_dataLenOne_shouldEncodeCorrectly(void)
{
    crc_init();
    uint8_t test_payload[1] = {0xAB};
    LORA_frame_t input_frame = {
        .sof = TEST_SOF,
        .packet_type = TEST_PACKET_TYPE,
        .message_type = TEST_MESSAGE_TYPE,
        .data_len = 1,
        .data_payload = test_payload,
        .eof = TEST_EOF
    };

    uint8_t encoded_buffer[64] = {0};
    LORA_Frame_Status_t status = loRaEncryptedFrame(&input_frame, encoded_buffer);
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_SUCCESS, status);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_payload, &encoded_buffer[5], 1);

    uint16_t offset = 6; // 5 header + 1 payload
    crc_t expected_crc = crc_fast(encoded_buffer, offset);
    crc_t actual_crc;
    memcpy(&actual_crc, &encoded_buffer[offset], sizeof(crc_t));
    TEST_ASSERT_EQUAL_UINT32(expected_crc, actual_crc);

    TEST_ASSERT_EQUAL_HEX8(TEST_EOF, encoded_buffer[offset + sizeof(crc_t)]);
}


void test_loRaEncryptedFrame_dataLen16_shouldEncodeCorrectly(void)
{
    crc_init();
    uint8_t test_payload[16];
    for (int i = 0; i < 16; i++) test_payload[i] = i;

    LORA_frame_t input_frame = {
        .sof = TEST_SOF,
        .packet_type = TEST_PACKET_TYPE,
        .message_type = TEST_MESSAGE_TYPE,
        .data_len = 16,
        .data_payload = test_payload,
        .eof = TEST_EOF
    };

    uint8_t encoded_buffer[64] = {0};
    LORA_Frame_Status_t status = loRaEncryptedFrame(&input_frame, encoded_buffer);
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_SUCCESS, status);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_payload, &encoded_buffer[5], 16);

    uint16_t offset = 5 + 16;
    crc_t expected_crc = crc_fast(encoded_buffer, offset);
    crc_t actual_crc;
    memcpy(&actual_crc, &encoded_buffer[offset], sizeof(crc_t));
    TEST_ASSERT_EQUAL_UINT32(expected_crc, actual_crc);

    TEST_ASSERT_EQUAL_HEX8(TEST_EOF, encoded_buffer[offset + sizeof(crc_t)]);
}


void test_loRaEncryptedFrame_dataLen63_shouldEncodeCorrectly(void)
{
    crc_init();
    uint8_t test_payload[63];
    for (int i = 0; i < 63; i++) test_payload[i] = i;

    LORA_frame_t input_frame = {
        .sof = TEST_SOF,
        .packet_type = TEST_PACKET_TYPE,
        .message_type = TEST_MESSAGE_TYPE,
        .data_len = 63,
        .data_payload = test_payload,
        .eof = TEST_EOF
    };

    uint8_t encoded_buffer[128] = {0}; // tăng buffer để chứa được

    LORA_Frame_Status_t status = loRaEncryptedFrame(&input_frame, encoded_buffer);
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_SUCCESS, status);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_payload, &encoded_buffer[5], 63);

    uint16_t offset = 5 + 63;
    crc_t expected_crc = crc_fast(encoded_buffer, offset);
    crc_t actual_crc;
    memcpy(&actual_crc, &encoded_buffer[offset], sizeof(crc_t));
    TEST_ASSERT_EQUAL_UINT32(expected_crc, actual_crc);

    TEST_ASSERT_EQUAL_HEX8(TEST_EOF, encoded_buffer[offset + sizeof(crc_t)]);
}

/* ////////////////////////////////////////// TEST GET FSM FRAME /////////////////////////////////////////////////////////////// */


void test_loRaGetFsmFrame_shouldReceiveCompleteFrame(void) {
    printf("Start Test Fsm\n");
    fflush(stdout);

    // Chuẩn bị dữ liệu input
    uint8_t payload[] = {0x11, 0x22, 0x33, 0x44};
    uint16_t payload_len = sizeof(payload);
    LORA_frame_t test_frame = {
        .sof = LORA_SOF,
        .packet_type = 0xA1,
        .message_type = 0xB2,
        .data_len = payload_len,
        .data_payload = payload,
        .eof = LORA_EOF
    };
    printf("Create buffer encoded\n");
    // Tạo buffer encoded
    uint8_t encoded_buffer[LORA_PACKET_MAX_SIZE] = {0};
    LORA_Frame_Status_t status = loRaEncryptedFrame(&test_frame, encoded_buffer);
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_SUCCESS, status);

    printf("Caculate length frame encoded\n");
    // Tính độ dài frame thực tế
    uint16_t encoded_len = LORA_FRAME_HEADER_SIZE + payload_len + sizeof(crc_t) + 1 ; // +1 EOF

    // Gửi từng byte vào FSM
    printf("Start send each byte to fsm receive\n");
    for (uint16_t i = 0; i < encoded_len; i++) {
        loRaGetFsmFrame(encoded_buffer[i]);
        printf("Send byte: %02X, state: %d\n", encoded_buffer[i], lora_fsm_frame.state);
    }
    
    TEST_ASSERT_EQUAL(encoded_len, lora_fsm_frame.index);
    TEST_ASSERT_EQUAL(1, lora_fsm_frame.is_done);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(encoded_buffer, lora_fsm_frame.buffer, encoded_len);
}

void test_loRaGetFsmFrame_wrongCRC_shouldRejectFrame(void) {
    // Tạo frame hợp lệ
    uint8_t payload[] = {0x55, 0x66, 0x77};
    LORA_frame_t frame = {
        .sof = LORA_SOF,
        .packet_type = 0x01,
        .message_type = 0x02,
        .data_len = sizeof(payload),
        .data_payload = payload,
        .eof = LORA_EOF,
    };

    uint8_t encoded[LORA_PACKET_MAX_SIZE] = {0};
    loRaEncryptedFrame(&frame, encoded);

    // Làm CRC bị sai
    encoded[LORA_FRAME_HEADER_SIZE + sizeof(payload)] ^= 0xFF;

    // Reset FSM
    loRaInitFsmFrame();

    // Gửi toàn bộ frame
    uint16_t total_len = LORA_FRAME_HEADER_SIZE + sizeof(payload) + sizeof(crc_t) + 1;
    for (int i = 0; i < total_len; i++) {
        loRaGetFsmFrame(encoded[i]);
    }

    // FSM không nên set is_done
    TEST_ASSERT_EQUAL(0, lora_fsm_frame.is_done);
}




void print_fsm_state(LoraFsmState_t state)
{
    switch (state) {
        case LORA_FSM_STATE_START:    printf("FSM State: RECV_SOF\n , %d",lora_fsm_frame.is_done ); break;
        case LORA_FSM_STATE_TYPE: printf("FSM State: RECV_TYPE\n", lora_fsm_frame.is_done); break;
        case LORA_FSM_STATE_LENGTH:   printf("FSM State: RECV_LENGTH\n"); break;
        case LORA_FSM_STATE_DATA:    printf("FSM State: RECV_DATA\n"); break;
        case LORA_FSM_STATE_CRC:    printf("FSM State: RECV_CRC\n"); break;
        case LORA_FSM_STATE_END:        printf("FSM State: RECV_EOF DONE\n"); break;
        default:                   printf("FSM State: UNKNOWN\n"); break;
    }
    if(lora_fsm_frame.is_done == 1)
    {
        printf("flag fsm done , is_done = 1");
        fflush(stdout);
    } 
}
/* ////////////////////////////////////////// TEST DECODE ARRAY FRAME TO STRUCT DATA OUTPUT  /////////////////////////////////// */


void test_loRaDecodeFrame_shouldReturnValidStruct(void) {
    uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    LORA_frame_t input_frame = {
        .sof = LORA_SOF,
        .packet_type = 0x01,
        .message_type = 0x02,
        .data_len = sizeof(payload),
        .data_payload = payload,
        .eof = LORA_EOF
    };

    uint8_t encoded[LORA_PACKET_MAX_SIZE] = {0};
    LORA_Frame_Status_t enc_status = loRaEncryptedFrame(&input_frame, encoded);
    
    TEST_ASSERT_EQUAL(LORA_STATUS_ENCODE_FRAME_SUCCESS, enc_status);

    LORA_frame_t decoded_frame = {0};
    LORA_Frame_Status_t dec_status = loRaDecodeFrame(encoded, LORA_FRAME_HEADER_SIZE + sizeof(payload) + sizeof(crc_t) + 1, &decoded_frame);
    TEST_ASSERT_EQUAL(LORA_STATUS_DECODE_FRAME_SUCCESS, dec_status);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(payload, decoded_frame.data_payload, decoded_frame.data_len);
    TEST_ASSERT_EQUAL(input_frame.packet_type, decoded_frame.packet_type);
    TEST_ASSERT_EQUAL(input_frame.message_type, decoded_frame.message_type);
}

#endif // TEST

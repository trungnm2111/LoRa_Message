
#ifdef TEST

#include "unity.h"

#include "lora_node_storage.h"


void setUp(void) {
    storageClearAll();
    storageInit();
}

void tearDown(void) {
    /* nothing */
}

void test_storage_init_idempotent(void) {
    // calling twice should be safe
    // storageInit();
    TEST_ASSERT_FALSE(storageHas(STORAGE_NODE_MAC));
    TEST_ASSERT_FALSE(storageHas(STORAGE_NETWORK_KEY));
    TEST_ASSERT_FALSE(storageHas(STORAGE_GATEWAY_MAC));
    TEST_ASSERT_FALSE(storageHas(STORAGE_JOIN_STATE));
    TEST_ASSERT_FALSE(storageHas(STORAGE_DEVICE_ADDRESS));
    TEST_ASSERT_FALSE(storageHas(STORAGE_MESSAGE_COUNTER));
}

void test_storage_set_get_node_mac_and_flags(void) {
    storageInit();
    uint8_t mac[6] = {0xDE,0xAD,0xBE,0xEF,0x00,0x01};
    TEST_ASSERT_TRUE(storageSet(STORAGE_NODE_MAC, mac, sizeof(mac)));
    TEST_ASSERT_TRUE(storageHas(STORAGE_NODE_MAC));

    uint8_t out[6] = {0};
    TEST_ASSERT_TRUE(storageGet(STORAGE_NODE_MAC, out, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(mac, out, 6);
}

void test_storage_set_get_network_key_and_gateway(void) {
    uint8_t key[16];
    for (int i=0;i<16;i++) key[i] = (uint8_t)i;
    uint8_t gw[6] = {1,2,3,4,5,6};

    TEST_ASSERT_TRUE(storageSet(STORAGE_NETWORK_KEY, key, sizeof(key)));
    TEST_ASSERT_TRUE(storageSet(STORAGE_GATEWAY_MAC, gw, sizeof(gw)));

    uint8_t outkey[16] = {0};
    uint8_t outgw[6] = {0};

    TEST_ASSERT_TRUE(storageGet(STORAGE_NETWORK_KEY, outkey, sizeof(outkey)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(key, outkey, 16);

    TEST_ASSERT_TRUE(storageGet(STORAGE_GATEWAY_MAC, outgw, sizeof(outgw)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(gw, outgw, 6);
}

void test_storage_invalid_size_and_null_pointer_handling(void) {
    uint8_t smallbuf[4];
    uint32_t addr = 0x11223344;
    uint16_t counter = 123;

    // NULL data pointer for set/get => false
    TEST_ASSERT_FALSE(storageSet(STORAGE_NODE_MAC, NULL, 6));
    TEST_ASSERT_FALSE(storageGet(STORAGE_NODE_MAC, NULL, 6));

    // wrong size for MAC
    TEST_ASSERT_FALSE(storageSet(STORAGE_NODE_MAC, smallbuf, sizeof(smallbuf)));

    // wrong size for device address
    TEST_ASSERT_FALSE(storageSet(STORAGE_DEVICE_ADDRESS, &addr, sizeof(uint16_t))); // too small

    // wrong size for message counter
    TEST_ASSERT_FALSE(storageSet(STORAGE_MESSAGE_COUNTER, &counter, sizeof(uint32_t))); // too big
}

void test_storage_join_state_and_device_address_and_message_counter(void) {
    // Replace the cast below with your real enum constant if available,
    // e.g. node_join_state_t state = NODE_JOIN_STATE_JOINED;
    node_join_state_t state = (node_join_state_t)1;
    uint32_t addr = 0xAABBCCDD;
    uint16_t counter = 0x1234;

    TEST_ASSERT_TRUE(storageSet(STORAGE_JOIN_STATE, &state, sizeof(state)));
    TEST_ASSERT_TRUE(storageSet(STORAGE_DEVICE_ADDRESS, &addr, sizeof(addr)));
    TEST_ASSERT_TRUE(storageSet(STORAGE_MESSAGE_COUNTER, &counter, sizeof(counter)));

    node_join_state_t recovered_state;
    uint32_t oaddr = 0;
    uint16_t ocounter = 0;

    TEST_ASSERT_TRUE(storageGet(STORAGE_JOIN_STATE, &recovered_state, sizeof(recovered_state)));
    TEST_ASSERT_EQUAL((int)state, (int)recovered_state);

    TEST_ASSERT_TRUE(storageGet(STORAGE_DEVICE_ADDRESS, &oaddr, sizeof(oaddr)));
    TEST_ASSERT_EQUAL_UINT32(addr, oaddr);

    TEST_ASSERT_TRUE(storageGet(STORAGE_MESSAGE_COUNTER, &ocounter, sizeof(ocounter)));
    TEST_ASSERT_EQUAL_UINT16(counter, ocounter);
}

void test_storage_get_and_increment_counter_behaviour(void) {
    // default counter is 0 after clearAll
    uint16_t cur = storageGetAndIncrementCounter();
    TEST_ASSERT_EQUAL_UINT16(0, cur);

    uint16_t cur2 = storageGetAndIncrementCounter();
    TEST_ASSERT_EQUAL_UINT16(1, cur2);

    TEST_ASSERT_TRUE(storageHas(STORAGE_MESSAGE_COUNTER));

    uint16_t value = 0;
    TEST_ASSERT_TRUE(storageGet(STORAGE_MESSAGE_COUNTER, &value, sizeof(value)));
    // After two increments the stored counter should be 2
    TEST_ASSERT_EQUAL_UINT16(2, value);
}

void test_storage_clear_all_resets_flags_and_values(void) {
    uint8_t mac[6] = {9,9,9,9,9,9};
    uint16_t count = 7;

    TEST_ASSERT_TRUE(storageSet(STORAGE_NODE_MAC, mac, sizeof(mac)));
    TEST_ASSERT_TRUE(storageSet(STORAGE_MESSAGE_COUNTER, &count, sizeof(count)));

    storageClearAll();

    TEST_ASSERT_FALSE(storageHas(STORAGE_NODE_MAC));
    TEST_ASSERT_FALSE(storageHas(STORAGE_MESSAGE_COUNTER));

    uint8_t out[6];
    uint16_t outcount;
    TEST_ASSERT_FALSE(storageGet(STORAGE_NODE_MAC, out, sizeof(out)));
    TEST_ASSERT_FALSE(storageGet(STORAGE_MESSAGE_COUNTER, &outcount, sizeof(outcount)));
}
#endif // TEST

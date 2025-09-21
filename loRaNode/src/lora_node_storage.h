#ifndef LORA_NODE_STORAGE_H
#define LORA_NODE_STORAGE_H

#include "lora.h"
#include <stdint.h>
#include <stdbool.h>

// ===========================================
// STORAGE STRUCTURE
// ===========================================

// Storage data types
typedef enum {
    STORAGE_NODE_MAC = 0,      // 6 bytes
    STORAGE_NETWORK_KEY,       // 16 bytes  
    STORAGE_GATEWAY_MAC,       // 6 bytes
    STORAGE_JOIN_STATE,        // 1 byte
    STORAGE_DEVICE_ADDRESS,    // 4 bytes
    STORAGE_MESSAGE_COUNTER    // 2 bytes
} storage_type_t;

// Node join state
typedef enum {
    NODE_UNJOINED = 0,
    NODE_JOINED = 1
} node_join_state_t;

typedef struct {
    // Node identity
    uint8_t node_mac[6];
    bool has_mac;
    
    // Network credentials
    uint8_t network_key[16];
    bool has_network_key;
    uint32_t device_address;
    bool has_device_address;
    
    // Join state
    node_join_state_t join_state;
    bool has_join_state;
    
    // Message counter
    uint16_t message_counter;
    bool has_message_counter;
    
    // Gateway info
    uint8_t gateway_mac[6];
    bool has_gateway;
    
} node_info_t;

extern node_info_t storage_data;

void storageInit(void) ;
bool storageSet(storage_type_t type, const void *data, uint16_t size) ;
bool storageGet(storage_type_t type, void *data, uint16_t size) ;
bool storageHas(storage_type_t type);
void storageClearAll(void);

#endif // LORA_NODE_STORAGE_H

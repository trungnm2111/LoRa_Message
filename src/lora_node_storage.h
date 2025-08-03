#ifndef LORA_NODE_STORAGE_H
#define LORA_NODE_STORAGE_H

#include "lora.h"

typedef enum {
    NODE_STATE_IDLE = 0,
    NODE_STATE_JOIN_REQUESTING,
    NODE_STATE_JOIN_ACCEPTING,
    NODE_STATE_JOIN_CONFIRMING,
    NODE_STATE_JOINED,
    NODE_STATE_JOIN_FAILED
} node_join_state_t;


typedef struct {
    // Node identity
    uint8_t mac_address[6];
    
    // Join process state
    node_join_state_t join_state;
    uint32_t join_start_time;
    uint32_t last_activity_time;
    uint8_t join_retry_count;
    
    // Security keys (nhận từ gateway)
    uint8_t net_key[16];        // Từ JOIN_ACCEPT
    uint8_t dev_key[16];        // Gửi trong JOIN_CONFIRM
    uint16_t unicast_addr;      // Từ JOIN_COMPLETE
    
    // Status flags
    uint8_t is_joined;
    uint8_t has_net_key;
    uint8_t has_unicast_addr;
    
} node_info_t;

// Global node storage
extern node_info_t g_node_info;

#endif // LORA_NODE_STORAGE_H

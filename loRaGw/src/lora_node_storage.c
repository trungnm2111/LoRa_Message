#include "lora_node_storage.h"

// // lora_node_storage.c
// node_info_t g_node_info = {0};

// // Initialize node storage
// void loRaNodeStorageInit(const uint8_t *mac_address)
// {
//     memset(&g_node_info, 0, sizeof(node_info_t));
    
//     if (mac_address) {
//         memcpy(g_node_info.mac_address, mac_address, 6);
//     } else {
//         // Generate MAC từ chip ID
//         loRaGenerateMacFromChipId(g_node_info.mac_address);
//     }
    
//     g_node_info.join_state = NODE_STATE_IDLE;
//     g_node_info.is_joined = 0;
//     g_node_info.has_net_key = 0;
//     g_node_info.has_unicast_addr = 0;
    
//     //printf("Node storage initialized with MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
//         //    g_node_info.mac_address[0], g_node_info.mac_address[1],
//         //    g_node_info.mac_address[2], g_node_info.mac_address[3],
//         //    g_node_info.mac_address[4], g_node_info.mac_address[5]);
// }

// // Update node join state
// void loRaNodeUpdateJoinState(node_join_state_t new_state)
// {
//     //printf("Node state: %d -> %d\n", g_node_info.join_state, new_state);
//     g_node_info.join_state = new_state;
//     g_node_info.last_activity_time = HAL_GetTick(); // hoặc system time function
// }

// // Save network key (từ JOIN_ACCEPT)
// LORA_Frame_Status_t loRaNodeSaveNetworkKey(const uint8_t *net_key)
// {
//     if (!net_key) {
//         return LORA_STATUS_DECODE_FRAME_ERROR;
//     }
    
//     memcpy(g_node_info.net_key, net_key, 16);
//     g_node_info.has_net_key = 1;
    
//     //printf("Network key saved: ");
//     for (int i = 0; i < 16; i++) {
//         //printf("%02X", g_node_info.net_key[i]);
//     }
//     //printf("\n");
    
//     return LORA_STATUS_DECODE_FRAME_SUCCESS;
// }

// // Save unicast address (từ JOIN_COMPLETE)
// LORA_Frame_Status_t loRaNodeSaveUnicastAddr(uint16_t unicast_addr)
// {
//     g_node_info.unicast_addr = unicast_addr;
//     g_node_info.has_unicast_addr = 1;
//     g_node_info.is_joined = 1;
    
//     //printf("Unicast address assigned: 0x%04X\n", unicast_addr);
    
//     return LORA_STATUS_DECODE_FRAME_SUCCESS;
// }

// // Get node info
// const node_info_t* loRaNodeGetInfo(void)
// {
//     return &g_node_info;
// }

// // Check if node is joined
// uint8_t loRaNodeIsJoined(void)
// {
//     return g_node_info.is_joined;
// }

node_info_t storage_data;
static bool storage_initialized = false;


// No complex lookup table needed!

// ===========================================
// CORE STORAGE FUNCTIONS
// ===========================================

void storageInit(void) 
{
    if (storage_initialized) {
        return;
    }
    
    // Initialize to zero
    memset(&storage_data, 0, sizeof(storage_data));
    
    storage_initialized = true;
}

bool storageSet(storage_type_t type, const void *data, uint16_t size) 
{
    if (!data) return false;
    
    switch (type) {
        case STORAGE_NODE_MAC:
            if (size != 6) return false;
            memcpy(storage_data.node_mac, data, 6);
            storage_data.has_mac = true;
            break;
            
        case STORAGE_NETWORK_KEY:
            if (size != 16) return false;
            memcpy(storage_data.network_key, data, 16);
            storage_data.has_network_key = true;
            break;
            
        case STORAGE_GATEWAY_MAC:
            if (size != 6) return false;
            memcpy(storage_data.gateway_mac, data, 6);
            storage_data.has_gateway = true;
            break;
            
        case STORAGE_JOIN_STATE:
            if (size != sizeof(node_join_state_t)) return false;
            storage_data.join_state = *(node_join_state_t*)data;
            storage_data.has_join_state = true;
            break;
            
        case STORAGE_DEVICE_ADDRESS:
            if (size != sizeof(uint32_t)) return false;
            storage_data.device_address = *(uint32_t*)data;
            storage_data.has_device_address = true;
            break;
            
        case STORAGE_MESSAGE_COUNTER:
            if (size != sizeof(uint16_t)) return false;
            storage_data.message_counter = *(uint16_t*)data;
            storage_data.has_message_counter = true;
            break;
            
        default:
            return false;
    }
    
    return true;
}

bool storageGet(storage_type_t type, void *data, uint16_t size) 
{
    if (!data) return false;
    
    switch (type) {
        case STORAGE_NODE_MAC:
            if (size != 6 || !storage_data.has_mac) return false;
            memcpy(data, storage_data.node_mac, 6);
            break;
            
        case STORAGE_NETWORK_KEY:
            if (size != 16 || !storage_data.has_network_key) return false;
            memcpy(data, storage_data.network_key, 16);
            break;
            
        case STORAGE_GATEWAY_MAC:
            if (size != 6 || !storage_data.has_gateway) return false;
            memcpy(data, storage_data.gateway_mac, 6);
            break;
            
        case STORAGE_JOIN_STATE:
            if (size != sizeof(node_join_state_t) || !storage_data.has_join_state) return false;
            *(node_join_state_t*)data = storage_data.join_state;
            break;
            
        case STORAGE_DEVICE_ADDRESS:
            if (size != sizeof(uint32_t) || !storage_data.has_device_address) return false;
            *(uint32_t*)data = storage_data.device_address;
            break;
            
        case STORAGE_MESSAGE_COUNTER:
            if (size != sizeof(uint16_t) || !storage_data.has_message_counter) return false;
            *(uint16_t*)data = storage_data.message_counter;
            break;
            
        default:
            return false;
    }
    
    return true;
}

bool storageHas(storage_type_t type) 
{
    switch (type) {
        case STORAGE_NODE_MAC:
            return storage_data.has_mac;
        case STORAGE_NETWORK_KEY:
            return storage_data.has_network_key;
        case STORAGE_GATEWAY_MAC:
            return storage_data.has_gateway;
        case STORAGE_JOIN_STATE:
            return storage_data.has_join_state;
        case STORAGE_DEVICE_ADDRESS:
            return storage_data.has_device_address;
        case STORAGE_MESSAGE_COUNTER:
            return storage_data.has_message_counter;
        default:
            return false;
    }
}

// ===========================================
// CONVENIENCE FUNCTIONS
// ===========================================

uint16_t storageGetAndIncrementCounter(void) 
{
    uint16_t current = storage_data.message_counter;
    
    // Increment counter
    storage_data.message_counter++;
    storage_data.has_message_counter = true;
    
    return current;
}

void storageClearAll(void) 
{
    memset(&storage_data, 0, sizeof(storage_data));
}
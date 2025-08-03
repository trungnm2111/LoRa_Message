#include "lora_node_storage.h"

// lora_node_storage.c
node_info_t g_node_info = {0};

// Initialize node storage
void loRaNodeStorageInit(const uint8_t *mac_address)
{
    memset(&g_node_info, 0, sizeof(node_info_t));
    
    if (mac_address) {
        memcpy(g_node_info.mac_address, mac_address, 6);
    } else {
        // Generate MAC từ chip ID
        loRaGenerateMacFromChipId(g_node_info.mac_address);
    }
    
    g_node_info.join_state = NODE_STATE_IDLE;
    g_node_info.is_joined = 0;
    g_node_info.has_net_key = 0;
    g_node_info.has_unicast_addr = 0;
    
    //printf("Node storage initialized with MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
        //    g_node_info.mac_address[0], g_node_info.mac_address[1],
        //    g_node_info.mac_address[2], g_node_info.mac_address[3],
        //    g_node_info.mac_address[4], g_node_info.mac_address[5]);
}

// Update node join state
void loRaNodeUpdateJoinState(node_join_state_t new_state)
{
    //printf("Node state: %d -> %d\n", g_node_info.join_state, new_state);
    g_node_info.join_state = new_state;
    g_node_info.last_activity_time = HAL_GetTick(); // hoặc system time function
}

// Save network key (từ JOIN_ACCEPT)
LORA_Frame_Status_t loRaNodeSaveNetworkKey(const uint8_t *net_key)
{
    if (!net_key) {
        return LORA_STATUS_DECODE_FRAME_ERROR;
    }
    
    memcpy(g_node_info.net_key, net_key, 16);
    g_node_info.has_net_key = 1;
    
    //printf("Network key saved: ");
    for (int i = 0; i < 16; i++) {
        //printf("%02X", g_node_info.net_key[i]);
    }
    //printf("\n");
    
    return LORA_STATUS_DECODE_FRAME_SUCCESS;
}

// Save unicast address (từ JOIN_COMPLETE)
LORA_Frame_Status_t loRaNodeSaveUnicastAddr(uint16_t unicast_addr)
{
    g_node_info.unicast_addr = unicast_addr;
    g_node_info.has_unicast_addr = 1;
    g_node_info.is_joined = 1;
    
    //printf("Unicast address assigned: 0x%04X\n", unicast_addr);
    
    return LORA_STATUS_DECODE_FRAME_SUCCESS;
}

// Get node info
const node_info_t* loRaNodeGetInfo(void)
{
    return &g_node_info;
}

// Check if node is joined
uint8_t loRaNodeIsJoined(void)
{
    return g_node_info.is_joined;
}

// Print node status
void loRaNodePrintStatus(void)
{
    //printf("=== NODE STATUS ===\n");
    //printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
        //    g_node_info.mac_address[0], g_node_info.mac_address[1],
        //    g_node_info.mac_address[2], g_node_info.mac_address[3],
        //    g_node_info.mac_address[4], g_node_info.mac_address[5]);
    //printf("Join State: %d\n", g_node_info.join_state);
    //printf("Is Joined: %d\n", g_node_info.is_joined);
    //printf("Has Net Key: %d\n", g_node_info.has_net_key);
    //printf("Has Unicast Addr: %d (0x%04X)\n", g_node_info.has_unicast_addr, g_node_info.unicast_addr);
    //printf("Join Retry Count: %d\n", g_node_info.join_retry_count);
    //printf("==================\n");
}
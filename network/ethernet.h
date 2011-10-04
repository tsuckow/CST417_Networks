#pragma once
#include "EMAC.h"

#define ETH_DA_OFS           0                   // Destination MAC address (48 Bit)
#define ETH_SA_OFS           6                   // Source MAC address (48 Bit)
#define ETH_TYPE_OFS         12                  // Type field (16 Bit)
#define ETH_DATA_OFS         14                  // Frame Data
#define ETH_HEADER_SIZE      14

void ethernet_init(void);

void ethernet_interrupt_handler(void);

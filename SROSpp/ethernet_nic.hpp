#pragma once

#include "ethernet.hpp"

class EthernetFilter
{
public:
	//Src Addr
	//Dest Addr
	
	bool filterType;
	uint16_t type;
};

class Ethernet_NIC
{
public:
	virtual void init() = 0;
};

class Ethernet_NIC_LPC23xx : public Ethernet_NIC
{
	uint8_t frame[ETH_MAX_FLEN];

public:
	virtual void init()
	{
		ethernet_init();
	}
};
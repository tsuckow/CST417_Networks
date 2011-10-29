#pragma once

class ARPCacheEntry
{
	//IP4Addr
	//EthernetAddress
};

static uint8_t const ARP_HTYPE_ETHERNET = 0x0001;
static uint8_t const ARP_PTYPE_IPV4     = 0x0800;

static uint8_t const ARP_HSIZE_ETHERNET = 6;
static uint8_t const ARP_PSIZE_IPV4     = 4;

class ARPFrame_Eth_IPv4
{
	EthernetFrame * const eframe;

	ARPFrame_Eth_IPv4( EthernetFrame * eframe )	: eframe(eframe)
	{
	}

	bool isValid()
	{
		//TODO
	}
};
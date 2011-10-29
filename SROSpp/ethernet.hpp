#pragma once

#include <network/ethernet.h>
#include "mailbox.hpp"
#include <stdint.h>
#include <string.h>

static uint16_t const ETHERNET_TYPE_ARP = 0x0806;
//static uint16_t const ETHERNET_TYPE_IPV4 = ;


class EthernetAddress
{
	static size_t const SIZE = 6;
	uint8_t addr[SIZE];

public:
	EthernetAddress( uint8_t * addr_ )
	{
		memcpy( addr, addr_, SIZE );
	}

	EthernetAddress( EthernetAddress const & addr_ )
	{
		memcpy( addr, addr_.addr, SIZE );
	}

	bool operator==( EthernetAddress const & rhs )
	{
		return
			addr[0] == rhs.addr[0] &&
			addr[1] == rhs.addr[1] &&
			addr[2] == rhs.addr[2] &&
			addr[3] == rhs.addr[3] &&
			addr[4] == rhs.addr[4] &&
			addr[5] == rhs.addr[5]
			;
	}
};

class EthernetFrame
{
	uint8_t buffer[ETH_MAX_FLEN];
	uint_fast16_t size;

public:
	EthernetFrame() : size(0)
	{
	}

	EthernetFrame( uint8_t * frame_buffer, uint_fast16_t frame_size ) : size( frame_size )
	{
		memcpy( buffer, frame_buffer, frame_size );
	}

	EthernetAddress getDestination()
	{
		return EthernetAddress( &buffer[ETH_DA_OFS] );
	}

    EthernetAddress getSource()
	{
		return EthernetAddress( &buffer[ETH_SA_OFS] );
	}

	uint16_t getEtherType()
	{
		return (*(buffer + ETH_TYPE_OFS) << 8) + *(buffer + ETH_TYPE_OFS + 1);
	}

	uint8_t * payload()
	{
		return &buffer[ETH_DATA_OFS];
	}

	uint_fast16_t getPayloadSize()
	{
		return size - ETH_HEADER_SIZE;
	}	
};

extern Mailbox<EthernetFrame> eth_incoming;

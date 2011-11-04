#pragma once

#include <network/ethernet.h>
#include "mailbox.hpp"
#include <stdint.h>
#include <string.h>
#include "endian.hpp"

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
	uint8_t * const buffer;
	uint_fast16_t size;

public:
	EthernetFrame( uint8_t * const frame_buffer, uint_fast16_t frame_size ) : buffer(frame_buffer), size( frame_size )
	{
	}

	EthernetAddress getDestination() const
	{
		return EthernetAddress( &buffer[ETH_DA_OFS] );
	}

    EthernetAddress getSource() const
	{
		return EthernetAddress( &buffer[ETH_SA_OFS] );
	}

	uint16_t getEtherType() const
	{
		return loadBig16(buffer + ETH_TYPE_OFS);
	}

	uint8_t * getPayload()
	{
		return &buffer[ETH_DATA_OFS];
	}

	uint_fast16_t getPayloadSize() const
	{
		return size - ETH_HEADER_SIZE;
	}
	
	uint8_t * getFrame()
	{
		return buffer;
	}
	
	uint_fast16_t getSize() const
	{
		return size;
	}
	
	static uint16_t getOverhead()
	{
		return 6*2 + 2 + 4;
	}
};

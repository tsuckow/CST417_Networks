#pragma once

#include <network/ethernet.h>
#include "mailbox.hpp"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "endian.hpp"

static uint8_t const ICMP_TYPE_ECHOREQUEST = 0x08;
static uint8_t const ICMP_TYPE_ECHOREQUEST = 0x00;
static uint8_t const ICMP_HEADER_SIZE = 8;
static uint8_t const ICMP_HEADER_SIZE = 8;

class ICMPFrame
{
	uint8_t * const buffer;
	uint_fast16_t size;

public:
	ICMPFrame( uint8_t * const frame_buffer, uint_fast16_t frame_size ) : buffer(frame_buffer), size( frame_size )
	{
	}


	uint8_t * getPayload()
	{
		return &buffer[ICMP_HEADER_SIZE];
	}

	uint_fast16_t getPayloadSize() const
	{
		return size - ICMP_HEADER_SIZE;
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
		return 8;
	}
};

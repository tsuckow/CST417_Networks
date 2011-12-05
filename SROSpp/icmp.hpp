#pragma once

#include <network/ethernet.h>
#include "mailbox.hpp"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "endian.hpp"
#include "ip.hpp"

static uint8_t const ICMP_TYPE_ECHOREQUEST = 0x08;
static uint8_t const ICMP_TYPE_ECHOREPLY = 0x00;
static uint8_t const ICMP_HEADER_SIZE = 8;
static uint8_t const ICMP_TYPE_OFFSET= 0;
static uint8_t const ICMP_CODE_OFFSET = 1;
static uint8_t const ICMP_CHECKSUM_OFFSET = 2;

class ICMPFrame
{
	uint8_t * const buffer;
	uint_fast16_t size;

public:
	ICMPFrame( uint8_t * const frame_buffer, uint_fast16_t frame_size ) : buffer(frame_buffer), size( frame_size )
	{
	}

   uint8_t getType()
   {
      return load8( buffer + ICMP_TYPE_OFFSET );
   }
   
   uint16_t getChecksum() const
   {
      return loadBig16( buffer + ICMP_CHECKSUM_OFFSET );
   }

   uint16_t computeChecksum() const
   {
      return IP::checksum( buffer, size, 1 );
   }
   
   bool isValid()
   {
      return getChecksum() == computeChecksum();
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
		return ICMP_HEADER_SIZE;
	}
};

class ICMP_Listener
{
public:
      virtual ~ICMP_Listener()
      {
      }

      virtual void processFrame( ICMPFrame * frame ) = 0;
};

class ICMP_Handler : public IP::IPv4_Listener
{
      typedef ll<ICMP_Listener *> hlist;
      hlist listeners;

public:
	ICMP_Handler()
	{
	}

	virtual ~ICMP_Handler()
	{
		
	}

	virtual void processFrame( IP::IPv4Frame * ipframe )
	{
		if( ipframe->getProtocol() == IP::IPv4_PROTO_ICMP )
		{
         ICMPFrame icmpframe( ipframe->getPayload(), ipframe->getPayloadSize() );
         
         if( icmpframe.isValid() )
         {
   		   hlist::iterator begin = listeners.begin();
            hlist::iterator end   = listeners.end();
   
            for(; begin != end; ++begin)
            {
               (*begin)->processFrame( &icmpframe );
            }
         }
		}
	}
   
   void addListener( ICMP_Listener * listener )
   {
      listeners.push_back( listener );
   }
};

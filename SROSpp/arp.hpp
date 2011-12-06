//Basic ARP structures

#pragma once

#include "ethernet_handler.hpp"
#include "ipaddress.hpp"
#include "endian.hpp"

static uint_fast8_t const ARP_HTYPE_OFFSET = 0;
static uint_fast8_t const ARP_PTYPE_OFFSET = 2;

static uint_fast8_t const ARP_HLEN_OFFSET = 4;
static uint_fast8_t const ARP_PLEN_OFFSET = 5;

static uint_fast8_t const ARP_OPER_OFFSET = 6;

static uint_fast8_t const ARP_ADDR_OFFSET = 8;

static uint16_t const ARP_HTYPE_ETHERNET = 0x0001;
static uint16_t const ARP_PTYPE_IPV4     = 0x0800;

static uint8_t const ARP_HLEN_ETHERNET = 6;
static uint8_t const ARP_PLEN_IPV4     = 4;

static uint16_t const ARP_OPER_REQUEST  = 1;
static uint16_t const ARP_OPER_REPLY    = 2;

class ARPFrame_Eth_IPv4
{
	uint8_t * const payload;

	public:
	ARPFrame_Eth_IPv4( uint8_t * payload )	: payload(payload)
	{
	}

	uint16_t getHTYPE()
	{
		return loadBig16( payload+ARP_HTYPE_OFFSET );
	}

	uint16_t getPTYPE()
	{
		return loadBig16( payload+ARP_PTYPE_OFFSET );
	}

	uint8_t getHLEN()
	{
		return load8( payload+ARP_HLEN_OFFSET );
	}

	uint8_t getPLEN()
	{
		return load8( payload+ARP_PLEN_OFFSET );
	}

   void setHeaders()
   {
      storeBig16( payload+ARP_HTYPE_OFFSET, ARP_HTYPE_ETHERNET );
      storeBig16( payload+ARP_PTYPE_OFFSET, ARP_PTYPE_IPV4     );
      store8(     payload+ARP_HLEN_OFFSET,  ARP_HLEN_ETHERNET  );
      store8(     payload+ARP_PLEN_OFFSET,  ARP_PLEN_IPV4      );
   }

   uint16_t getOPER()
   {
      return loadBig16( payload+ARP_OPER_OFFSET );
   }

   void setOPER( uint16_t oper )
   {
      storeBig16( payload+ARP_OPER_OFFSET, oper );
   }

	EthernetAddress getSenderEthernetAddress()
	{
		return EthernetAddress( payload + ARP_ADDR_OFFSET );
	}

	IPAddress getSenderIPAddress()
	{
		return IPAddress( payload + ARP_ADDR_OFFSET + ARP_HLEN_ETHERNET );
	}

	EthernetAddress getTargetEthernetAddress()
	{
		return EthernetAddress( payload + ARP_ADDR_OFFSET + ARP_HLEN_ETHERNET + ARP_PLEN_IPV4 );
	}

	IPAddress getTargetIPAddress()
	{
		return IPAddress( payload + ARP_ADDR_OFFSET + ARP_HLEN_ETHERNET*2 + ARP_PLEN_IPV4 );
	}

   void setSenderEthernetAddress(EthernetAddress eaddr)
   {
      eaddr.store( payload + ARP_ADDR_OFFSET );
   }

   void setSenderIPAddress(IPAddress iaddr)
   {
      iaddr.store( payload + ARP_ADDR_OFFSET + ARP_HLEN_ETHERNET );
   }

   void setTargetEthernetAddress(EthernetAddress eaddr)
   {
      eaddr.store( payload + ARP_ADDR_OFFSET + ARP_HLEN_ETHERNET + ARP_PLEN_IPV4 );
   }

   void setTargetIPAddress(IPAddress iaddr)
   {
      iaddr.store( payload + ARP_ADDR_OFFSET + ARP_HLEN_ETHERNET*2 + ARP_PLEN_IPV4 );
   }

	bool isValid()
	{
		bool valid =
			(getHTYPE() == ARP_HTYPE_ETHERNET) &&
			(getPTYPE() == ARP_PTYPE_IPV4) &&
			(getHLEN()  == ARP_HLEN_ETHERNET) &&
			(getPLEN()  == ARP_PLEN_IPV4) &&
				(
					(getOPER()  == ARP_OPER_REQUEST) ||
					(getOPER()  == ARP_OPER_REPLY)
				)
			;

		return valid;
	}

	static uint16_t getSize()
	{
		return 8 + ARP_HLEN_ETHERNET*2 + ARP_PLEN_IPV4*2;
	}
};

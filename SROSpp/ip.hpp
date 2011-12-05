//Internet Protocol Structures

#pragma once

static uint8_t const IP_UNCONFIGURED[4] = {0x00,0x00,0x00,0x00};

class IPAddress
{
protected:

	static size_t const SIZE = 4;
	uint8_t addr[SIZE];

public:
	IPAddress()
	{
	}

	IPAddress( uint8_t const * addr_ )
	{
		memcpy( addr, addr_, SIZE );
	}

	IPAddress( IPAddress const & addr_ )
	{
		memcpy( addr, addr_.addr, SIZE );
	}

	bool operator==( IPAddress const & rhs ) const
	{
		return
			addr[0] == rhs.addr[0] &&
			addr[1] == rhs.addr[1] &&
			addr[2] == rhs.addr[2] &&
			addr[3] == rhs.addr[3]
			;
	}

	void print()
	{
		printf("%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);
	}

   void store( uint8_t * buffer )
   {
      memcpy( buffer, addr, SIZE );
   }
};

namespace IP
{
           /* Compute Internet Checksum for "count" bytes
            *         beginning at location "addr".
            */
   uint16_t checksum( uint8_t const * addr, size_t count, int skip16 = -1 )
   {
      register int32_t sum = 0;

      while( count > 1 )
      {
         /*  This is the inner loop */
         if( skip16 != 0 )
            sum += loadBig16( addr );
         addr += 2;
         count -= 2;
         skip16 -= 1;
      }

      /*  Add left-over byte, if any */
      if( count > 0 )
         sum += *(uint8_t *) addr;

           /*  Fold 32-bit sum to 16 bits */
      while (sum>>16)
         sum = (sum & 0xffff) + (sum >> 16);

      return ~sum;
   }

static uint8_t const IPv4_PROTO_ICMP = 0x01;
static uint8_t const IPv4_VERSION_OFFSET = 0;
static uint8_t const IPv4_VERSION_MASK = 0xF0;
static uint8_t const IPv4_VERSION_BITOFFSET = 4;
static uint8_t const IPv4_HLEN_OFFSET = 0;
static uint8_t const IPv4_HLEN_MASK = 0x0F;
static uint8_t const IPv4_HELN_BITOFFSET = 0;
static uint8_t const IPv4_TOTALLEN_OFFSET = 2;
static uint8_t const IPv4_IDENTIFICATION_OFFSET = 4;
static uint8_t const IPv4_FRAGMENTATION_OFFSET = 6;
static uint8_t const IPv4_TTL_OFFSET = 8;
static uint8_t const IPv4_PROTOCOL_OFFSET = 9;
static uint8_t const IPv4_CHECKSUM_OFFSET = 10;
static uint8_t const IPv4_SOURCE_OFFSET = 12;
static uint8_t const IPv4_DESTINATION_OFFSET = 14;

   class IPv4Frame
   {
      uint8_t * const buffer;
      uint_fast16_t size;

   public:
      IPv4Frame( uint8_t * const frame_buffer, uint_fast16_t frame_size ) : buffer(frame_buffer), size( frame_size )
      {
         printf("IPsize: %u\n", size );
      }

      uint8_t getVersion() const
      {
         uint8_t tmp = load8( buffer + IPv4_VERSION_OFFSET );
         tmp &= IPv4_VERSION_MASK;
         return tmp >> IPv4_VERSION_BITOFFSET;
      }
      
      uint8_t getHeaderLength() const
      {
         uint8_t tmp = load8( buffer + IPv4_HLEN_OFFSET );
         tmp &= IPv4_HLEN_MASK;
         tmp >>= IPv4_HELN_BITOFFSET;
         return tmp * 4;
      }
      
      uint16_t getTotalLength() const
      {
         return loadBig16( buffer + IPv4_TOTALLEN_OFFSET );
      }

  	  uint8_t getProtocol() const
  	  {
      return load8( buffer + IPv4_PROTOCOL_OFFSET );
  	  }

      uint16_t getChecksum() const
      {
         return loadBig16( buffer + IPv4_CHECKSUM_OFFSET );
      }

      uint16_t computeChecksum() const
      {
         return checksum( buffer, getHeaderLength(), 5 );
      }

      bool isValid() const
      {
         return (getChecksum() == computeChecksum()) && ( getHeaderLength() >= (5*4) ) && ( getTotalLength() <= size );
      }

      uint8_t * getPayload()
      {
         return &buffer[getHeaderLength()];
      }

      uint_fast16_t getPayloadSize() const
      {
         return getTotalLength() - getHeaderLength();
      }
      
      uint8_t * getFrame()
      {
         return buffer;
      }
      
      uint_fast16_t getSize() const
      {
         return size;
      }
      
      //We don't support setting options
      static uint16_t getOverhead()
      {
         return 5*4;
      }
   };

   class IPv4_Listener
   {
   public:
      virtual ~IPv4_Listener()
      {
      }

      virtual void processFrame( IPv4Frame * frame ) = 0;
   };

   class IPv4_Handler : public Ethernet_Listener
   {
   protected:
      Ethernet_Handler * const eth_handler;
      IPAddress myIP;
      typedef ll<IPv4_Listener *> hlist;
      hlist listeners;

   public:
      IPv4_Handler( Ethernet_Handler * handler, IPAddress myIP )
         : eth_handler( handler ),
           myIP( myIP )
      {
      }

      virtual ~IPv4_Handler()
      {
      }

      virtual void processFrame( EthernetFrame * frame )
      {
         if( frame->getEtherType() == ETHERNET_TYPE_IPv4 )
         {
            IPv4Frame ipframe( frame->getPayload(), frame->getPayloadSize() );

            if( ipframe.isValid() )
            {
               hlist::iterator begin = listeners.begin();
               hlist::iterator end   = listeners.end();

               for(; begin != end; ++begin)
               {
                  (*begin)->processFrame( &ipframe );
               }
            }
         }
      }

      void addListener( IPv4_Listener * listener )
      {
         listeners.push_back( listener );
      }
   };
}

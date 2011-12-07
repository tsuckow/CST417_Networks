//Internet Protocol Structures

#pragma once

#include "mutex.hpp"
#include "ipaddress.hpp"
#include "arp_handler.hpp"
#include "routingtable.hpp"

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
static uint8_t const IPv4_DESTINATION_OFFSET = 16;

   class IPv4Frame
   {
      uint8_t * const buffer;
      uint_fast16_t size;

   public:
      IPv4Frame( uint8_t * const frame_buffer, uint_fast16_t frame_size ) : buffer(frame_buffer), size( frame_size )
      {
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
     
     void setProtocol(uint8_t protocol ) const
  	  {
      store8( buffer + IPv4_PROTOCOL_OFFSET, protocol );
  	  }
     
     void setTTL(uint8_t ttl )
  	  {
      store8( buffer + IPv4_TTL_OFFSET, ttl );
  	  }
     
     void setIdent(uint8_t id )
  	  {
      storeBig16( buffer + IPv4_IDENTIFICATION_OFFSET, id );
  	  }
     
      IPAddress getSource()
   	{
   		return IPAddress( buffer + IPv4_SOURCE_OFFSET );
   	}
      
      void setSource(IPAddress iaddr)
      {
         iaddr.store( buffer + IPv4_SOURCE_OFFSET );
      }
     
      IPAddress getDestination()
   	{
   		return IPAddress( buffer + IPv4_DESTINATION_OFFSET );
   	}
      
      void setDestination(IPAddress iaddr)
      {
         iaddr.store( buffer + IPv4_DESTINATION_OFFSET );
      }

      uint16_t getChecksum() const
      {
         return loadBig16( buffer + IPv4_CHECKSUM_OFFSET );
      }

      uint16_t computeChecksum() const
      {
         return checksum( buffer, getHeaderLength(), IPv4_CHECKSUM_OFFSET / 2 );
      }
      
      void setChecksum( uint16_t checksum )
      {
         return storeBig16( buffer + IPv4_CHECKSUM_OFFSET, checksum );
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
      
      void setHeaders()
      {
         storeBig16( buffer + 0, 0x4500 );
         storeBig16( buffer + IPv4_TOTALLEN_OFFSET, size );
         storeBig16( buffer + IPv4_FRAGMENTATION_OFFSET, 0 );
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
      ARP_Handler * arp_handler;
      RoutingTable * routingtable;
      
      typedef ll<IPv4_Listener *> hlist;
      hlist listeners;
      
      Mutex identity_mutex;
      uint16_t identity;

      uint16_t nextIdent()
      {
         identity_mutex.lock();
         uint16_t tmp = ++identity;
         identity_mutex.release();
         return tmp;
      }

   public:
      IPv4_Handler( Ethernet_Handler * handler, IPAddress myIP, ARP_Handler * arp_handler, RoutingTable * table )
         : eth_handler( handler ),
           myIP( myIP ),
           arp_handler( arp_handler ),
           routingtable( table )
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
            
            if( ipframe.isValid() && ( (ipframe.getDestination() == IPv4_BROADCAST) || (ipframe.getDestination() == myIP) ) )
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
      
      bool sendFrame( EthernetFrame * ethframe, IP::IPv4Frame * ipframe )
      {
         ipframe->setIdent( nextIdent() );
         ipframe->setSource( myIP );
         ipframe->setChecksum( ipframe->computeChecksum() );
         
         ethframe->setEtherType( ETHERNET_TYPE_IPv4 );    
         
         IPAddress hop = routingtable->nextHop( ipframe->getDestination() );
         
         EthernetAddress eaddress = arp_handler->request( hop );
			if( !(eaddress == ETHERNET_UNCONFIGURED) )
			{
      			ethframe->setDestination( eaddress );
               
               eth_handler->sendFrame( ethframe );
               
               return true;
			}
         
         return false;
      }
   };
}

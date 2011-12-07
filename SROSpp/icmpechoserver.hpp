#pragma once

#include "icmp.hpp"

class ICMPEchoServer : public ICMP_Listener
{
   struct EchoReply
   {
      uint8_t * buffer;
      size_t size;
   };
   
   Mailbox<EchoReply> replies;
   
   bool enabled;

public:
	ICMPEchoServer() : replies( 3 ), enabled( true )
	{
	}

	virtual bool processFrame( IP::IPv4Frame * ipframe, ICMPFrame * icmpframe )
   {
      if( icmpframe->getType() == ICMP_TYPE_ECHOREQUEST && enabled )
      {         
         size_t packetsize = icmpframe->getSize() + IP::IPv4Frame::getOverhead() + EthernetFrame::getOverhead();
         uint8_t * buf = new uint8_t[ packetsize ];
         
         if( buf != 0 )
         {
            EthernetFrame rethframe( buf, packetsize );
            IP::IPv4Frame ripframe ( rethframe.getPayload(), rethframe.getPayloadSize() );
            ripframe.setHeaders();
            ICMPFrame ricmpframe( ripframe.getPayload(), ripframe.getPayloadSize() );
            
            ricmpframe.setType( ICMP_TYPE_ECHOREPLY );
            ricmpframe.setQuench( icmpframe->getQuench() );
            memcpy( ricmpframe.getPayload(), icmpframe->getPayload(), icmpframe->getPayloadSize() );
            ricmpframe.setChecksum( ricmpframe.computeChecksum() );
            
            ripframe.setDestination( ipframe->getSource() );
            ripframe.setSource( ipframe->getDestination() );
            ripframe.setProtocol( IP::IPv4_PROTO_ICMP );
            ripframe.setTTL( 64 );
            
            EchoReply reply;
            reply.buffer = buf;
            reply.size = packetsize;
            
            bool sent = replies.send( 0, &reply );
            
            // :( Drop the reply, buffer is full
            if( !sent )
            {
               delete [] buf;
            }
         }
         else
         {
            printf("Out of mem: Echo Server\n");
         }
      }
      
      return false;
   }
   
   void serverThread( IP::IPv4_Handler * handler )
   {
      EchoReply reply;
      
      while(true)
      {
         replies.recv( -1, &reply );
         
         EthernetFrame ethframe( reply.buffer, reply.size );
         IP::IPv4Frame ipframe ( ethframe.getPayload(), ethframe.getPayloadSize() );
         
         handler->sendFrame( &ethframe, &ipframe );
         
         delete [] reply.buffer;
      }
   }
   
   static void serverThread( ICMPEchoServer * server, IP::IPv4_Handler * handler )
   {
      server->serverThread( handler );
   }
   
   bool isEnabled()
   {
      return enabled;
   }
   
   void setEnabled( bool en )
   {
      enabled = en;
   }
};

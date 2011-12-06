// Middle layer between the ARP cache and ethernet
// Provides the user API for performing an address resolution

#pragma once

#include "arp.hpp"
#include "ipaddress.hpp"
#include "mailbox.hpp"
#include "arp_cache.hpp"

class ARP_Handler : public Ethernet_Listener
{
protected:
	Ethernet_Handler * const eth_handler;
	ARP_Cache cache;
	IPAddress myIP;

	struct RequestMessage
	{
		IPAddress const * request;
		Mailbox<EthernetAddress> * response;
	};

	struct ARPFrame
	{
		uint16_t opcode;
		EthernetAddress hsender;
		IPAddress		psender;
		EthernetAddress htarget;
		IPAddress		ptarget;
	};

	static unsigned int const CACHE_ENTRY_EXPIRATION = 60 * 1000;
   static unsigned int const REQUEST_EXPIRATION = 3000;
   static unsigned int const REQUEST_TRIES = 2;

	Mailbox<RequestMessage> requestQueue;
	static unsigned int const REQUEST_QUEUE_SIZE = 1; //Threads can just block until it is put on.

	Mailbox<ARPFrame> recieveQueue;
	static unsigned int const RECIEVE_QUEUE_SIZE = 4;
     
   void sendARP( EthernetAddress etarget, IPAddress itarget, uint16_t oper )
   {
      size_t const packet_size = ARPFrame_Eth_IPv4::getSize() + EthernetFrame::getOverhead();
      uint8_t * packetBuffer = new uint8_t[ packet_size ];
      
      if( packetBuffer != 0)
      {
         {
            EthernetFrame eframe( packetBuffer, packet_size );
            ARPFrame_Eth_IPv4 arpframe( eframe.getPayload() );
            
            eframe.setDestination( etarget );
            eframe.setSource( eth_handler->getAddress() );
            eframe.setEtherType( ETHERNET_TYPE_ARP );
            
            arpframe.setHeaders();
            
            arpframe.setOPER( oper );
            arpframe.setTargetEthernetAddress( etarget );
            arpframe.setTargetIPAddress( itarget );
            
            arpframe.setSenderEthernetAddress( eth_handler->getAddress() );
            arpframe.setSenderIPAddress( myIP );
            
            eth_handler->sendFrame( &eframe );
         }
         
         delete [] packetBuffer;
      }
      else
      {
         printf("Out of mem: sendARP\n");
      }
   }  
       
   void sendRequest( IPAddress target )
   {
      sendARP( ETHERNET_BROADCAST, target, ARP_OPER_REQUEST );
   }
   
   void sendReply( EthernetAddress etarget, IPAddress itarget )
   {
      sendARP( etarget, itarget, ARP_OPER_REPLY );
   }
   
   void handleExpired()
   {
      uint64_t t = srostime;
      
      cache.removeExpiredEntries( t );
      
      ARPNotification an;
      
      while( (an = cache.removeExpiredRequest( t )).response != 0 )
      {
         if( an.count > 0 ) an.count--;
         if( an.count > 0 )
         {
            //Try Again
            an.time = t + REQUEST_EXPIRATION;
            if( !cache.notify( an ) )
            {
			   //printf("Sending ARP Request Again...\n");
               sendRequest( *an.request );
            }
         }
         else
         {
            //Give Up
            EthernetAddress addr( ETHERNET_UNCONFIGURED );
            an.response->send( 0, &addr );
         }
      }
   }

public:
	ARP_Handler( Ethernet_Handler * handler, IPAddress myIP )
		: eth_handler( handler ),
		  myIP( myIP ),
		  requestQueue( REQUEST_QUEUE_SIZE ),
		  recieveQueue( RECIEVE_QUEUE_SIZE )
	{
	}

	virtual ~ARP_Handler()
	{
	}

	virtual void processFrame( EthernetFrame * frame )
	{
		if( frame->getEtherType() == ETHERNET_TYPE_ARP )
		{
			ARPFrame_Eth_IPv4 arpFrame( frame->getPayload() );

			if( arpFrame.isValid() )
			{
				ARPFrame recieveframe;

				recieveframe.opcode = arpFrame.getOPER();
				recieveframe.hsender = arpFrame.getSenderEthernetAddress();
				recieveframe.psender = arpFrame.getSenderIPAddress();
				recieveframe.htarget = arpFrame.getTargetEthernetAddress();
				recieveframe.ptarget = arpFrame.getTargetIPAddress();

				//Return immidiatly drop packet if buffer full.
				recieveQueue.send( 0, &recieveframe );
			}
		}
	}

	void requestThread()
	{
		while(true)
		{
			RequestMessage message;
         
         uint64_t next = cache.nextExpiration();
         int timediff = -1;
         if( next > 0 )
         {
            timediff = next - srostime;
            if( timediff < 0 ) timediff = 0;
         }

         bool gotMsg = requestQueue.recv( timediff, &message );

         handleExpired();

         if( gotMsg )
         {
            ARPNotification notification;
            notification.request = message.request;
            notification.response = message.response;
            notification.count = 2;
            notification.time = srostime + REQUEST_EXPIRATION;       
            
            if( !cache.notify( notification ) )
            {
			      //printf("Sending ARP Request...\n");
               sendRequest( *message.request );
            }
         }
		}
	}

	void packetRecieveThread()
	{
		while(true)
		{
			ARPFrame message;
			recieveQueue.recv( -1, &message );
         
         if( !(message.psender == IPv4_UNCONFIGURED) )
         {
            //printf( "Adding to cache: %llu\n", srostime );
			   cache.updateEntry( message.hsender, message.psender, srostime + CACHE_ENTRY_EXPIRATION );
         }
			
         //Handle request for me
         if( message.opcode == ARP_OPER_REQUEST )
         {
            if( message.ptarget == myIP )
            {
               //printf( "Sending reply\n" );
               sendReply( message.hsender, message.psender );
            }
         }
		}
	}

	//! \note This function is thread safe
    //! \note This call blocks until the address is resolved or timeout
	//! \return Ethernet address associated with the IP address or all zeros if resolution failed.
	EthernetAddress request( IPAddress const & address )
	{
		Mailbox<EthernetAddress> response(1);
		
		RequestMessage message;
		message.request = &address;
		message.response = &response;

		requestQueue.send( -1, &message );

		EthernetAddress eaddress;
		response.recv( -1, &eaddress );

		return eaddress;
	}
   
   void clear()
   {
      cache.clear();
   }
   
   void print()
   {
      cache.print( srostime );
   }
};

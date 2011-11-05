#pragma once

#include "arp.hpp"
#include "ip.hpp"
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

	Mailbox<RequestMessage> requestQueue;
	static unsigned int const REQUEST_QUEUE_SIZE = 1; //Threads can just block until it is put on.

	Mailbox<ARPFrame> recieveQueue;
	static unsigned int const RECIEVE_QUEUE_SIZE = 4;

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
			//TODO
			RequestMessage message;
			requestQueue.recv( -1, &message );

			EthernetAddress eaddress( (unsigned char const *)"\x00\x01\x02\x03\x04\x05" );
			message.response->send( 0, &eaddress ); //return now if full, only I should send to it.
		}
	}

	void packetRecieveThread()
	{
		while(true)
		{
			ARPFrame message;
			recieveQueue.recv( -1, &message );

			cache.updateEntry( message.hsender, message.psender );

			//TODO
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
};

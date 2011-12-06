#pragma once

#include "ll.hpp"
#include "ethernet.hpp"
#include "ethernet_driver.hpp"

class Ethernet_Listener
{
public:
	virtual ~Ethernet_Listener(){}
	
	virtual void processFrame( EthernetFrame * frame ) = 0;
};

class Ethernet_Handler
{
	protected:
		typedef ll<Ethernet_Listener *> hlist;
		hlist listeners;
		
		Ethernet_Driver * const driver;
	
		struct SendQueueItem
		{
			EthernetFrame * frame;
			Semaphore response;
		};
		
		Mailbox<SendQueueItem *> sendQueue;
		static unsigned int const QUEUE_SIZE = 3;
	
	public:
		Ethernet_Handler( Ethernet_Driver * driver ) : driver( driver ), sendQueue( QUEUE_SIZE )
		{
		}
	
		void addListener( Ethernet_Listener * listener )
		{
			listeners.push_back( listener );
		}
		
		void handleRecv( EthernetFrame * frame )
		{
			hlist::iterator begin = listeners.begin();
			hlist::iterator end   = listeners.end();
			
			for(; begin != end; ++begin)
			{
				(*begin)->processFrame( frame );
			}		
		}
		
		void sendFrame( EthernetFrame * frame, int32_t timeout = -1 )
		{
			SendQueueItem item;
			item.frame = frame;
			SendQueueItem * itemptr( &item );
         
			sendQueue.send( timeout, &itemptr );
         
			item.response.wait();
		}
		
		void sender()
		{
			SendQueueItem * item;
			
			while(true)
			{
				sendQueue.recv( -1, &item );
				//Process it
				driver->sendFrame( item->frame->getFrame(), item->frame->getSize() );
				item->response.signal();
			}
		}
      
      EthernetAddress getAddress()
		{
         return driver->getAddress();
		}
};

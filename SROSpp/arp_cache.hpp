#pragma once

#include "ll.hpp"
#include "mutex.hpp"

struct ARPCacheEntry
{
	IPAddress iaddr;
	EthernetAddress eaddr;
   uint64_t time;
   
   ARPCacheEntry() : time( 0 )
   {
   }
};

struct ARPNotification
{
	IPAddress const * request;
	Mailbox<EthernetAddress> * response;
   unsigned int count;
   uint64_t time;
};

//! \note This class is thread safe
class ARP_Cache
{

protected:
   
   static size_t const NUM_ENTRIES = 4;
   
   ARPCacheEntry entries[NUM_ENTRIES];
  
   typedef ll< ARPNotification > plist; 
   plist pending;
	Mutex mutex;
   
   void informPending( EthernetAddress eaddress, IPAddress iaddress )
   {
      plist::iterator begin = pending.begin();
		plist::iterator end   = pending.end();
		
		while( begin != end )
		{
         if( *(begin->request) == iaddress )
         {
		      begin->response->send( 0, &eaddress );
            begin = pending.erase( begin );
            continue; //Skip increment
         }
         
         ++begin;
		}
   }

public:

	void updateEntry(EthernetAddress eaddress, IPAddress iaddress, uint64_t timeout)
	{
		mutex.lock();
      {
   		printf("UPDATE ARP WITH: ");
   	    eaddress.print();
   		printf(" ");
   		iaddress.print();
   		printf("\n");
         
         bool found = false;
         ARPCacheEntry * oldest = &entries[0];
         
         for( size_t i = 0; i < NUM_ENTRIES; ++i )
         {
            if( entries[i].iaddr == iaddress )
            {
               entries[i].time = timeout;
               entries[i].eaddr = eaddress;
               found = true;
               break;
            }
            
            if( entries[i].time < oldest->time )
            {
               oldest = &entries[i];
            }
         }
      
         if( !found )
         {
            oldest->time = timeout;
            oldest->eaddr = eaddress;
            oldest->iaddr = iaddress;
         }
         
         informPending( eaddress, iaddress );
		}	
		mutex.release();
	}

	void removeExpiredEntries()
	{
		mutex.lock();
		mutex.release();
	}
   
   ARPNotification removeExpiredRequest( uint64_t time )
	{
      ARPNotification notification;
		mutex.lock();
      {
         notification.request = 0;
         notification.response = 0;
         notification.count = 0;
         notification.time = 0;
         
         plist::iterator begin = pending.begin();
   		plist::iterator end   = pending.end();
   		
   		for(; begin != end; ++begin)
   		{
            if( begin->time < time )
            {
   		      notification = *begin;
               begin = pending.erase( begin );
               break;
            }
   		}
      }
		mutex.release();
      return notification;
	}

	bool notify( ARPNotification & msg )
	{
		mutex.lock();
      
      bool found = false;
      
      for( size_t i = 0; i < NUM_ENTRIES; ++i )
      {
         if( entries[i].iaddr == *msg.request )
         {
            msg.response->send( 0, &entries[i].eaddr );
            found = true;
            break;
         }
      }
      
      if( !found )
      {
         pending.push_front( msg );
      }
      
		mutex.release();
      return found;
	}
};


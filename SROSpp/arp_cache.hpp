#pragma once

#include "mutex.hpp"

class ARPCacheEntry
{
	//IP4Addr
	//EthernetAddress
	//Timeout
	//Valid (Combined with timeout?)
};

//! \note This class is thread safe
class ARP_Cache
{

protected:
	
	Mutex mutex;

public:

	void updateEntry(EthernetAddress eaddress, IPAddress iaddress)
	{
		mutex.lock();

		printf("UPDATE ARP WITH: ");
	    eaddress.print();
		printf(" ");
		iaddress.print();
		printf("\n");
			
		mutex.release();
	}

	void removeExpired()
	{
		mutex.lock();
		mutex.release();
	}

	void peekEntry()
	{
		mutex.lock();
		mutex.release();
	}
};


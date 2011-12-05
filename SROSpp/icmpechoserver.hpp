#pragma once

#include "icmp.hpp"

class ICMPEchoServer : public ICMP_Listener
{

public:
	ICMPEchoServer()
	{
	}

	virtual void processFrame( ICMPFrame * frame )
   {
      if( frame->getType() == ICMP_TYPE_ECHOREQUEST )
      {
         printf("Echo Request\n");
      }
   }
};

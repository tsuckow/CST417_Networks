#pragma once

class RoutingTable
{
	IPAddress network;
	IPAddress mask;
	IPAddress GW;

public:
	RoutingTable( IPAddress network, IPAddress mask, IPAddress GW )
		: network( network ), mask( mask ), GW( GW )
	{
	}

	IPAddress nextHop( IPAddress dest )
	{
      if( (dest & mask) == (network & mask) )
      {
         return dest;
      }
      else
      {
         return GW;
      }
	}
};
